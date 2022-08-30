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
* @file scib.c
*
* @brief The module is buffer management utility for SKernel modules.
*
*
* @version   26
********************************************************************************
*/
#include <os/simTypesBind.h>
#include <asicSimulation/SCIB/scib.h>
#include <asicSimulation/SInit/sinit.h>
#include <asicSimulation/SDistributed/sdistributed.h>
#include <asicSimulation/wmApi.h>
#include <asicSimulation/SLog/simLog.h>

#define MASK_BITS(numOfBits) \
   (((numOfBits) == 32) ? 0xFFFFFFFF : (~(0xFFFFFFFF << (numOfBits))))

static GT_U32   _non_dest = 0;
static GT_U32   _broker = 0;
static GT_U32   _bus = 0;
static GT_U32   _app = 0;
static GT_U32   _dev = 0;

#define IN_GLOBAL_CB(cb_func) \
    inside_wmGlobalBindAsimServices.inside_##cb_func = GT_TRUE
#define OUT_GLOBAL_CB(cb_func) \
    inside_wmGlobalBindAsimServices.inside_##cb_func = GT_FALSE;        \
    CLEAR_IS_STUCK_INSIDE_GLOBAL_CB(cb_func)

#define CHECK_IS_STUCK_INSIDE_GLOBAL_CB(cb_func) \
    if(inside_wmGlobalBindAsimServices.stuck_inside_##cb_func == GT_TRUE) \
    {                                                                     \
        fprintf( stderr,"stuck inside [%s] \n",#cb_func);                 \
    }

#define START_IS_STUCK_INSIDE_GLOBAL_CB(cb_func) \
    if(inside_wmGlobalBindAsimServices.inside_##cb_func == GT_TRUE)      \
    {                                                                    \
        inside_wmGlobalBindAsimServices.stuck_inside_##cb_func = GT_TRUE;\
    }

#define CLEAR_IS_STUCK_INSIDE_GLOBAL_CB(cb_func) \
    inside_wmGlobalBindAsimServices.stuck_inside_##cb_func = GT_FALSE

#define DEFINE_MEMBERS_INSIDE_GLOBAL_CB(cb_func)   \
    GT_BOOL inside_##cb_func;                      \
    GT_BOOL stuck_inside_##cb_func
/*
 * Typedef: struct WM_CHECK_CALLBACK_RETURN_STC
 *
 * Description:
 *      Hold the call back (CB) functions that the WM need to access the outer world:
 *      read/write DMA/DRAM , trigger interrupt , egress packet from a port
 *
 * Fields:
 *      inside_dmaReadFunc          : indication that we are inside a CB function for DMA read
 *      inside_dmaWriteFunc         : indication that we are inside a CB function for DMA write
 *      inside_interruptTriggerFunc : indication that we are inside a CB function for interrupt triggering
 *      inside_egressPacketFunc     : indication that we are inside a CB function for egress packet from a port in device.
 *      inside_traceLogFunc         : indication that we are inside a CB function for trace strings that WM wants to log.
 *
 * Comments: follow calls done by : wmGlobalBindAsimServices.xxx
 */
typedef struct{
    DEFINE_MEMBERS_INSIDE_GLOBAL_CB    (dmaReadFunc           );
    DEFINE_MEMBERS_INSIDE_GLOBAL_CB    (dmaWriteFunc          );
    DEFINE_MEMBERS_INSIDE_GLOBAL_CB    (interruptTriggerFunc  );
    DEFINE_MEMBERS_INSIDE_GLOBAL_CB    (egressPacketFunc      );
    DEFINE_MEMBERS_INSIDE_GLOBAL_CB    (traceLogFunc          );

}WM_CHECK_CALLBACK_RETURN_STC;
static WM_CHECK_CALLBACK_RETURN_STC inside_wmGlobalBindAsimServices = {GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE,GT_FALSE};
static void scibInitCbFuncWatchDogTask(void);

static WM_BIND_CB_FUNC_STC  wmGlobalBindAsimServices = {NULL,NULL,NULL,NULL,NULL};

extern GT_STATUS skernelRegister_0x4c_Update
(
    IN void*  /*SKERNEL_DEVICE_OBJECT* */   *devObjPtr,
    IN GT_U32                  DeviceID_field,
    IN GT_U32                  RevisionID_field
);
GT_STATUS skernelBusInterfaceGet
(
    IN void*  /*SKERNEL_DEVICE_OBJECT* */   *devObjPtr,
    OUT GT_U32                *interfaceBmpPtr
);

extern WM_STATUS skernelPortConnectionInfo
(
    IN void*  /*SKERNEL_DEVICE_OBJECT* */    devObjPtr,
    IN GT_U32   portNum,
    IN WM_PORT_CONNECTION_INFO_STC  *infoPtr
);

extern void skernelDeviceResetType(
    IN void*  /*SKERNEL_DEVICE_OBJECT* */    devObjPtr,
    IN GT_U32           resetType
);

/*******************************************************************************
* Private type definition
*******************************************************************************/
/**
* @struct SCIB_DEVICE_DB_STC
 *
 * @brief Describe a SCIB database entry (entry per device).
*/
typedef struct{

    /** @brief : the info is (about this device)
     *  deviceObj   : Pointer to opaque deviceObj, is used as parameter
     *  : for memAccesFun calls.
     */
    GT_BOOL valid;

    void *   deviceObj;

    /** : Entry point for R/W SKernel memory function. */
    SCIB_RW_MEMORY_FUN memAccesFun;

    /** : Interrupt line number. */
    GT_U32 intLine;

    /** : Physical device Id. */
    GT_U32 deviceHwId;

    /** : device enable/disable address completion */
    GT_BOOL addressCompletionStatus;

    /** @brief : saved copy of the memAccesFun used for 'rebind' action
     *  see function scibReBindDevice
     */
    SCIB_RW_MEMORY_FUN origMemAccesFun;

    /** @brief : CB function to allow device to be bound with
     *  it's interrupt line to MPP (multi-purpose pin) of another device
     *  Comments:
     */
    SCIB_INTERRUPT_SET_FUN interruptMppTriggerFun;

    /** @brief : which interfaces are forbidden by the device.
     */
    GT_U32      forbiddenInterfaceBmp;

    /** @brief : indication that the device must work with the BARs 0,2 as defined in the 'pci config space' .
        so function like scibGetDeviceId() is not supported .
     */
    GT_BOOL     deviceForceBar0Bar2;

    GT_U32 pciBus;           /*relevant for : wmMemPciConfigSpaceRead,wmMemPciConfigSpaceWrite */
    GT_U32 pciDev;           /*relevant for : wmMemPciConfigSpaceRead,wmMemPciConfigSpaceWrite */
    GT_U32 pciFunc;          /*relevant for : wmMemPciConfigSpaceRead,wmMemPciConfigSpaceWrite */
#define ADDR_LOW_INDEX   0
#define ADDR_HIGH_INDEX  1
    GT_U32  bar0_base[2];    /*relevant for : wmMemPciRead,wmMemPciWrite : index[0] low 32 bits , index[1] high 32 bits */
    GT_U32  bar0_size;       /*relevant for : wmMemPciRead,wmMemPciWrite */

    GT_U32  bar2_base[2];    /*relevant for : wmMemPciRead,wmMemPciWrite : index[0] low 32 bits , index[1] high 32 bits */
    GT_U32  bar2_size;       /*relevant for : wmMemPciRead,wmMemPciWrite */

    GT_U32 interrupt_line  ;  /*relevant for : wmGlobalBindAsimServices.interruptTriggerFunc */
    GT_U32 interrupt_source;  /*relevant for : wmGlobalBindAsimServices.interruptTriggerFunc */
    GT_U32 interrupt_flags ;  /*relevant for : wmGlobalBindAsimServices.interruptTriggerFunc */

} SCIB_DEVICE_DB_STC;

static SCIB_DEVICE_DB_STC      *   scibDbPtr;
static SCIB_DEVICE_DB_STC      *   scibFaDbPtr;
static GT_U32               maxDevicesPerSystem;

#define FA_ADDRESS_BIT_CNS  (1 << 30)

#define DEV_NUM_CHECK_MAC(deviceId,funcNam)     \
    DEV_NUM_CHECK_RANGE_MAC(deviceId,funcNam);   \
    if (scibDbPtr[deviceId].deviceObj == NULL)  \
        return

#define DEV_NUM_CHECK_RANGE_MAC(deviceId,funcNam)  \
    if (deviceId >= maxDevicesPerSystem)           \
        skernelFatalError(" %s : device id [%d] out of boundary \n",#funcNam,deviceId)

#define DEV_NUM_CHECK_MAC_WITH_RETURN_STATUS(deviceId,funcNam)     \
    DEV_NUM_CHECK_RANGE_MAC(deviceId,funcNam);   \
    if (scibDbPtr[deviceId].deviceObj == NULL)  \
        return GT_NOT_FOUND


/* Maximal devices number */
#define SOHO_SMI_DEV_TOTAL          (32)
/* Start address of global register.       */
#define SOHO_GLOBAL_REGS_START_ADDR (0x1b)
/* Start address of ports related register.*/
#define SOHO_PORT_REGS_START_ADDR   (0x10)

/* debug printings */
#define SCIB_DEBUG_PRINT_MAC(x)     if(debugPrint_enable) printf x


/* debug printings */
#define SCIB_DEBUG_PRINT_WRITE_MAC(deviceId,memAddr,length,dataPtr)        \
    {                                                                      \
        GT_U32  index;                                                     \
        SCIB_DEBUG_PRINT_MAC(("write[%d][%8.8x][%8.8x]" ,deviceId,memAddr,dataPtr[0]));\
                                                                           \
        for(index = 1 ;index < length; index++)                            \
        {                                                                  \
            SCIB_DEBUG_PRINT_MAC(("[%8.8x]" ,deviceId,memAddr,dataPtr[index]));\
        }                                                                  \
        SCIB_DEBUG_PRINT_MAC(("\n"));                                      \
    }



static GT_BOOL debugPrint_enable = GT_FALSE;

/*******************************************************************************
*  DMA_READ_FUN
*
* DESCRIPTION:
*      read HOST CPU DMA memory function.
* INPUTS:
*
*       deviceId    - device id. (of the device in the simulation)
*       address     - physical address that PP refer to.
*                     HOST CPU must convert it to HOST memory address
*       memSize     - Size of ASIC memory to read or write.
*       dataIsWords - the data to read is words or bytes
*                     1 - words --> swap network order to cpu order
*                     0 - bytes --> NO swap network order to cpu order
*
* OUTPUTS:
*       memPtr     - (pointer to) PP's memory in which HOST CPU memory will be
*                    copied.
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
typedef void (* DMA_READ_FUN)
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr,
    IN GT_U32  dataIsWords
);

/*******************************************************************************
*  DMA_WRITE_FUN
*
* DESCRIPTION:
*      write to HOST CPU DMA memory function.
* INPUTS:
*
*       deviceId    - device id. (of the device in the simulation)
*       address     - physical address that PP refer to.
*                     HOST CPU must convert it to HOST memory address
*       memSize     - Size of ASIC memory to read or write.
*       memPtr     - (pointer to) data to write to HOST CPU memory.
*       dataIsWords - the data to read is words or bytes
*                     1 - words --> swap network order to cpu order
*                     0 - bytes --> NO swap network order to cpu order
*
* OUTPUTS:
*       none
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
typedef void (* DMA_WRITE_FUN)
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_U32 * memPtr,
    IN GT_U32  dataIsWords
);

/**
* @struct SCIB_GLOBAL_DB_STC
 *
 * @brief Describe a SCIB global database .
*/
typedef struct{

    /** function set the interrupt line */
    SCIB_INTERRUPT_SET_FUN interruptSetFun;

    /** @brief function read DMA memory
     *  Comments:
     */
    DMA_READ_FUN dmaReadFun;

    /** function write DMA memory */
    DMA_WRITE_FUN dmaWriteFun;

} SCIB_GLOBAL_DB_STC;


#define DMA_READ_PROTOTYPE_BUILD_MAC(funcName)\
    static void funcName                    \
    (                                       \
        IN GT_U32 deviceId,             \
        IN GT_U32 address,              \
        IN GT_U32 memSize,              \
        OUT GT_U32 * memPtr,            \
        IN GT_U32  dataIsWords          \
    )

#define DMA_WRITE_PROTOTYPE_BUILD_MAC(funcName)\
    static void funcName                    \
    (                                       \
        IN GT_U32 deviceId,             \
        IN GT_U32 address,              \
        IN GT_U32 memSize,              \
        IN GT_U32 * memPtr,             \
        IN GT_U32  dataIsWords          \
    )

#define INTERRUPT_SET_PROTOTYPE_BUILD_MAC(funcName)\
    static void funcName                    \
    (                                       \
        IN  GT_U32        deviceId          \
    )

#define REGISTER_READ_WRITE_PROTOTYPE_BUILD_MAC(funcName)\
    static void funcName                           \
    (                                              \
        IN SCIB_MEMORY_ACCESS_TYPE accessType,     \
        IN GT_U32   deviceId,                      \
        IN GT_U32 address,                     \
        IN GT_U32 memSize,                     \
        INOUT GT_U32 * memPtr                  \
    )



DMA_READ_PROTOTYPE_BUILD_MAC(dmaRead);
DMA_READ_PROTOTYPE_BUILD_MAC(dmaRead_distributedApplication);
DMA_READ_PROTOTYPE_BUILD_MAC(dmaRead_distributedAsic);

DMA_WRITE_PROTOTYPE_BUILD_MAC(dmaWrite);
DMA_WRITE_PROTOTYPE_BUILD_MAC(dmaWrite_distributedApplication);
DMA_WRITE_PROTOTYPE_BUILD_MAC(dmaWrite_distributedAsic);

INTERRUPT_SET_PROTOTYPE_BUILD_MAC(interruptSet);
INTERRUPT_SET_PROTOTYPE_BUILD_MAC(interruptSet_distributedApplication);
INTERRUPT_SET_PROTOTYPE_BUILD_MAC(interruptSet_distributedAsic);

REGISTER_READ_WRITE_PROTOTYPE_BUILD_MAC(registerReadWrite_distributedApplication);

/* the functions needed by the "non-distributed" simulation */
static SCIB_GLOBAL_DB_STC nonDistributedObject =
{
    &interruptSet ,
    &dmaRead ,
    &dmaWrite
};

/* the functions needed by the "distributed application side" simulation */
static SCIB_GLOBAL_DB_STC distributedApplicationSideObject =
{
    &interruptSet_distributedApplication ,
    &dmaRead_distributedApplication ,
    &dmaWrite_distributedApplication
};

/* the functions needed by the "distributed asic side" simulation */
static SCIB_GLOBAL_DB_STC distributedAsicSideObject =
{
    &interruptSet_distributedAsic ,
    &dmaRead_distributedAsic ,
    &dmaWrite_distributedAsic
};

/* the functions needed by the "distributed broker" simulation */
static SCIB_GLOBAL_DB_STC distributedBrokerObject =
{
    &interruptSet_distributedApplication ,
    &dmaRead_distributedApplication ,
    &dmaWrite_distributedApplication
};

/* the functions needed by the "distributed interface bus" simulation */
static SCIB_GLOBAL_DB_STC distributedBusObject =
{
    &interruptSet_distributedApplication ,
    &dmaRead_distributedApplication ,
    &dmaWrite_distributedApplication
};

/* pointer to the object that implement the needed functions */
static SCIB_GLOBAL_DB_STC* globalObjectPtr = &nonDistributedObject;

static GT_MUTEX simulationProtectorMtx = (GT_MUTEX)0;
/* counter to indicate the locking level of simulationProtectorMtx */
static GT_U32   simulationProtectorMtx_count = 0;
/* get the counter to indicate the locking level of simulationProtectorMtx */
GT_U32  scibAccessMutexCountGet(void)
{
    return simulationProtectorMtx_count;
}

/**
* @internal scibInit0 function
* @endinternal
*
* @brief   Init SCIB mutex for scibAccessLock(), scibAccessUnlock
*/
void scibInit0(void)
{
    /*create the SCIB layer mutex */
    if (simulationProtectorMtx == (GT_MUTEX)0)
    {
        simulationProtectorMtx = SIM_OS_MAC(simOsMutexCreate)();
    }
}

/**
* @internal scibInit function
* @endinternal
*
* @brief   Init SCIB library.
*
* @param[in] maxDevNumber             - maximal number of SKernel devices in the Simulation.
*
* @note the database of the FA is different from the devices because
*       they have the same ID but different types of devices.
*
*/
void scibInit
(
    IN GT_U32 maxDevNumber
)
{
    GT_U32              size;

    if (maxDevNumber == 0) {
        skernelFatalError("scibInit: illegal maxDevNumber %lu\n", maxDevNumber);
    }

    switch(sasicgSimulationRole)
    {
        case SASICG_SIMULATION_ROLE_NON_DISTRIBUTED_E:
            /* bind the dynamic functions to the object of "non-distributed"
               architecture , direct accessing no sockets used */
            globalObjectPtr = &nonDistributedObject;
            _non_dest = 1;
            break;

        case SASICG_SIMULATION_ROLE_DISTRIBUTED_APPLICATION_SIDE_E:
        case SASICG_SIMULATION_ROLE_DISTRIBUTED_APPLICATION_SIDE_VIA_BROKER_E:
            _app = 1;
            /* bind the dynamic functions to the object of "distributed- application side"
               architecture , accessing via socket */
            globalObjectPtr = &distributedApplicationSideObject;
            break;

        case SASICG_SIMULATION_ROLE_DISTRIBUTED_ASIC_SIDE_E:
        case SASICG_SIMULATION_ROLE_DISTRIBUTED_ASIC_SIDE_VIA_INTERFACE_BUS_BRIDGE_E:
            _dev = 1;
            /* bind the dynamic functions to the object of "distributed- Asic side"
               architecture , accessing via socket */
            globalObjectPtr = &distributedAsicSideObject;
            break;
        case SASICG_SIMULATION_ROLE_BROKER_E:
            _broker = 1;
            globalObjectPtr = &distributedBrokerObject;
            break;
        case SASICG_SIMULATION_ROLE_DISTRIBUTED_INTERFACE_BUS_BRIDGE_E:
            _bus = 1;
            globalObjectPtr = &distributedBusObject;
            break;
        default:
            skernelFatalError("scibInit: sasicgSimulationRole unknown role [%d]\n",sasicgSimulationRole);
    }


    maxDevicesPerSystem = maxDevNumber;

    /* Allocate array of device control structures  */
    size = maxDevNumber * sizeof(SCIB_DEVICE_DB_STC);
    scibDbPtr = malloc(size);
    if ( scibDbPtr == NULL ) {
        skernelFatalError("scibInit: scibDbPtr allocation error\n");
    }
    memset(scibDbPtr, 0, size);

    /* Allocate array of fabric adaptr control structures  */
    scibFaDbPtr = malloc(size);
    if ( scibFaDbPtr == NULL )
    {
        skernelFatalError("scibInit: scibFaDbPtr - allocation error\n");
    }
    memset(scibFaDbPtr, 0, size);
}

/**
* @internal scibBindRWMemory function
* @endinternal
*
* @brief   Bind callbacks of SKernel for R/W memory requests.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
*                                      deviceObj - pointer to the opaque for SCIB device object.
* @param[in] rwFun                    - pointer to the R/W SKernel memory CallBack function.
* @param[in] isPP                     - boolean if the bind is for PP or for FA.
* @param[in] addressCompletionStatus  - device enable/disable address completion
*
* @note if device is fa , then the id is the one of the device connected to it.
*
*/
void scibBindRWMemory
(
    IN GT_U32               deviceId,
    IN GT_U32               deviceHwId,
    IN void         *       deviceObjPtr,
    IN SCIB_RW_MEMORY_FUN   rwFun,
    IN GT_U8                isPP,
    IN GT_BOOL              addressCompletionStatus
)
{
    SCIB_DEVICE_DB_STC  *scibDevPtr;

    if(_non_dest != 1 &&  _dev != 1)
    {
        /* need to use scibRemoteInit instead of this bind ... */
        skernelFatalError("scibBindRWMemory : bad state \n");
    }

    DEV_NUM_CHECK_RANGE_MAC(deviceId,scibBindRWMemory);

    if (rwFun == NULL || deviceObjPtr == NULL)
    {
        skernelFatalError(" scibBindRWMemory : no initialization was made for the array \n");
    }

    /* Write data to the device control entry for deviceId. */
    if (isPP)
    {
        scibDevPtr = &scibDbPtr[deviceId];
    }
    else
    {
        /* fa with id of "1" is located at "0" in the scibFaDbPtr */
        scibDevPtr = &scibFaDbPtr[deviceId];
    }

    scibDevPtr->valid = GT_TRUE;
    scibDevPtr->deviceObj = deviceObjPtr;
    scibDevPtr->memAccesFun = rwFun;
    scibDevPtr->deviceHwId =  deviceHwId;
    scibDevPtr->addressCompletionStatus = addressCompletionStatus;

    scibDevPtr->origMemAccesFun = scibDevPtr->memAccesFun;
}



/**
* @internal scibBindExt function
* @endinternal
*
* @brief   extra Bind info of SKernel for the device.
*
* @param[in] deviceId                 - ID of device (as appears in the INI file).
* @param[in] bindExtInfoPtr           - pointer to more info.
*
* @note
*
*/
void scibBindExt
(
    IN GT_U32                   deviceId,
    IN SCIB_BIND_EXT_INFO_STC   *bindExtInfoPtr
)
{
    DEV_NUM_CHECK_MAC(deviceId,scibBindExt);

    if(bindExtInfoPtr->update_deviceForceBar0Bar2 == GT_TRUE)
    {
        scibDbPtr[deviceId].deviceForceBar0Bar2 = bindExtInfoPtr->deviceForceBar0Bar2;
    }
    if(bindExtInfoPtr->update_pciInfo == GT_TRUE)
    {
        /*set values only if not already have explicit set by wmDeviceOnPciBusSet */

        if(scibDbPtr[deviceId].pciBus == 0)
        {
            scibDbPtr[deviceId].pciBus  = bindExtInfoPtr->pciBus;
        }
        if(scibDbPtr[deviceId].pciDev == 0)
        {
            scibDbPtr[deviceId].pciDev  = bindExtInfoPtr->pciDev;
        }
        if(scibDbPtr[deviceId].pciFunc == 0)
        {
            scibDbPtr[deviceId].pciFunc = bindExtInfoPtr->pciFunc;
        }
    }
    if(bindExtInfoPtr->update_bar0_base == GT_TRUE)
    {
        scibDbPtr[deviceId].bar0_base[ADDR_LOW_INDEX] = bindExtInfoPtr->bar0_base;
    }
    if(bindExtInfoPtr->update_bar0_base_high == GT_TRUE)
    {
        scibDbPtr[deviceId].bar0_base[ADDR_HIGH_INDEX] = bindExtInfoPtr->bar0_base_high;
    }

    if(bindExtInfoPtr->update_bar0_size == GT_TRUE)
    {
        scibDbPtr[deviceId].bar0_size = bindExtInfoPtr->bar0_size;
    }

    if(bindExtInfoPtr->update_bar2_base == GT_TRUE)
    {
        scibDbPtr[deviceId].bar2_base[ADDR_LOW_INDEX] = bindExtInfoPtr->bar2_base;
    }
    if(bindExtInfoPtr->update_bar2_base_high == GT_TRUE)
    {
        scibDbPtr[deviceId].bar2_base[ADDR_HIGH_INDEX] = bindExtInfoPtr->bar2_base_high;
    }

    if(bindExtInfoPtr->update_bar2_size == GT_TRUE)
    {
        scibDbPtr[deviceId].bar2_size = bindExtInfoPtr->bar2_size;
    }
}

/**
* @internal scibRemoteInit function
* @endinternal
*
* @brief   init a device info in Scib , when working in distributed architecture and
*         this is the application side .
*         Asic send message to application side , and on application side this
*         function is called.
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID
* @param[in] deviceHwId               - Physical device Id.
* @param[in] interruptLine            - interrupt line of the device.
* @param[in] isPp                     - (GT_BOOL) is PP of FA
* @param[in] addressCompletionStatus  - device enable/disable address completion
*/
void scibRemoteInit
(
    IN GT_U32  deviceId,
    IN GT_U32  deviceHwId,
    IN GT_U32  interruptLine,
    IN GT_U32  isPp,
    IN GT_U32  addressCompletionStatus
)
{
    SCIB_DEVICE_DB_STC  *scibDevPtr;

    DEV_NUM_CHECK_RANGE_MAC(deviceId,scibRemoteInit);

    if(_non_dest == 1 ||  _dev == 1)
    {
        skernelFatalError("scibRemoteInit : bad state \n");
    }

    /* Write data to the device control entry for deviceId. */
    if (isPp)
    {
        scibDevPtr = &scibDbPtr[deviceId];
    }
    else
    {
        /* fa with id of "1" is located at "0" in the scibFaDbPtr */
        scibDevPtr = &scibFaDbPtr[deviceId];
    }

    scibDevPtr->valid = GT_TRUE;
    scibDevPtr->deviceObj = NULL;  /* not used */
    scibDevPtr->memAccesFun = NULL;  /* not used */
    scibDevPtr->deviceHwId = deviceHwId;
    scibDevPtr->addressCompletionStatus = (GT_BOOL)addressCompletionStatus;

    if(interruptLine != SCIB_INTERRUPT_LINE_NOT_USED_CNS)
    {
        scibSetIntLine(deviceId,interruptLine);
    }

}


/**
* @internal scibSetIntLine function
* @endinternal
*
* @brief   Set interrupt line for SKernel device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] intrline                 - number of interrupt line.
*
* @note update the FA database for fa devices and pp database for pp devices.
*
*/
void scibSetIntLine
(
    IN GT_U32               deviceId,
    IN GT_U32               intrline
)
{
    GT_U32 data;
    DEV_NUM_CHECK_MAC(deviceId,scibSetIntLine);

    /* the FA and the PP must have the same interrupt line */
    scibDbPtr[deviceId].intLine = intrline;
    scibFaDbPtr[deviceId].intLine = intrline;

    if(_non_dest == 1 ||  _dev == 1)
    {
        /* Set int line in PCI register 0x3c bits 0..7 */
        scibPciRegRead(deviceId,0x3c,1,&data);
        data &= 0xffffff00;
        data |= intrline & 0x000000ff;
        scibPciRegWrite(deviceId,0x3c,1,&data);
    }
}
/**
* @internal scibReadMemory function
* @endinternal
*
* @brief   Read memory from SKernel device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
*
* @param[out] dataPtr                  - pointer to copy read data.
*
* @note if the address is fa register then operation is done on fa object.
*
*/
void scibReadMemory
(
    IN  GT_U32        deviceId,
    IN  GT_U32        memAddr,
    IN  GT_U32        length,
    OUT GT_U32 *      dataPtr
 )
 {
    void * deviceObjPtr;
    SCIB_RW_MEMORY_FUN   rwFun;

    *dataPtr = 0xFFFFFFFF;/* assign explicit value before DEV_NUM_CHECK_MAC may 'return' on non valid device */


    if(_app)
    {
        SCIB_SEM_TAKE;
        /* only in the application side of  distributed architecture we need
           function to send the info to the other side (Asic side)*/
        registerReadWrite_distributedApplication(SCIB_MEMORY_READ_E,
                                                 deviceId,
                                                 memAddr,
                                                 length,
                                                 dataPtr);
        SCIB_SEM_SIGNAL;

        return ;
    }

     DEV_NUM_CHECK_MAC(deviceId,scibReadMemory);

    /* Find device control entry for deviceId. check if it is fa address*/
    if ((memAddr & FA_ADDRESS_BIT_CNS) &&
        (scibFaDbPtr[deviceId].valid == GT_TRUE))
    {
        deviceObjPtr = scibFaDbPtr[deviceId].deviceObj;
        rwFun = scibFaDbPtr[deviceId].memAccesFun;
    }
    else
    {
        deviceObjPtr = scibDbPtr[deviceId].deviceObj;
        rwFun = scibDbPtr[deviceId].memAccesFun;
    }

    /* Read memory from SKernel device.*/
    if (rwFun)
    {
        SCIB_SEM_TAKE;

        rwFun(SCIB_MEMORY_READ_E,
              deviceObjPtr,
              memAddr,
              length,
              dataPtr);

        SCIB_SEM_SIGNAL;
    }
    else
        memset(dataPtr, 0xff, length * sizeof(GT_U32));
 }
/*******************************************************************************
*   scibPciRegRead
*
* DESCRIPTION:
*       Read PCI registers memory from SKernel device.
*
* INPUTS:
*       deviceId  - ID of device, which is equal to PSS Core API device ID.
*       memAddr - address of first word to read.
*       length - number of words to read.
*
* OUTPUTS:
*       dataPtr - pointer to copy read data.
*
* RETURNS:
*
*
* COMMENTS:
*      Pci registers are not relevant for the fa device.
*
*******************************************************************************/
void scibPciRegRead
(
    IN  GT_U32        deviceId,
    IN  GT_U32        memAddr,
    IN  GT_U32        length,
    OUT GT_U32 *      dataPtr
 )
 {
    void * deviceObjPtr;

    *dataPtr = 0xFFFFFFFF;/* assign explicit value before DEV_NUM_CHECK_MAC may 'return' on non valid device */

    if(_app)
    {
        SCIB_SEM_TAKE;
        /* only in the application side of  distributed architecture we need
           function to send the info to the other side (Asic side)*/
        registerReadWrite_distributedApplication(SCIB_MEMORY_READ_PCI_E,
                                                 deviceId,
                                                 memAddr,
                                                 length,
                                                 dataPtr);
        SCIB_SEM_SIGNAL;
        return ;
    }

    DEV_NUM_CHECK_MAC(deviceId,scibPciRegRead);
    SCIB_SEM_TAKE;

    /* Find device control entry for deviceId. */
    deviceObjPtr = scibDbPtr[deviceId].deviceObj;

    /* Read PCI memory from SKernel device.*/
    if (scibDbPtr[deviceId].memAccesFun)
    {

        scibDbPtr[deviceId].memAccesFun( SCIB_MEMORY_READ_PCI_E,
                                         deviceObjPtr,
                                         memAddr,
                                         length,
                                         dataPtr);

    }
    else
        memset(dataPtr, 0xff, length * sizeof(GT_U32));

    SCIB_SEM_SIGNAL;
}
/**
* @internal scibWriteMemory function
* @endinternal
*
* @brief   Write to memory of a SKernel device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] dataPtr                  - pointer to copy read data.
*
* @note Pci registers are not relevant for the fa device.
*
*/
void scibWriteMemory
(
    IN  GT_U32        deviceId,
    IN  GT_U32        memAddr,
    IN  GT_U32        length,
    IN  GT_U32 *      dataPtr
)
{
    void *            deviceObjPtr;
    SCIB_RW_MEMORY_FUN   rwFun;
    GT_U32 wordIndex;


    SCIB_DEBUG_PRINT_WRITE_MAC(deviceId,memAddr,length,dataPtr);

    if(_app)
    {
        SCIB_SEM_TAKE;
        /* Loop is needed to ensure explicitly call of smemFindMemChunk function in resolution of memory word.
           This way of memory access gives ASIC ability to control internal buffer memory write */
        for(wordIndex = 0; wordIndex < length; wordIndex++, memAddr+=4)
        {
            /* only in the application side of  distributed architecture we need
               function to send the info to the other side (Asic side) */
            registerReadWrite_distributedApplication(SCIB_MEMORY_WRITE_E,
                                                     deviceId,
                                                     memAddr,
                                                     1,
                                                     &dataPtr[wordIndex]);
        }
        SCIB_SEM_SIGNAL;
        return ;
    }

    DEV_NUM_CHECK_MAC(deviceId,scibWriteMemory);

    /* Find device control entry for deviceId. check if it is fa address*/
    if ((memAddr & FA_ADDRESS_BIT_CNS) &&
        (scibFaDbPtr[deviceId].valid == GT_TRUE))
    {
        deviceObjPtr = scibFaDbPtr[deviceId].deviceObj;
        rwFun = scibFaDbPtr[deviceId].memAccesFun;
    }
    else
    {
        deviceObjPtr = scibDbPtr[deviceId].deviceObj;
        rwFun = scibDbPtr[deviceId].memAccesFun;
    }

    if (rwFun)
    {
        SCIB_SEM_TAKE;
        /* Loop is needed to ensure explicitly call of smemFindMemChunk function in resolution of memory word.
           This way of memory access gives ASIC ability to control internal buffer memory write */
        for(wordIndex = 0; wordIndex < length; wordIndex++, memAddr+=4)
        {
            /* Write memory from SKernel device.*/
            rwFun(SCIB_MEMORY_WRITE_E,
                  deviceObjPtr,
                  memAddr,
                  1,
                  &dataPtr[wordIndex]);
        }
        SCIB_SEM_SIGNAL;
    }
}
/**
* @internal scibPciRegWrite function
* @endinternal
*
* @brief   Write to PCI memory of a SKernel device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] dataPtr                  - pointer to copy read data.
*
* @note Pci registers are not relevant for the fa device.
*
*/
void scibPciRegWrite
(
    IN  GT_U32        deviceId,
    IN  GT_U32        memAddr,
    IN  GT_U32        length,
    IN  GT_U32 *      dataPtr
)
{
    void *            deviceObjPtr;

    DEV_NUM_CHECK_MAC(deviceId,scibPciRegWrite);

    if(_app)
    {
        SCIB_SEM_TAKE;
        /* only in the application side of  distributed architecture we need
           function to send the info to the other side (Asic side)*/
        registerReadWrite_distributedApplication(SCIB_MEMORY_WRITE_PCI_E,
                                                 deviceId,
                                                 memAddr,
                                                 length,
                                                 dataPtr);
        SCIB_SEM_SIGNAL;
        return ;
    }

    /* Find device control entry for deviceId. */
    deviceObjPtr = scibDbPtr[deviceId].deviceObj;

    /* Find device control entry for deviceId. */
    if (scibDbPtr[deviceId].memAccesFun)
    {
        SCIB_SEM_TAKE;
        /* Write PCI memory from SKernel device.*/
        scibDbPtr[deviceId].memAccesFun( SCIB_MEMORY_WRITE_PCI_E,
                                         deviceObjPtr,
                                         memAddr,
                                         length,
                                         dataPtr);
        SCIB_SEM_SIGNAL;
    }

}
/**
* @internal scibSetInterrupt function
* @endinternal
*
* @brief   Generate interrupt for SKernel device.
*
* @param[in] deviceId                 - ID of device.
*
* @note because the fa and the pp has the same interrupt link i chosen to use
*       only the interrupt line of pp .
*
*/
void scibSetInterrupt
(
    IN  GT_U32        deviceId
)
{
    if(_non_dest == 1 )
    {
        DEV_NUM_CHECK_MAC(deviceId,scibSetInterrupt);
    }
    else
    {
        DEV_NUM_CHECK_RANGE_MAC(deviceId,scibSetInterrupt);
    }

    if(scibDbPtr[deviceId].interruptMppTriggerFun)
    {
        scibDbPtr[deviceId].interruptMppTriggerFun(deviceId);
    }
    else
    {
        globalObjectPtr->interruptSetFun(deviceId);
    }
}
/**
* @internal scibSmiRegRead function
* @endinternal
*
* @brief   This function reads a switch's port register.
*
* @param[in] deviceId                 - Device object Id.
* @param[in] smiDev                   - Smi device to read the register for
* @param[in] regAddr                  - The register's address.
*
* @param[out] data                     - The read register's data.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
void scibSmiRegRead
(
    IN  GT_U32      deviceId,
    IN  GT_U32      smiDev,
    IN  GT_U32      regAddr,
    OUT GT_U32      *data
)
{
    GT_U32  memAddr;
    GT_U32  memData;
    GT_U32  index;

    if (smiDev > SOHO_SMI_DEV_TOTAL)
    {
        skernelFatalError("scibSmiRegWrite : smiDev%d is out of boundary \n",
                           smiDev);
    }

    if (scibAddressCompletionStatusGet(deviceId))
    {
        /* patch for Peridot */
        index = (smiDev >= SOHO_GLOBAL_REGS_START_ADDR) ? 0 /* global registers */:
                (smiDev < SOHO_PORT_REGS_START_ADDR) ? 1 /* port registers */:
                2/* PHY registers */;
    }
    else
    {
        index = (smiDev >= SOHO_GLOBAL_REGS_START_ADDR) ? 0 :
                (smiDev >= SOHO_PORT_REGS_START_ADDR) ? 1 : 2;
    }

    /* Make 32 bit word address */
    memAddr = (index << 28) | (smiDev << 16) | (regAddr << 4);

    scibReadMemory(deviceId, memAddr, 1, &memData);
    /* Read actual Smi register data */
    *data = (GT_U16)memData;
}

/**
* @internal scibSmiRegWrite function
* @endinternal
*
* @brief   This function writes to a switch's port register.
*
* @param[in] deviceId                 - Device object Id
* @param[in] smiDev                   - Smi device number to read the register for.
* @param[in] regAddr                  - The register's address.
* @param[in] data                     - The  to be written.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
void scibSmiRegWrite
(
    IN  GT_U32      deviceId,
    IN  GT_U32      smiDev,
    IN  GT_U32      regAddr,
    IN  GT_U32      data
)
{
    GT_U32  memAddr, index;

    if (smiDev > SOHO_SMI_DEV_TOTAL)
    {
        skernelFatalError("scibSmiRegWrite : smiDev%d is out of boundary \n",
                           smiDev);
    }

    if (scibAddressCompletionStatusGet(deviceId))
    {
        /* patch for Peridot */
        index = (smiDev >= SOHO_GLOBAL_REGS_START_ADDR) ? 0 /* global registers */:
                (smiDev < SOHO_PORT_REGS_START_ADDR) ? 1 /* port registers */:
                2/* PHY registers */;
    }
    else
    {
        index = (smiDev >= SOHO_GLOBAL_REGS_START_ADDR) ? 0 :
                (smiDev >= SOHO_PORT_REGS_START_ADDR) ? 1 : 2;
    }

    /* Make 32 bit word address */
    memAddr = (index << 28) | (smiDev << 16) | (regAddr << 4);

    scibWriteMemory(deviceId, memAddr, 1, (GT_U32 *)&data);
}

/**
* @internal scibGetDeviceId function
* @endinternal
*
* @brief   scans for index of the entry with given hwId
*
* @param[in] deviceHwId               - maximal number of SKernel devices in the Simulation.
*/
GT_U32 scibGetDeviceId
(
    IN  GT_U32        deviceHwId
)
{
    GT_U32  deviceId;
    GT_U32  nonPexUsage = ((deviceHwId & 0xFFFFFFE0) == 0xFFFFFFE0) ? 1 : 0;

    if(nonPexUsage)
    {
        /* called from extDrvSmiDevVendorIdGet(...) */
        deviceHwId &= 0x1F;/*0..31*/
    }

    for (deviceId = 0; deviceId < maxDevicesPerSystem; deviceId++)
    {
        if (scibDbPtr[deviceId].valid == GT_FALSE ||
            ((!nonPexUsage) && scibDbPtr[deviceId].deviceForceBar0Bar2 == GT_TRUE))/* valid device but not allowed to support this API */
        {
            continue;
        }

        if(nonPexUsage && scibDbPtr[deviceId].deviceForceBar0Bar2 == GT_TRUE)
        {
            /* the scibDbPtr[deviceId].deviceHwId belongs to the 'PEX' */
            /* and we can not compare with it ! so can only compare the 'index' in the array */
            if(deviceHwId == deviceId)
            {
                return deviceId;
            }
            else
            {
                continue;
            }
        }

        if (scibDbPtr[deviceId].deviceHwId == deviceHwId)
        {
            return deviceId;
        }
    }

    /* deviceHwId was not found */
    return SCIB_NOT_EXISTED_DEVICE_ID_CNS;
}

/**
* @internal scibAddressCompletionStatusGet function
* @endinternal
*
* @brief   Get address completion status for given hwId
*
* @param[in] devNum                   - number of SKernel devices in the Simulation.
*/
GT_BOOL  scibAddressCompletionStatusGet
(
    IN  GT_U32        devNum
)
{
    if (scibDbPtr[devNum].valid == GT_FALSE)
    {
        skernelFatalError("scibAddressCompletionStatusGet : illegal device[%ld]\n",devNum);
    }

    /* Check device type for enable/disable address completion */
    return scibDbPtr[devNum].addressCompletionStatus;
}

static GT_U32 scibDmaUpper32Bits = 0;
/**
* @internal scibDmaUpper32BitsSet function
* @endinternal
*
* @brief   set the WM for 64bits CPU , that allocated 'malloc' for DMA in
*           address >= 4G
*
* @param[in] addr_high                - the high 32bits of the DMA address.
*
*/
void scibDmaUpper32BitsSet(
    IN GT_U32   addr_high
)
{
    scibDmaUpper32Bits = addr_high;
    printf("DMA to use upper bits : [0x%8.8x] \n",addr_high);
}

/**
* @internal dmaRead function
* @endinternal
*
* @brief   read HOST CPU DMA memory function.
*
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to read .
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*
* @param[out] memPtr                   - (pointer to) PP's memory in which HOST CPU memory will be
*                                      copied.
*/
static void dmaRead
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    deviceId = deviceId;/* don't care !!! */
    dataIsWords = dataIsWords;/* don't care !!! */

    scibDmaRead64BitAddr(scibDmaUpper32Bits,address,memSize,memPtr);
}

/**
* @internal dmaWrite function
* @endinternal
*
* @brief   write to HOST CPU DMA memory function.
*
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to write .
* @param[in] memPtr                   - (pointer to) data to write to HOST CPU memory.
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*/
static void dmaWrite
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    deviceId = deviceId;/* don't care !!! */
    dataIsWords = dataIsWords;/* don't care !!! */

    scibDmaWrite64BitAddr(scibDmaUpper32Bits,address,memSize,memPtr);
}

/**
* @internal interruptSet function
* @endinternal
*
* @brief   Generate interrupt for SKernel device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
*
* @note because the fa and the pp has the same interrupt link i chosen to use
*       only the interrupt line of pp .
*
*/
static void interruptSet
(
    IN  GT_U32        deviceId
)
{

    if(_non_dest == 1 )
    {
        DEV_NUM_CHECK_MAC(deviceId,interruptSet);
    }
    else
    {
        DEV_NUM_CHECK_RANGE_MAC(deviceId,interruptSet);
    }

    /* the fa and pp must have the same interrupt line */

    /* increment intLine to fix Host bug */

    if(!wmGlobalBindAsimServices.interruptTriggerFunc)
    {
        /* Call simOs to generate interrupt for interrupt line */
        SIM_OS_MAC(simOsInterruptSet)(scibDbPtr[deviceId].intLine + 1); /* increment intLine to fix Host bug */
    }
    else
    {
        IN_GLOBAL_CB(interruptTriggerFunc);

        wmGlobalBindAsimServices.interruptTriggerFunc(
            1,    /*assert*/
            scibDbPtr[deviceId].interrupt_line   ,
            scibDbPtr[deviceId].interrupt_source ,
            scibDbPtr[deviceId].interrupt_flags);

        OUT_GLOBAL_CB(interruptTriggerFunc);
    }

}

/**
* @internal scibUnSetInterrupt function
* @endinternal
*
* @brief   give indication that the interrupt reason has ended.
*
* @param[in] deviceId                 - ID of device.
*
* @note because the fa and the pp has the same interrupt link i chosen to use
*       only the interrupt line of pp .
*
*/
void scibUnSetInterrupt
(
    IN  GT_U32        deviceId
)
{

    DEV_NUM_CHECK_MAC(deviceId,scibUnSetInterrupt);

    if(wmGlobalBindAsimServices.interruptTriggerFunc)
    {
        IN_GLOBAL_CB(interruptTriggerFunc);
        wmGlobalBindAsimServices.interruptTriggerFunc(
            0,  /*de-assert*/
            scibDbPtr[deviceId].interrupt_line   ,
            scibDbPtr[deviceId].interrupt_source ,
            scibDbPtr[deviceId].interrupt_flags);
        OUT_GLOBAL_CB(interruptTriggerFunc);
    }

}

/**
* @internal dmaRead_distributedApplication function
* @endinternal
*
* @brief   read HOST CPU DMA memory function.
*         in distributed architecture on application size
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to read .
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*
* @param[out] memPtr                   - (pointer to) PP's memory in which HOST CPU memory will be
*                                      copied.
*/
static void dmaRead_distributedApplication
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    dmaRead(deviceId,address,memSize,memPtr,dataIsWords);

    /* the caller will return the read data to the "other side" */
}

/**
* @internal dmaWrite_distributedApplication function
* @endinternal
*
* @brief   write to HOST CPU DMA memory function.
*         in distributed architecture on application size
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to write .
* @param[in] memPtr                   - (pointer to) data to write to HOST CPU memory.
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*/
static void dmaWrite_distributedApplication
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    dmaWrite(deviceId,address,memSize,memPtr,dataIsWords);
}

/**
* @internal interruptSet_distributedApplication function
* @endinternal
*
* @brief   Generate interrupt for SKernel device.
*         in distributed architecture on application size
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
*
* @note because the fa and the pp has the same interrupt link i chosen to use
*       only the interrupt line of pp .
*
*/
static void interruptSet_distributedApplication
(
    IN  GT_U32        deviceId
)
{
    interruptSet(deviceId);
}


/**
* @internal dmaRead_distributedAsic function
* @endinternal
*
* @brief   read HOST CPU DMA memory function.
*         in distributed architecture on Asic size
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to read .
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*
* @param[out] memPtr                   - (pointer to) PP's memory in which HOST CPU memory will be
*                                      copied.
*/
static void dmaRead_distributedAsic
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    simDistributedDmaRead(deviceId,address,memSize,memPtr,dataIsWords);
}

/**
* @internal dmaWrite_distributedAsic function
* @endinternal
*
* @brief   write to HOST CPU DMA memory function.
*         in distributed architecture on Asic size
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to write .
* @param[in] memPtr                   - (pointer to) data to write to HOST CPU memory.
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*/
static void dmaWrite_distributedAsic
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    simDistributedDmaWrite(deviceId,address,memSize,memPtr,dataIsWords);
}

/**
* @internal interruptSet_distributedAsic function
* @endinternal
*
* @brief   Generate interrupt for SKernel device.
*         in distributed architecture on Asic size
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
*
* @note because the fa and the pp has the same interrupt link i chosen to use
*       only the interrupt line of pp .
*
*/
static void interruptSet_distributedAsic
(
    IN  GT_U32        deviceId
)
{
    simDistributedInterruptSet(deviceId);
}


/**
* @internal registerReadWrite_distributedApplication function
* @endinternal
*
* @brief   FUNCTION of R/W Skernel memory function.
*         in distributed architecture on Application size
* @param[in] accessType               - Define access operation Read or Write.
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - Size of ASIC memory to read or write.
* @param[in,out] memPtr                   - For Write this pointer to application memory,which
*                                      will be copied to the ASIC memory .
* @param[in,out] memPtr                   - For Read this pointer to application memory in which
*                                      ASIC memory will be copied.
*/
static void registerReadWrite_distributedApplication
(
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32   deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    INOUT GT_U32 * memPtr
)
{
    switch(accessType)
    {
        case SCIB_MEMORY_READ_E:
        case SCIB_MEMORY_READ_PCI_E:
            simDistributedRegisterRead(accessType,deviceId,address,memSize,memPtr);
            break;
        case SCIB_MEMORY_WRITE_E:
        case SCIB_MEMORY_WRITE_PCI_E:
            simDistributedRegisterWrite(accessType,deviceId,address,memSize,memPtr);
            break;
        default:
            skernelFatalError("registerReadWrite_distributedApplication: unknown accessType[%d]\n", accessType);
    }
}


/**
* @internal scibDmaRead function
* @endinternal
*
* @brief   read HOST CPU DMA memory function.
*         Asic is calling this function to read DMA.
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to read .
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*
* @param[out] memPtr                   - (pointer to) PP's memory in which HOST CPU memory will be
*                                      copied.
*/
void scibDmaRead
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    if (address == 0 || memPtr == NULL)
       return;
    globalObjectPtr->dmaReadFun(deviceId,address,memSize,memPtr,dataIsWords);
}

/**
* @internal scibDmaWrite function
* @endinternal
*
* @brief   write to HOST CPU DMA memory function.
*         Asic is calling this function to write DMA.
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to write .
* @param[in] memPtr                   - (pointer to) data to write to HOST CPU memory.
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*/
void scibDmaWrite
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    if (address == 0 || memPtr == NULL)
       return;
    globalObjectPtr->dmaWriteFun(deviceId,address,memSize,memPtr,dataIsWords);
}

/**
* @internal scibAccessLock function
* @endinternal
*
* @brief   function to protect the accessing to the SCIB layer .
*         the function LOCK the access.
*         The mutex implementations allow reentrant of the locking task.
*/
void scibAccessLock(void)
{
    SIM_OS_MAC(simOsMutexLock)(simulationProtectorMtx);
    simulationProtectorMtx_count++;
}


/**
* @internal scibAccessUnlock function
* @endinternal
*
* @brief   function to protect the accessing to the SCIB layer .
*         the function UN-LOCK the access.
*         The mutex implementations allow reentrant of the locking task.
*/
void scibAccessUnlock(void)
{
    simulationProtectorMtx_count--;
    SIM_OS_MAC(simOsMutexUnlock)(simulationProtectorMtx);
}

static void nonExistsDeviceAccess
(
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN void * devObjPtr,
    IN GT_U32 address,
    IN GT_U32 memSize,
    INOUT GT_U32 * memPtr
)
{
    accessType = accessType;
    devObjPtr = devObjPtr;
    address = address;

    if(!IS_WRITE_OPERATION_MAC(accessType))
    {
        memset(memPtr, 0xff, memSize * sizeof(GT_U32));
    }

    return;
}


/**
* @internal scibUnBindDevice function
* @endinternal
*
* @brief   unBind the BUS from read/write register functions
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
*
* @note accessing to the 'read'/'write' registers will cause fatal error.
*
*/
void scibUnBindDevice
(
    IN GT_U32               deviceId
)
{
    DEV_NUM_CHECK_MAC(deviceId,scibUnBindDevice);

    SCIB_SEM_TAKE;
    if(scibDbPtr[deviceId].memAccesFun == NULL)
    {
        skernelFatalError(" scibUnBindDevice : device id %d not bound \n",deviceId);
    }

    scibDbPtr[deviceId].memAccesFun = nonExistsDeviceAccess;
    SCIB_SEM_SIGNAL;
}

/**
* @internal scibReBindDevice function
* @endinternal
*
* @brief   re-bind the BUS to read/write register functions
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
*/
void scibReBindDevice
(
    IN GT_U32               deviceId
)
{
    DEV_NUM_CHECK_MAC(deviceId,scibReBindDevice);

    SCIB_SEM_TAKE;
    if(scibDbPtr[deviceId].memAccesFun == NULL)
    {
        skernelFatalError(" scibReBindDevice : device id %d was never bound , so can't re-bind \n",deviceId);
    }

    scibDbPtr[deviceId].memAccesFun = scibDbPtr[deviceId].origMemAccesFun;
    SCIB_SEM_SIGNAL;
}


/**
* @internal scibDebugPrint function
* @endinternal
*
* @brief   allow printings of the registers settings
*/
void scibDebugPrint(GT_BOOL   enable)
{
    debugPrint_enable = enable;
}

/**
* @internal scibPortLoopbackForceModeSet function
* @endinternal
*
* @brief   the function set the 'loopback force mode' on a port of device.
*         this function needed for devices that not support loopback on the ports
*         and need 'external support'
* @param[in] deviceId                 - the simulation device Id .
* @param[in] portNum                  - the physical port number .
* @param[in] mode                     - the loopback force mode.
*                                      0 - SKERNEL_PORT_LOOPBACK_NOT_FORCED_E,
*                                      1 - SKERNEL_PORT_LOOPBACK_FORCE_ENABLE_E,
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad portNum or mode
*
* @note function do fatal error on non-exists device or out of range device.
*
*/
GT_STATUS scibPortLoopbackForceModeSet
(
    IN  GT_U32                      deviceId,
    IN  GT_U32                      portNum,
    IN  GT_U32                      mode
)
{
    GT_STATUS   rc;

    DEV_NUM_CHECK_MAC_WITH_RETURN_STATUS(deviceId,scibPortLoopbackForceModeSet);

    if(_app)
    {
        rc = GT_NOT_IMPLEMENTED;
        return rc;
    }

    SCIB_SEM_TAKE;

    rc = skernelPortLoopbackForceModeSet(deviceId,portNum,mode);

    SCIB_SEM_SIGNAL;

    return rc;
}

/**
* @internal scibPortLinkStateSet function
* @endinternal
*
* @brief   the function set the 'link state' on a port of device.
*         this function needed for devices that not support 'link change' from the
*         'MAC registers' of the ports.
*         this is relevant to 'GM devices'
* @param[in] deviceId                 - the simulation device Id .
* @param[in] portNum                  - the physical port number .
* @param[in] linkState                - the link state to set for the port.
*                                      GT_TRUE  - force 'link UP'
*                                      GT_FALSE - force 'link down'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad portNum or mode
*
* @note function do fatal error on non-exists device or out of range device.
*
*/
GT_STATUS scibPortLinkStateSet
(
    IN  GT_U32                      deviceId,
    IN  GT_U32                      portNum,
    IN GT_BOOL                      linkState
)
{
    GT_STATUS   rc;

    DEV_NUM_CHECK_MAC_WITH_RETURN_STATUS(deviceId,scibPortLinkStateSet);

    if(_app)
    {
        rc = GT_NOT_IMPLEMENTED;
        return rc;
    }

    SCIB_SEM_TAKE;

    rc = skernelPortLinkStateSet(deviceId,portNum,linkState);

    SCIB_SEM_SIGNAL;

    return rc;
}

/**
* @internal scibMemoryClientRegRead function
* @endinternal
*
* @brief   Generic read of SCIB client registers memory from SKernel device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] scibClient               - memory access client: PCI/Core/DFX
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
*
* @param[out] dataPtr                  - pointer to copy read data.
*                                       None
*/
void scibMemoryClientRegRead
(
    IN  GT_U32                  deviceId,
    IN  SCIB_MEM_ACCESS_CLIENT  scibClient,
    IN  GT_U32                  memAddr,
    IN  GT_U32                  length,
    OUT GT_U32 *                dataPtr
)
{
    void * deviceObjPtr;
    SCIB_MEMORY_ACCESS_TYPE accessType;

    *dataPtr = 0xFFFFFFFF;/* assign explicit value before DEV_NUM_CHECK_MAC may 'return' on non valid device */
    DEV_NUM_CHECK_MAC(deviceId, scibMemoryClientRegRead);

    /* Find device control entry for deviceId. */
    deviceObjPtr = scibDbPtr[deviceId].deviceObj;

    switch(scibClient)
    {
        case SCIB_MEM_ACCESS_PCI_E:
            accessType = SCIB_MEMORY_READ_PCI_E;
            break;
        case SCIB_MEM_ACCESS_DFX_E:
            accessType = SCIB_MEMORY_READ_DFX_E;
            break;
        case SCIB_MEM_ACCESS_CORE_E:
            accessType = SCIB_MEMORY_READ_E;
            break;
        case SCIB_MEM_ACCESS_BAR0_E:
            accessType = SCIB_MEMORY_READ_BAR0_E;
            break;
        case SCIB_MEM_ACCESS_BAR2_E:
            accessType = SCIB_MEMORY_READ_BAR2_E;
            break;
        default:
            accessType = SCIB_MEMORY_LAST_E;
            skernelFatalError("scibMemoryClientRegRead: illegal client type %lu\n", scibClient);
    }

    /* Read DFX memory from SKernel device.*/
    if (scibDbPtr[deviceId].memAccesFun)
    {
        SCIB_SEM_TAKE;

        scibDbPtr[deviceId].memAccesFun(accessType,
                                        deviceObjPtr,
                                        memAddr,
                                        length,
                                        dataPtr);

        SCIB_SEM_SIGNAL;
    }
    else
        memset(dataPtr, 0xff, length * sizeof(GT_U32));
}

/**
* @internal scibMemoryClientRegWrite function
* @endinternal
*
* @brief   Generic write to registers of SKernel device according to SCIB client.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] dataPtr                  - pointer to copy read data.
*                                       None
*/
void scibMemoryClientRegWrite
(
    IN  GT_U32                  deviceId,
    IN  SCIB_MEM_ACCESS_CLIENT  scibClient,
    IN  GT_U32                  memAddr,
    IN  GT_U32                  length,
    IN  GT_U32 *                dataPtr
)
{
    void * deviceObjPtr;
    SCIB_MEMORY_ACCESS_TYPE accessType;

    DEV_NUM_CHECK_MAC(deviceId, scibMemoryClientRegWrite);

    /* Find device control entry for deviceId. */
    deviceObjPtr = scibDbPtr[deviceId].deviceObj;

    switch(scibClient)
    {
        case SCIB_MEM_ACCESS_PCI_E:
            accessType = SCIB_MEMORY_WRITE_PCI_E;
            break;
        case SCIB_MEM_ACCESS_DFX_E:
            accessType = SCIB_MEMORY_WRITE_DFX_E;
            break;
        case SCIB_MEM_ACCESS_CORE_E:
            accessType = SCIB_MEMORY_WRITE_E;
            break;
        case SCIB_MEM_ACCESS_BAR0_E:
            accessType = SCIB_MEMORY_WRITE_BAR0_E;
            break;
        case SCIB_MEM_ACCESS_BAR2_E:
            accessType = SCIB_MEMORY_WRITE_BAR2_E;
            break;
        default:
            accessType = SCIB_MEMORY_LAST_E;
            skernelFatalError("scibMemoryClientRegWrite: illegal client type %lu\n", scibClient);
    }

    /* Read DFX memory from SKernel device.*/
    if (scibDbPtr[deviceId].memAccesFun)
    {
        SCIB_SEM_TAKE;

        scibDbPtr[deviceId].memAccesFun(accessType,
                                        deviceObjPtr,
                                        memAddr,
                                        length,
                                        dataPtr);

        SCIB_SEM_SIGNAL;
    }
}

/**
* @internal scibRegister_0x4c_Update function
* @endinternal
*
* @brief   Update the value of the <DeviceID> and <RevisionID> in register 0x4C.
*
* @param[in] deviceId                 - the device id as appear in the INI file
* @param[in] DeviceID_field           - 16 bits value of <DeviceID>.
*                                      value SMAIN_NOT_VALID_CNS means ignored
* @param[in] RevisionID_field         - 4 bits value of <RevisionID>
*                                      value SMAIN_NOT_VALID_CNS means ignored
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS scibRegister_0x4c_Update
(
    IN GT_U32                  deviceId,
    IN GT_U32                  DeviceID_field,
    IN GT_U32                  RevisionID_field
)
{
    GT_STATUS   rc;
    void * deviceObjPtr;

    DEV_NUM_CHECK_MAC_WITH_RETURN_STATUS(deviceId, scibRegister_0x4c_Update);

    /* Find device control entry for deviceId. */
    deviceObjPtr = scibDbPtr[deviceId].deviceObj;

    SCIB_SEM_TAKE;

    rc = skernelRegister_0x4c_Update(deviceObjPtr,DeviceID_field,RevisionID_field);

    SCIB_SEM_SIGNAL;

    return rc;
}

/**
* @internal scibBindInterruptMppTrigger function
* @endinternal
*
* @brief   Bind callback of device to be called in interrupt instead of to trigger the CPU.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] interruptTriggerFun      - the CallBack function.
*/
void scibBindInterruptMppTrigger
(
    IN GT_U32               deviceId,
    IN SCIB_INTERRUPT_SET_FUN interruptTriggerFun
)
{
    DEV_NUM_CHECK_MAC(deviceId, scibBindInterruptMppTrigger);

    scibDbPtr[deviceId].interruptMppTriggerFun = interruptTriggerFun;
}


#define   SCIB_BUS_INTERFACE_PEX    (1<<1)
#define   SCIB_BUS_INTERFACE_SMI    (1<<2)
/**
* @internal scibBusInterfaceGet function
* @endinternal
*
* @brief   get the bus interface(s) that the device is connected to.
*
* @param[in] deviceId                 - the device id as appear in the INI file
* @param[out] interfaceBmpPtr         - (pointer to)the BMP of interfaces that
*                                        the device connected to
*                                       (combination of :SCIB_BUS_INTERFACE_PEX ,
*                                       SCIB_BUS_INTERFACE_SMI,
*                                       SCIB_BUS_INTERFACE_SMI_INDIRECT)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS scibBusInterfaceGet
(
    IN GT_U32                  deviceId,
    OUT GT_U32                *interfaceBmpPtr
)
{
    GT_STATUS   rc;
    void * deviceObjPtr;

    DEV_NUM_CHECK_MAC_WITH_RETURN_STATUS(deviceId, scibBusInterfaceGet);

    /* Find device control entry for deviceId. */
    deviceObjPtr = scibDbPtr[deviceId].deviceObj;

    SCIB_SEM_TAKE;

    rc = skernelBusInterfaceGet(deviceObjPtr,interfaceBmpPtr);

    if(rc == GT_OK)
    {
        /* remove the forbidden interfaces */
        *interfaceBmpPtr &= ~scibDbPtr[deviceId].forbiddenInterfaceBmp;
    }

    SCIB_SEM_SIGNAL;

    return rc;
}

/**
* @internal scibBusInterfaceAllowedSet function
* @endinternal
*
* @brief   state the which interface(s) will be able to show in scibBusInterfaceGet(...)
*          (meaning that the device maybe connected to the SMI , but we want to
*          'hide' the device from the 'SMI scan')
*
* @param[in] deviceId                 - the device id as appear in the INI file
* @param[out] allowedInterfaceBmp     - the BMP of interfaces that the device
*                                       allow in scibBusInterfaceGet(...).
*                                       (combination of :SCIB_BUS_INTERFACE_PEX ,
*                                       SCIB_BUS_INTERFACE_SMI,
*                                       SCIB_BUS_INTERFACE_SMI_INDIRECT,
*                                       Value of 0xFFFFFFFF - means 'no limitations'
*                                           --> default behavior
*                                       Value of 0 - means the 'device hide' will
*                                           no be seen by scibBusInterfaceGet(...)
*                                           that used by 'SMI scan')
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS scibBusInterfaceAllowedSet
(
    IN GT_U32                  deviceId,
    IN GT_U32                  allowedInterfaceBmp
)
{
    GT_U32  forbiddenInterfaceBmp;

    DEV_NUM_CHECK_MAC_WITH_RETURN_STATUS(deviceId, scibBusInterfaceAllowedSet);

    /* Find device control entry for deviceId. */
    forbiddenInterfaceBmp = ~allowedInterfaceBmp;

    SCIB_SEM_TAKE;

    scibDbPtr[deviceId].forbiddenInterfaceBmp = forbiddenInterfaceBmp;

    SCIB_SEM_SIGNAL;

    return GT_OK;


}


/**
* @internal wmMemPciConfigSpaceRead function
* @endinternal
*
* @brief   WM memory function to read register(s) from the PCIe configuration space.
*
* @param[in] pciBus      - the bus ID.
* @param[in] pciDev      - the device ID on the bus.
* @param[in] pciFunc     - the function ID in the device.
* @param[in] addr        - the address in the configuration space.
* @param[out] valuePtr   - (pointer to) the read value
*
*/
WM_STATUS wmMemPciConfigSpaceRead(
    IN  GT_U32 pciBus,
    IN  GT_U32 pciDev,
    IN  GT_U32 pciFunc,
    IN  GT_U32 addr,
    OUT GT_U32 *valuePtr
)
{
    GT_U32 wmDeviceId;
    WM_STATUS rc;

    rc = wmDeviceOnPciBusGet(pciBus,pciDev,pciFunc,&wmDeviceId);
    if(rc != WM_OK)
    {
        return rc;
    }

    scibPciRegRead(wmDeviceId,addr,1,valuePtr);

    return WM_OK;
}

/**
* @internal wmMemPciConfigSpaceWrite function
* @endinternal
*
* @brief   WM memory function to write register(s) to the PCIe configuration space.
*
* @param[in] pciBus      - the bus ID.
* @param[in] pciDev      - the device ID on the bus.
* @param[in] pciFunc     - the function ID in the device.
* @param[in] addr        - the address in the configuration space.
* @param[in] value       - the value to write
*
*/
WM_STATUS wmMemPciConfigSpaceWrite(
    IN GT_U32 pciBus,
    IN GT_U32 pciDev,
    IN GT_U32 pciFunc,
    IN GT_U32 addr,
    IN GT_U32 value
)
{
    GT_U32 wmDeviceId;
    WM_STATUS rc;

    rc = wmDeviceOnPciBusGet(pciBus,pciDev,pciFunc,&wmDeviceId);
    if(rc != WM_OK)
    {
        return rc;
    }

    scibPciRegWrite(wmDeviceId,addr,1,&value);

    return WM_OK;
}

static  WM_STATUS  pexAddrToDevAnBarIdConvert
(
    IN GT_U32  pexAddrHigh,
    IN GT_U32  pexAddrLow,
    OUT GT_U32 *wmDeviceIdPtr,
    OUT SCIB_MEM_ACCESS_CLIENT  *scibClientPtr
)
{
    GT_U32  wmDeviceId;
    GT_U32  existWmDeviceId = 0xFFFFFFFF;

    for(wmDeviceId = 0 ; wmDeviceId < maxDevicesPerSystem; wmDeviceId++)
    {
        if (scibDbPtr[wmDeviceId].deviceObj == NULL)
        {
            continue;
        }

        if(existWmDeviceId == 0xFFFFFFFF)
        {
            existWmDeviceId = wmDeviceId;
        }

        if( pexAddrHigh == scibDbPtr[wmDeviceId].bar0_base[ADDR_HIGH_INDEX])
        {
            if( pexAddrLow >= scibDbPtr[wmDeviceId].bar0_base[ADDR_LOW_INDEX]  &&
                pexAddrLow <  scibDbPtr[wmDeviceId].bar0_base[ADDR_LOW_INDEX] + scibDbPtr[wmDeviceId].bar0_size)
            {
                *wmDeviceIdPtr = wmDeviceId;
                *scibClientPtr = SCIB_MEM_ACCESS_BAR0_E;
                return WM_OK;
            }
        }

        if( pexAddrHigh == scibDbPtr[wmDeviceId].bar2_base[ADDR_HIGH_INDEX])
        {
            if( pexAddrLow >= scibDbPtr[wmDeviceId].bar2_base[ADDR_LOW_INDEX]  &&
                pexAddrLow <  scibDbPtr[wmDeviceId].bar2_base[ADDR_LOW_INDEX] + scibDbPtr[wmDeviceId].bar2_size)
            {
                *wmDeviceIdPtr = wmDeviceId;
                *scibClientPtr = SCIB_MEM_ACCESS_BAR2_E;
                return WM_OK;
            }
        }
    }

    wmDeviceId = existWmDeviceId;

    if(wmDeviceId != 0xFFFFFFFF)
    {
        void* devObjPtr = scibDbPtr[wmDeviceId].deviceObj;/*needed by MACRO:__LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK*/
        if(simLogIsOpenFlag && devObjPtr)
        {
            __LOG_NO_LOCATION_META_DATA(("PCI address no match : ERROR : addr_high[0x%x]addr_low[0x%8.8x] : OUT of BAR0 addr_high[0x%x]addr_low[0x%8.8x](size[%d]MB) , BAR2 addr_high[0x%x]addr_low[0x%8.8x] (size[%d]MB)\n",
                pexAddrHigh,
                pexAddrLow,
                scibDbPtr[wmDeviceId].bar0_base[ADDR_HIGH_INDEX],
                scibDbPtr[wmDeviceId].bar0_base[ADDR_LOW_INDEX],
                scibDbPtr[wmDeviceId].bar0_size>>20,
                scibDbPtr[wmDeviceId].bar2_base[ADDR_HIGH_INDEX],
                scibDbPtr[wmDeviceId].bar2_base[ADDR_LOW_INDEX],
                scibDbPtr[wmDeviceId].bar2_size>>20));
        }
    }
    else if(simLogIsOpenFlag) /* no device exists */
    {
        simLogInfoSave(NULL,NULL,0,NULL,SIM_LOG_INFO_TYPE_MEMORY_E);
        simLogInternalLog("PCI address no match : ERROR : no device exists to reply to : addr_high[0x%x]addr_low[0x%8.8x] \n",
            pexAddrHigh,
            pexAddrLow);
    }

    return WM_ERROR;
}

/**
* @internal wmMemPciRead function
* @endinternal
*
* @brief   WM memory function to read register(s) from the device via PCIe 'bus' on address that
*           match one of the 'BARs'.
*
* @param[in] addr_high                - the high 32bits of the address.
* @param[in] addr_low                 - the low  32bits of the address.
* @param[in] numOfWords               - number of consecutive 32bits words to read from the address
* @param[out] wordsArray[]            - array of words fill with the read values
*
*/
WM_STATUS wmMemPciRead(
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    OUT GT_U32  wordsArray[]/* according to numOfWords */
)
{
    WM_STATUS               rc;
    GT_U32                  wmDeviceId;
    SCIB_MEM_ACCESS_CLIENT  scibClient;
    GT_U32                  ii;

    SCIB_SEM_TAKE;
    rc = pexAddrToDevAnBarIdConvert(addr_high,addr_low,&wmDeviceId,&scibClient);
    if(rc != WM_OK)
    {
        SCIB_SEM_SIGNAL;

        for(ii = 0 ; ii < numOfWords; ii++)
        {
            wordsArray[ii]=0xFFFFFFFF;
        }
        return rc;
    }

    scibMemoryClientRegRead(wmDeviceId,
        scibClient,
        addr_low,
        numOfWords,
        wordsArray);

    SCIB_SEM_SIGNAL;

    return WM_OK;
}

/**
* @internal wmMemPciWrite function
* @endinternal
*
* @brief   WM memory function to write register(s) to the device via PCIe 'bus' on address that
*           match one of the 'BARs'
*
* @param[in] addr_high                - the high 32bits of the address.
* @param[in] addr_low                 - the low  32bits of the address.
* @param[in] numOfWords               - number of consecutive 32bits words to write from the address
* @param[in] wordsArray[]             - array of words fill with the values to write
*
*/
WM_STATUS wmMemPciWrite(
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    IN GT_U32   wordsArray[]/* according to numOfWords */
)
{
    WM_STATUS               rc;
    GT_U32                  wmDeviceId;
    SCIB_MEM_ACCESS_CLIENT  scibClient;

    SCIB_SEM_TAKE;
    rc = pexAddrToDevAnBarIdConvert(addr_high,addr_low,&wmDeviceId,&scibClient);
    if(rc != WM_OK)
    {
        SCIB_SEM_SIGNAL;
        return rc;
    }

    scibMemoryClientRegWrite(wmDeviceId,
        scibClient,
        addr_low,
        numOfWords,
        wordsArray);

    SCIB_SEM_SIGNAL;

    return WM_OK;
}


/**
* @internal wmMemSmiRead function
* @endinternal
*
* @brief   WM memory function to read register from the device via SMI bus.
*
* @param[in] smiBusId                - the SMI bus ID.
* @param[in] smiAddr                 - the address on the SMI bus.
* @param[out] valuePtr               - (pointer to) the value that was read
*
*/
WM_STATUS wmMemSmiRead(
    IN GT_U32   smiBusId,
    IN GT_U32   smiAddr,
    OUT GT_U16  *valuePtr
)
{
    /* not implemented */
    return WM_ERROR;
}

/**
* @internal wmMemSmiWrite function
* @endinternal
*
* @brief   WM memory function to write register to the device via SMI bus.
*
* @param[in] smiBusId                - the SMI bus ID.
* @param[in] smiAddr                 - the address on the SMI bus.
* @param[in] value                   - the value to write
*
*/
WM_STATUS wmMemSmiWrite(
    IN GT_U32   smiBusId,
    IN GT_U32   smiAddr,
    IN GT_U16   value
)
{
    /* not implemented */
    return WM_ERROR;
}

extern WM_STATUS smainTrafficIngressPacket(
    IN GT_U32   deviceId,
    IN GT_U32   portNum,
    IN GT_U32   numOfBytes,
    IN char*    packetPtr /* pointer to start of packet (network order) according to 'numOfBytes' */
);

/**
* @internal wmTrafficIngressPacket function
* @endinternal
*
* @brief   WM traffic function to ingress packet into a port in a device for processing.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
* @param[in] portNum                 - the WM device ingress port number (MAC number).
* @param[in] numOfBytes              - the munber of bytes in the packet.
* @param[in] packetPtr               - (pointer to) array of bytes of the packet
*
*   NOTE:
*       0. if the port hold SLAN , this function should NOT be called (but the WM will process it !)
*       1. the caller is responsible to free the packet memory.
*       2. the caller to this function may free the memory of the packet as soon
*       as the function ends.
*       3. the function responsible to 'copy' the packet for it's own usage , to it's own memory.
*/
WM_STATUS wmTrafficIngressPacket(
    IN GT_U32   wmDeviceId,
    IN GT_U32   portNum,
    IN GT_U32   numOfBytes,
    IN char*    packetPtr /* pointer to start of packet (network order) according to 'numOfBytes' */
)
{
    void* devObjPtr = scibDbPtr[wmDeviceId].deviceObj;/*needed by MACRO:__LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK*/
    if(simLogIsOpenFlag && devObjPtr)
    {
        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("Ingress port [%d] :ingress packet : numOfBytes[%d] (from ASIM Env.) call WM to process \n",
            portNum,
            numOfBytes));
    }
    return smainTrafficIngressPacket(wmDeviceId,portNum,numOfBytes,packetPtr);
}

/**
* @internal wmTrafficEgressPacket function
* @endinternal
*
* @brief   function to egress packet from a port in a device via 'non-slan' transport layer.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
* @param[in] portNum                 - the WM device egress port number (MAC number).
* @param[in] numOfBytes              - the munber of bytes in the packet.
* @param[in] packetPtr               - (pointer to) array of bytes of the packet
*
*   NOTE:
*       0. if the port hold SLAN , this CB will NOT be called , and the SLAN will get the packet !
*       1. the caller is responsible to free the packet memory.
*       2. the caller to this function may free the memory of the packet as soon
*       as the function ends.
*       3. the function responsible to 'copy' the packet for it's own usage , to it's own memory.
*/
WM_STATUS wmTrafficEgressPacket(
    IN GT_U32   wmDeviceId,
    IN GT_U32   portNum,
    IN GT_U32   numOfBytes,
    IN char*    packetPtr /* pointer to start of packet (network order) according to 'numOfBytes' */
)
{
    if(wmGlobalBindAsimServices.egressPacketFunc)
    {
        void* devObjPtr = scibDbPtr[wmDeviceId].deviceObj;/*needed by MACRO:__LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK*/
        if(simLogIsOpenFlag && devObjPtr)
        {
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("Egress port  [%d] : egress packet : numOfBytes[%d] send to 'egress packet call back' (to ASIM Env.) \n",
                portNum,numOfBytes));
        }

        IN_GLOBAL_CB(egressPacketFunc);
        (void)wmGlobalBindAsimServices.egressPacketFunc(wmDeviceId,portNum,numOfBytes,packetPtr);
        OUT_GLOBAL_CB(egressPacketFunc);
    }

    return WM_OK;
}
typedef unsigned long long sim_uint64_t;
/**
* @internal scibDmaRead64BitAddr function
* @endinternal
*
* @brief   prototype for function that allow the device to read DMA (DRAM or other memory) via PCIe 'bus'
*
* @param[in] addr_high                - the high 32bits of the address.
* @param[in] addr_low                 - the low  32bits of the address.
* @param[in] numOfWords               - number of consecutive 32bits words to read from the address
* @param[out] wordsArray[]            - array of words fill with the read values
*
*/
void scibDmaRead64BitAddr(
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    OUT GT_U32  wordsArray[]/* according to numOfWords */
)
{
    if(wmGlobalBindAsimServices.dmaReadFunc)
    {
        IN_GLOBAL_CB(dmaReadFunc);
        (void)wmGlobalBindAsimServices.dmaReadFunc(addr_high,addr_low,numOfWords,wordsArray);
        OUT_GLOBAL_CB(dmaReadFunc);
    }
    else
    {
#if __WORDSIZE == 64
        sim_uint64_t pexAddr64;
        /* build a 64 bit address */
        pexAddr64 = (sim_uint64_t)addr_low |  (((sim_uint64_t)addr_high) << 32);
#else
        GT_U32 pexAddr64  = addr_low;
        addr_high = addr_high;/* not used ... */
#endif
        memcpy((void*)wordsArray,
               (void*)(GT_UINTPTR)pexAddr64,
               numOfWords * sizeof(GT_U32));

    }

    return ;
}


/**
* @internal scibDmaWrite64BitAddr function
* @endinternal
*
* @brief   prototype for function that allow the device to write DMA (DRAM or other memory) via PCIe 'bus'
*
* @param[in] addr_high                - the high 32bits of the address.
* @param[in] addr_low                 - the low  32bits of the address.
* @param[in] numOfWords               - number of consecutive 32bits words to write from the address
* @param[in] wordsArray[]             - array of words fill with the values to write
*
*/
void scibDmaWrite64BitAddr(
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    IN GT_U32  wordsArray[]/* according to numOfWords */
)
{
    if(wmGlobalBindAsimServices.dmaWriteFunc)
    {
        IN_GLOBAL_CB(dmaWriteFunc);
        (void)wmGlobalBindAsimServices.dmaWriteFunc(addr_high,addr_low,numOfWords,wordsArray);
        OUT_GLOBAL_CB(dmaWriteFunc);
    }
    else
    {
#if __WORDSIZE == 64
        sim_uint64_t pexAddr64;
        /* build a 64 bit address */
        pexAddr64 = (sim_uint64_t)addr_low |  (((sim_uint64_t)addr_high) << 32);
#else
        GT_U32 pexAddr64  = addr_low;
        addr_high = addr_high;/* not used ... */
#endif
        memcpy((void*)(GT_UINTPTR)pexAddr64,
               (void*)wordsArray,
               numOfWords * sizeof(GT_U32));

    }

    return;
}

/**
* @internal wmBindCbFunctions function
* @endinternal
*
* @brief   function to bind the WM with call back (CB) functions.
*
* @param[in] cbFuncInfoPtr            - (pointer to) info about the CB functions needed by the device.
*
*/
WM_STATUS wmBindCbFunctions(
    IN  WM_BIND_CB_FUNC_STC *cbFuncInfoPtr
)
{
    if(cbFuncInfoPtr == NULL)
    {
        return WM_ERROR;
    }

    wmGlobalBindAsimServices = *cbFuncInfoPtr;

    /* create the watchdog task for those CB functions */
    scibInitCbFuncWatchDogTask();

    return WM_OK;
}

/**
* @internal wmDeviceOnPciBusGet function
* @endinternal
*
* @brief   The function check if the {pciBus,pciDev,pciFunc} are associated with
*          any of the WM devices.
*          by default this association done by the parameters in the INI file,
*          but can be set in runtime by 'wmDeviceOfPciBusSet(...)'
*
* @param[in] pciBus         - the bus ID.
* @param[in] pciDev         - the device ID on the bus.
* @param[in] pciFunc        - the function ID in the device.
* @param[out] wmDeviceIdPtr - (pointer to) the WM deviceId if found.
*                             if not found the value is 0xFFFFFFFF
*
*/
WM_STATUS wmDeviceOnPciBusGet(
    IN  GT_U32 pciBus,
    IN  GT_U32 pciDev,
    IN  GT_U32 pciFunc,
    OUT GT_U32 *wmDeviceIdPtr
)
{
    GT_U32 wmDeviceId;

    SCIB_SEM_TAKE;
    for(wmDeviceId = 0 ; wmDeviceId < maxDevicesPerSystem; wmDeviceId++)
    {
        if (scibDbPtr[wmDeviceId].deviceObj == NULL)
        {
            continue;
        }

        if( scibDbPtr[wmDeviceId].pciBus  == pciBus   &&
            scibDbPtr[wmDeviceId].pciDev  == pciDev   &&
            scibDbPtr[wmDeviceId].pciFunc == pciFunc  )
        {
            *wmDeviceIdPtr = wmDeviceId;
            SCIB_SEM_SIGNAL;
            return WM_OK;
        }
    }

    SCIB_SEM_SIGNAL;
    return WM_ERROR;
}

/**
* @internal wmDeviceOnPciBusSet function
* @endinternal
*
* @brief   The function associate te WM device with the {pciBus,pciDev,pciFunc} .
*          by default this association done by the parameters in the INI file.
*
* @param[in] pciBus         - the bus ID.
* @param[in] pciDev         - the device ID on the bus.
* @param[in] pciFunc        - the function ID in the device.
* @param[out] wmDeviceId    - the WM deviceId
*
*/
WM_STATUS wmDeviceOnPciBusSet(
    IN  GT_U32 wmDeviceId,
    IN  GT_U32 pciBus,
    IN  GT_U32 pciDev,
    IN  GT_U32 pciFunc
)
{
    DEV_NUM_CHECK_RANGE_MAC(wmDeviceId,wmDeviceOnPciBusSet);

    SCIB_SEM_TAKE;
    if (scibDbPtr[wmDeviceId].deviceObj == NULL)
    {
        SCIB_SEM_SIGNAL;
        return WM_ERROR;
    }

    scibDbPtr[wmDeviceId].pciBus  = pciBus ;
    scibDbPtr[wmDeviceId].pciDev  = pciDev ;
    scibDbPtr[wmDeviceId].pciFunc = pciFunc;
    SCIB_SEM_SIGNAL;

    return WM_OK;
}

/**
* @internal wmDeviceIrqParamsSet function
* @endinternal
*
* @brief   The function associate the WM device with the IRQ info {line,source,flags} ,
*          by default 'line' comes from the INI file. (others are ZERO)
*          those params will be called by this device on CB function of 'interruptTriggerFunc'
*
* @param[in] wmDeviceId    - the WM deviceId
* @param[in] line          - The interrupt line being asserted.
* @param[in] source        - The source core when line < 32
* @param[in] flags         - Sideband signals from the device to the interrupt controller
*
*/
WM_STATUS wmDeviceIrqParamsSet(
    IN  GT_U32 wmDeviceId,
    IN  GT_U32 line,
    IN  GT_U32 source,
    IN  GT_U32 flags
)
{
    DEV_NUM_CHECK_RANGE_MAC(wmDeviceId,wmDeviceIrqParamsSet);

    SCIB_SEM_TAKE;
    if (scibDbPtr[wmDeviceId].deviceObj == NULL)
    {
        SCIB_SEM_SIGNAL;
        return WM_ERROR;
    }

    scibDbPtr[wmDeviceId].interrupt_line    = line   ;
    scibDbPtr[wmDeviceId].interrupt_source  = source ;
    scibDbPtr[wmDeviceId].interrupt_flags   = flags  ;
    SCIB_SEM_SIGNAL;

    /* update the PCI config space */
    scibSetIntLine(wmDeviceId,line);

    return WM_OK;
}

/**
* @internal wmPortConnectionInfo function
* @endinternal
*
* @brief   WM function to state the port connection info.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
* @param[in] portNum                 - the WM device ingress port number (MAC number).
* @param[in] infoPtr                 - (pointer to) info about the port connection
*
*/
WM_STATUS wmPortConnectionInfo(
    IN GT_U32   wmDeviceId,
    IN GT_U32   portNum,
    IN WM_PORT_CONNECTION_INFO_STC  *infoPtr
)
{
    WM_STATUS   rc;
    DEV_NUM_CHECK_RANGE_MAC(wmDeviceId,wmPortConnectionInfo);

    SCIB_SEM_TAKE;
    if (scibDbPtr[wmDeviceId].deviceObj == NULL)
    {
        SCIB_SEM_SIGNAL;
        return WM_ERROR;
    }

    /* update the PCI config space */
    rc = skernelPortConnectionInfo(scibDbPtr[wmDeviceId].deviceObj,portNum,infoPtr);

    SCIB_SEM_SIGNAL;

    return rc;
}

/**
* @internal scibRemoteTraceInfo function
* @endinternal
*
* @brief   Function to get trace strings that WM wants to log.
*
* @param[in] str                   - string to log
*
* @retval WM_OK                    - on success
* @retval WM_FAIL                  - on error
*/
void scibRemoteTraceInfo
(
    IN const char *str
)
{
    if(wmGlobalBindAsimServices.traceLogFunc)
    {
        IN_GLOBAL_CB(traceLogFunc);
        wmGlobalBindAsimServices.traceLogFunc(str);
        OUT_GLOBAL_CB(traceLogFunc);
    }
}


/**
* @internal wmHardResetTrigger function
* @endinternal
*
* @brief   WM function to indicate the device to do 'HARD reset'.
*          NOTEs:
*          1. the WM will forget ALL the settings that previously configured on this device.
*            including :
*                1. the PCIe config space (all registers return to 'default')
*                2. all the registers in BAR0,BAR2 (all registers/memories return to 'default')
*                3. info set by wmDeviceOnPciBusSet
*                4. info set by wmDeviceIrqParamsSet
*                5. info set by wmPortConnectionInfo about all the ports.
*          2. no need to call again to 'global functions' (that are not 'per device') :
*                1. simOsFuncBindOwnSimOs / simOsFuncBind
*                2. simulationLibInit
*                3. wmBindCbFunctions
*          3. after the function returns to the caller , the caller can re-configure the WM device.
*             meaning that function not end till the WM device finished the Hard reset.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
*
*/
WM_STATUS wmHardResetTrigger(
    IN GT_U32   wmDeviceId
)
{
    void*      deviceObj;
    DEV_NUM_CHECK_RANGE_MAC(wmDeviceId,wmHardResetTrigger);

    SCIB_SEM_TAKE;
    deviceObj = scibDbPtr[wmDeviceId].deviceObj;
    if (scibDbPtr[wmDeviceId].deviceObj == NULL)
    {
        SCIB_SEM_SIGNAL;
        return WM_ERROR;
    }

    /* forget about previous settings */
    scibDbPtr[wmDeviceId].interrupt_line    = 0;
    scibDbPtr[wmDeviceId].interrupt_source  = 0;
    scibDbPtr[wmDeviceId].interrupt_flags   = 0;

    scibDbPtr[wmDeviceId].pciBus  = 0;
    scibDbPtr[wmDeviceId].pciDev  = 0;
    scibDbPtr[wmDeviceId].pciFunc = 0;

    scibDbPtr[wmDeviceId].bar0_base[ADDR_LOW_INDEX]  = 0;
    scibDbPtr[wmDeviceId].bar0_base[ADDR_HIGH_INDEX] = 0;
    scibDbPtr[wmDeviceId].bar0_size                  = 0;
    scibDbPtr[wmDeviceId].bar2_base[ADDR_LOW_INDEX]  = 0;
    scibDbPtr[wmDeviceId].bar2_base[ADDR_HIGH_INDEX] = 0;
    scibDbPtr[wmDeviceId].bar2_size                  = 0;

    SCIB_SEM_SIGNAL;

    /* call SKernel without locking the SCIB ! */
    skernelDeviceResetType(deviceObj,0/*HARD RESET*/);

    return WM_OK;
}

/**
* @internal wmSoftResetTrigger function
* @endinternal
*
* @brief   WM function to indicate the device to do 'SOFT reset' without loosing PCIe config.
*          This API needed by the Simics although no such 'API' in the HW !
*
*          NOTEs:
*          1. the WM will forget almost ALL the settings that previously configured on this device.
*            but will NOT forget about :
*                1. the PCIe config space (all registers return to 'default')
*                2. all the registers in BAR0,BAR2 (all registers/memories return to 'default')
*                3. info set by wmDeviceOnPciBusSet
*                4. info set by wmDeviceIrqParamsSet
*                5. info set by wmPortConnectionInfo about all the ports.
*          2. no need to call again to 'global functions' (that are not 'per device') :
*                1. simOsFuncBindOwnSimOs / simOsFuncBind
*                2. simulationLibInit
*                3. wmBindCbFunctions
*          3. after the function returns to the caller , the caller can re-configure the WM device.
*             meaning that function not end till the WM device finished the Soft reset.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
*
*/
WM_STATUS wmSoftResetTrigger(
    IN GT_U32   wmDeviceId
)
{
    void*      deviceObj;
    DEV_NUM_CHECK_RANGE_MAC(wmDeviceId,wmSoftResetTrigger);

    deviceObj = scibDbPtr[wmDeviceId].deviceObj;
    if (scibDbPtr[wmDeviceId].deviceObj == NULL)
    {
        return WM_ERROR;
    }

    /* call SKernel without locking the SCIB ! */
    skernelDeviceResetType(deviceObj,1/*SOFT RESET , without PCIe*/);

    return WM_OK;
}


GT_STATUS scibPerformanceTest(IN GT_U32 regAddr,IN GT_U32  numOfTimes)
{
    GT_U32  BAR0_low,BAR2_low;
    GT_U32  BAR0_high,BAR2_high;
    GT_U32  data=0;
    GT_U32  start,end;
    GT_U32  wmDevId=0,ii;

    start = SIM_OS_MAC(simOsTickGet)();

    if(sinit_global_usePexLogic)
    {
        /* 1. get the BAR0,BAR2 from the PCI config space */
        scibPciRegRead(wmDevId,0x10,1,&data);
        BAR0_low = data & (~0xf);
        scibPciRegRead(wmDevId,0x14,1,&data);
        BAR0_high = data;
        scibPciRegRead(wmDevId,0x18,1,&data);
        BAR2_low = data & (~0xf);
        scibPciRegRead(wmDevId,0x1c,1,&data);
        BAR2_high = data;

        /* 2. set window in BAR0 iATU */
        /* use the middle one (window 32) ... for 'avarage' performance */
        /* create mapping window : ATU_LOWER_BASE_ADDRESS_REG */
        data = BAR2_low;
        wmMemPciWrite(BAR0_high,BAR0_low + 0x1308, 1, &data );
        /*ATU_UPPER_BASE_ADDRESS_REG*/
        data = BAR2_high;
        wmMemPciWrite(BAR0_high,BAR0_low + 0x130c, 1, &data );
        /*ATU_LIMIT_ADDRESS_REG : set size to 1M (20 bits) offset from the 'base' */
        data = BAR2_low+0xFFFFF;
        wmMemPciWrite(BAR0_high,BAR0_low +0x1310, 1, &data );
        /*ATU_REGION_CTRL_1_REG : type of region to be mem */
        data = 0x0;
        wmMemPciWrite(BAR0_high,BAR0_low + 0x1300, 1, &data );
        /*ATU_REGION_CTRL_2_REG : enable the region */
        data = 0x80000000;
        wmMemPciWrite(BAR0_high,BAR0_low + 0x1304, 1, &data );
        /* Configure the window map : ATU_LOWER_TARGET_ADDRESS_REG*/
        data = regAddr & 0xfff00000;
        wmMemPciWrite(BAR0_high,BAR0_low + 0x1314, 1, &data );

        for(ii = 0 ; ii < numOfTimes ; ii++)
        {
            wmMemPciRead(BAR2_high,BAR2_low + (regAddr & 0x000fffff) , 1 , &data);
        }
    }
    else
    {
        for(ii = 0 ; ii < numOfTimes ; ii++)
        {
            scibReadMemory(wmDevId,regAddr,1,&data);
        }
    }

    end = SIM_OS_MAC(simOsTickGet)();

    printf("Performance of read register [0x%8.8x] for [%d] times in [%d] ms (regValue=[0x%8.8x]) \n",
        regAddr,numOfTimes,end-start,data);

    return GT_OK;
}

GT_STATUS scibPerformanceDmaTest(IN GT_U32 dma_addr_high,IN GT_U32  dma_addr_low,IN GT_U32  numOfTimes)
{
    GT_U32  data=0;
    GT_U32  start,end;
    GT_U32  ii;

    start = SIM_OS_MAC(simOsTickGet)();
    for(ii = 0 ; ii < numOfTimes ; ii++)
    {
        scibDmaRead64BitAddr(dma_addr_high,dma_addr_low,1,&data);
    }

    end = SIM_OS_MAC(simOsTickGet)();

    printf("Performance of DMA read memory  [0x%8.8x%8.8x] for [%d] times in [%d] ms (dmaValue=[0x%8.8x]) \n",
        dma_addr_high,dma_addr_low,numOfTimes,end-start,data);

    return GT_OK;
}

/* debug function to allow test with traffic before CPSS initialization */
extern GT_STATUS smemHarrierAllowTraffic
(
    IN GT_U32   devNum
);
/* debug function to allow test with traffic before CPSS initialization */
extern GT_STATUS smemAldrin3MAllowTraffic
(
    IN GT_U32   devNum
);


/**
* @internal wmHarrierAllowTraffic_forDebugOnly function
* @endinternal
*
* @brief   WM 'debug only' function to allow traffic to pass in the device , before CPSS initialization.
*          this function is for debug instead of the 'micro-init : supper image'
*          the function is for Harrier device.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
*
*/
WM_STATUS wmAllowTraffic_forDebugOnly_harrier(
    IN GT_U32   wmDeviceId
)
{
    WM_STATUS rc;
    DEV_NUM_CHECK_RANGE_MAC(wmDeviceId,wmAllowTraffic_forDebugOnly_harrier);

    SCIB_SEM_TAKE;
    if (scibDbPtr[wmDeviceId].deviceObj == NULL)
    {
        SCIB_SEM_SIGNAL;
        return WM_ERROR;
    }

    rc = smemHarrierAllowTraffic(wmDeviceId) ? WM_OK : WM_ERROR;

    SCIB_SEM_SIGNAL;

    return rc;
}

/**
* @internal wmAllowTraffic_forDebugOnly_aldrin3m function
* @endinternal
*
* @brief   WM 'debug only' function to allow traffic to pass in the device , before CPSS initialization.
*          this function is for debug instead of the 'micro-init : supper image'
*          the function is for Aldrin3-M device.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
*
*/
WM_STATUS wmAllowTraffic_forDebugOnly_aldrin3m(
    IN GT_U32   wmDeviceId
)
{
    WM_STATUS rc;
    DEV_NUM_CHECK_RANGE_MAC(wmDeviceId,wmAllowTraffic_forDebugOnly_aldrin3m);

    SCIB_SEM_TAKE;
    if (scibDbPtr[wmDeviceId].deviceObj == NULL)
    {
        SCIB_SEM_SIGNAL;
        return WM_ERROR;
    }

    rc = smemAldrin3MAllowTraffic(wmDeviceId) ? WM_OK : WM_ERROR;

    SCIB_SEM_SIGNAL;

    return rc;
}

/**
* @internal wmCheckCbStatus_forDebugOnly function
* @endinternal
*
* @brief   the function prints indication for the CB functions that we are currently
*          inside them.
*
*/
WM_STATUS wmCheckCbStatus_forDebugOnly(void)
{
    GT_U32  timeToSleep = 3000;

    START_IS_STUCK_INSIDE_GLOBAL_CB(dmaReadFunc           );
    START_IS_STUCK_INSIDE_GLOBAL_CB(dmaWriteFunc          );
    START_IS_STUCK_INSIDE_GLOBAL_CB(interruptTriggerFunc  );
    START_IS_STUCK_INSIDE_GLOBAL_CB(egressPacketFunc      );
    START_IS_STUCK_INSIDE_GLOBAL_CB(traceLogFunc          );

    SIM_OS_MAC(simOsSleep)(timeToSleep);

    CHECK_IS_STUCK_INSIDE_GLOBAL_CB(dmaReadFunc           );
    CHECK_IS_STUCK_INSIDE_GLOBAL_CB(dmaWriteFunc          );
    CHECK_IS_STUCK_INSIDE_GLOBAL_CB(interruptTriggerFunc  );
    CHECK_IS_STUCK_INSIDE_GLOBAL_CB(egressPacketFunc      );
    CHECK_IS_STUCK_INSIDE_GLOBAL_CB(traceLogFunc          );

    CLEAR_IS_STUCK_INSIDE_GLOBAL_CB(dmaReadFunc           );
    CLEAR_IS_STUCK_INSIDE_GLOBAL_CB(dmaWriteFunc          );
    CLEAR_IS_STUCK_INSIDE_GLOBAL_CB(interruptTriggerFunc  );
    CLEAR_IS_STUCK_INSIDE_GLOBAL_CB(egressPacketFunc      );
    CLEAR_IS_STUCK_INSIDE_GLOBAL_CB(traceLogFunc          );

    return WM_OK;
}
/**
* @internal scibCbFuncWatchDogTask function
* @endinternal
*
* @brief   'watch dog' task , to check if we are inside a CB function for
*       more than 3 seconds
*
*/
static void scibCbFuncWatchDogTask(void* dummy)
{
    printf("WM started the CB watchdog task \n");
    while(1)
    {
        wmCheckCbStatus_forDebugOnly();
    }
}

/**
* @internal scibInitCbFuncWatchDogTask function
* @endinternal
*
* @brief   init a 'watch dog' task , to check if we are inside a CB function for
*       more than 3 seconds
*
* @param[in] deviceObjPtr             - pointer to the device object for task.
*/
static void scibInitCbFuncWatchDogTask(void)
{
    GT_TASK_HANDLE          taskHandl;          /* task handle */

    taskHandl = SIM_OS_MAC(simOsTaskCreate)(GT_TASK_PRIORITY_LOWEST,
                      (unsigned (__TASKCONV *)(void*))scibCbFuncWatchDogTask,
                      (void *) NULL);
    if (taskHandl == NULL)
    {
        skernelFatalError(" scibInitCbFuncWatchDogTask: cannot create 'watch dog' task \n");
    }
}


GT_STATUS wmForDebugDmaRead64BitAddr(
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords
)
{
    GT_U32  *wordsArray;
    GT_U32  ii;

    if(numOfWords == 0)
    {
        return GT_BAD_PARAM;
    }

    wordsArray = malloc(sizeof(GT_U32)*numOfWords);
    if(wordsArray == NULL)
    {
        return GT_BAD_PTR;
    }

    fprintf( stderr,"Read from addr_high[0x%8.8x],addr_low[0x%8.8x],numOfWords[%d] \n",
        addr_high,addr_low,numOfWords);
    fprintf( stderr,"start the read \n");

    scibDmaRead64BitAddr(addr_high,addr_low,numOfWords,wordsArray);

    fprintf( stderr,"read ended \n");

    for(ii = 0 ; ii < numOfWords ; ii++)
    {
        if((ii % 8) == 0)
        {
            fprintf( stderr,"[%3.3d]: ",ii);
        }
        fprintf( stderr,"[0x%8.8x] ",wordsArray[ii]);
        if((ii % 8) == 7)
        {
            fprintf( stderr,"\n");
        }
    }
    fprintf( stderr,"\n");

    free(wordsArray);

    return GT_OK;
}

GT_STATUS wmForDebugDmaWrite64BitAddr(
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    IN GT_U32   pattern
)
{
    GT_U32  *wordsArray;
    GT_U32  ii;

    if(numOfWords == 0)
    {
        return GT_BAD_PARAM;
    }

    wordsArray = malloc(sizeof(GT_U32)*numOfWords);
    if(wordsArray == NULL)
    {
        return GT_BAD_PTR;
    }

    fprintf( stderr,"Write to addr_high[0x%8.8x],addr_low[0x%8.8x],numOfWords[%d] \n"
                    "pattern of : [0x%8.8x]\n",
        addr_high,addr_low,numOfWords);
    for(ii = 0 ; ii < numOfWords ; ii++)
    {
        wordsArray[ii] = pattern + ii;

        if((ii % 8) == 0)
        {
            fprintf( stderr,"[%3.3d]: ",ii);
        }
        fprintf( stderr,"[0x%8.8x] ",wordsArray[ii]);
        if((ii % 8) == 7)
        {
            fprintf( stderr,"\n");
        }
    }
    fprintf( stderr,"\n");

    fprintf( stderr,"start the write \n");

    scibDmaWrite64BitAddr(addr_high,addr_low,numOfWords,wordsArray);

    fprintf( stderr,"write ended \n");

    free(wordsArray);

    return GT_OK;
}

