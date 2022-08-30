#include <os/simTypes.h>
#include <stdlib.h>
#include <cm3NetPort.h>
#include <cm3ConfigFileParser.h>
#include <cm3FileCommon.h>
#include <stdio.h>
#include <string.h>

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>

#define MV_HWS_HARRIER_GOP_PORT_NUM_CNS    16


#define CM3_SIM_BIT0            0x00000001
#define CM3_SIM_BIT1            0x00000002


static MV_NET_PORT *portArray = NULL;
static size_t portArraySize = 0;

extern SKERNEL_DEVICE_OBJECT* smemTestDeviceIdToDevPtrConvert
(
    IN  GT_U32                      deviceId
);
/*   devNum  active   ppPortGroup  ppPortNum portMode  reqPrtMode txLanePolarity  rxLanePolarity   diagMode */
#define SIM_CM3_MICRO_INIT_PORT_DEFAULT( devNum, ppPortNum ) \
    { GT_FALSE, devNum, 0, ppPortNum, NON_SUP_MODE, NON_SUP_MODE, 0, 0, LB_MODE_NOLB}

MV_NET_PORT cm3SimSip6NetPortConfig_Switch[] =
{
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 0 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 1 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 2 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 3 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 4 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 5 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 6 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 7 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 8 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 9 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 10 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 11 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 12 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 13 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 14 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 16 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 18 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 20 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 22 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 24 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 26 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 28 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 30 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 32 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 33 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 34 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 35 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 36 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 37 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 38 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 39 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 40 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 41 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 42 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 43 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 44 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 45 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 46 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 47 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 64 ),/*added manually,it is not in the original CM3 array*/
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 80 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 81 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 82 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 83 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 84 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 85 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 86 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 87 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 88 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 89 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 90 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 91 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 92 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 93 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 94 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 95 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 112 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 113 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 114 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 115 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 116 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 117 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 118 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 119 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 120 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 121 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 122 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 123 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 124 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 125 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 126 ),
    SIM_CM3_MICRO_INIT_PORT_DEFAULT( 0, 127 )
};

#define SPEED_10M  ( 1 << 0 )
#define SPEED_100M ( 1 << 1 )
#define SPEED_1G   ( 1 << 2 )
#define SPEED_10G  ( 1 << 3 )
#define SPEED_2_5G ( 1 << 4 )
#define SPEED_5G   ( 1 << 5 )
#define SPEED_40G  ( 1 << 6 )
#define SPEED_20G  ( 1 << 7 )
#define SPEED_25G  ( 1 << 8 )
#define SPEED_50G  ( 1 << 9 )
#define SPEED_100G ( 1 << 10 )
#define SPEED_200G ( 1 << 11 )/*not exist in real MI*/


#define isalpha(a) ((((unsigned)(a)|32)-'a') < 26)
#define isdigit(a) (((unsigned)(a)-'0') < 10)
#define islower(a) (((unsigned)(a)-'a') < 26)
#define isupper(a) (((unsigned)(a)-'A') < 26)
#define isprint(a) (((unsigned)(a)-0x20) < 0x5f)
#define isgraph(a) (((unsigned)(a)-0x21) < 0x5e)
#define isspace(a) ((a) == ' ' || (unsigned)(a)-'\t' < 5)
#define tolower(c) (isupper(c) ? ((c) | 32) : (c))
#define toupper(c) (islower(c) ? ((c) & 0x5f) : (c))


typedef struct {
    char *name;
    CM3_SIM_HWS_PORT_STANDARD mode;
    GT_U16 speed;

} CM3_SIM_INTERFACE_MODE_STRING, *PCM3_SIM_INTERFACE_MODE_STRING;

size_t mvNetPortArrayGetSize( void )
{
    return portArraySize;
}


static CM3_SIM_INTERFACE_MODE_STRING if_modes[] = {
        {"SGMII",           SGMII,              SPEED_1G},
        {"10GBase-KR",      _10GBase_KR,        SPEED_10G},
        {"25GBase-KR",      _25GBase_KR,        SPEED_25G},
        {"25GBase-SR-LR",   _25GBase_SR,        SPEED_25G},
        {"50GBase_KR",      _50GBase_KR,        SPEED_50G},
        {"50GBase-KR",      _50GBase_KR,        SPEED_50G},
        {"50GBase_SR_LR",   _50GBase_SR_LR,     SPEED_50G},
        {"50GBase-SR-LR",   _50GBase_SR_LR,     SPEED_50G},
        {"40GBase-KR4",     _40GBase_KR4,       SPEED_40G},
        {"50GBase-SR-LR",   _50GBase_SR_LR,     SPEED_50G},
        {"50GBase-KR2",     _50GBase_KR2,       SPEED_50G},
        {"100GBase_KR2",   _100GBase_KR2,       SPEED_100G},
        {"100GBase-KR2",   _100GBase_KR2,       SPEED_100G},
        {"100GBase-KR4",   _100GBase_KR4,       SPEED_100G},
        {"200GBase-KR4",   _200GBase_KR4,       SPEED_200G},
        {NULL, NON_SUP_MODE, 0}
};


static GT_STATUS simCm3FalconPortEgfForceLinkUp(MV_NET_PORT *port, GT_BOOL enable);
static GT_STATUS simCm3FalconMtiExtForceLinkUp(MV_NET_PORT *port, GT_BOOL enable);

static int cm3SimStricmp( const char *lstr, const char *rstr )
{
    unsigned char lch = 0, rch = 0;

    for ( ; ( (lch = tolower(*lstr)) == (rch = tolower(*rstr)) ) && lch;
          lstr++, rstr++ );

    return lch - rch;
}

static GT_STATUS mvNetPortArrayInit( MV_NET_PORT *ports, size_t count )
{
    GT_U32  mallocSize;
    void* oldPtr;
    void* newPtr;

    if ( ports == NULL ) return GT_BAD_PTR;
    if ( count == 0 ) return GT_BAD_SIZE;

    oldPtr = portArray;
    mallocSize = sizeof(MV_NET_PORT)*count;

    /* fix IPBU JIRA : IPBUSW-12363 : ASIM-CN10KAS: Profile set before kernel reboot remains the same after reboot - switch Ports not functional after reboot */
    /* the DB need to be restored to 'empty' , after the CM3 did reset (during soft reset) */
    newPtr = malloc(mallocSize);
    memcpy(newPtr,ports,mallocSize);
    portArraySize   = count;
    portArray       = newPtr;

    if(oldPtr)
    {
        free(oldPtr);
    }

    return GT_OK;
}

GT_STATUS cm3SimInterfaceModeFromString(char *modeName,
                            CM3_SIM_HWS_PORT_STANDARD *mode)
{
    int i;
    if ( NULL != modeName && NULL != mode ) {
        for ( i=0; NULL != if_modes[i].name; i++ ) {
            if ( !cm3SimStricmp( modeName, if_modes[i].name) ) {
                *mode = if_modes[i].mode;
                break;
            }
        }
        if ( NULL == if_modes[i].name ) {
                SIM_CM3_LOG(  "Parser error :Unknown value '%s'. Variants are:", modeName );
                for ( i=0; NULL != if_modes[i].name; i++ ) {
                    SIM_CM3_LOG("  %s ",if_modes[i].name);
                    if (i%5 == 4) SIM_CM3_LOG("\r\n");
                }
                printf("\n");
                return GT_FAIL;
        }
    } else {
        return GT_BAD_PARAM;
    }

    return GT_OK;
}


/**
 * mvNetPortArrayGetPort
 *
 * @param ind
 *
 * @return MV_NET_PORT*
 */
MV_NET_PORT *mvNetPortArrayGetPort( size_t ind )
{
    if ( ind < portArraySize ){
        return &portArray[ind];
    }
    return NULL;
}


/**
 *
 * mvNetPortArrayGetPortByDevPPort
 *
 * @param devNum
 * @param portNum
 *
 * @return MV_NET_PORT*
 */
MV_NET_PORT *mvNetPortArrayGetPortByDevPPort( GT_U8 devNum, GT_U8 portNum )
{
    size_t ind = 0;
    MV_NET_PORT *port = NULL;
    while ( ( port = mvNetPortArrayGetPort( ind++ ) ) ) {
        /* devNum is ignored as the 'DB' always hold devNum = 0 and never changed*/
        if(/*devNum == port->devNum &&*/ portNum == port->ppPortNum){
            return port;
        }
    }
    return NULL;
}

GT_STATUS smemHarrierPortInit
(
    IN GT_U32   devNum,
    IN GT_U32   portNum
);

/* replace hwsHarrierIfInit that called from real MI */
static GT_STATUS simCm3_hwsHarrierIfInit(void)
{
    MV_NET_PORT *port;
    GT_U32  ii;

    for(ii = 0 ; ii < portArraySize ; ii++)
    {

        port = mvNetPortArrayGetPortByDevPPort(SIM_CM3_DEVICE_ID,ii);
        if(!port)
        {
            /* skip the port */
            continue;
        }

        smemHarrierPortInit(SIM_CM3_DEVICE_ID,port->ppPortNum);
    }

    return GT_OK;
}

GT_STATUS mvNetPortInit()
{
    GT_STATUS rc ;

    rc = mvNetPortArrayInit( cm3SimSip6NetPortConfig_Switch, sizeof(cm3SimSip6NetPortConfig_Switch)/sizeof(cm3SimSip6NetPortConfig_Switch[0]) );
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = simCm3_hwsHarrierIfInit();

    return rc;
}


/**
 *simCm3FalconMacTxRxChannelEnable (copy form  MacTxRxChannelEnable)
 *
 * @param devNum
 * @param isRaven
 * @param portNum
 *
 * @return GT_STATUS
 */

GT_STATUS simCm3FalconMacTxRxChannelEnable(GT_U32 devNum, GT_U8 isRaven, GT_U8 portNum)
{

    GT_U8 reduced_port;
    GT_U8 raven;
    GT_U8 tile = 0;
    GT_U8 d2d_unit;
    GT_U8 second_half;
    GT_U8 pipe = 0;
    GT_U32 addr;
    GT_U32 data;

    if(microInitCm3HostPpFamilyGet()!=CM3_SIM_PP_FAMILY_DXCH_FALCON_E)
    {
        return GT_OK;
    }

    reduced_port = portNum & 0x7;
    raven = portNum / 16;
    second_half = (portNum >> 3) & 0x1 ;

    d2d_unit = ((raven * 2) + second_half) % 4 ;

    if (raven % 4 > 2)
        pipe = 1;

    if (raven > 3)
        tile = 1 ;


    if (isRaven)
    {
        addr = 0x684004 + (reduced_port * 4) + (second_half * 0x10000);
        if (tile == 0 )
            addr = addr + ((raven%4) * 0x1000000);
        else
            addr = addr + (0x3000000 - ((raven%4) * 0x1000000)) + 0x20000000;
    }


    else /* Eagle */
    {
        addr = 0xD7F4004 + (reduced_port * 4);
        if (tile ==0)
            addr = addr + (d2d_unit * 0x10000) + (pipe * 0x8000000);
        else
            addr = addr + (0x30000 - (d2d_unit * 0x10000)) + (!pipe * 0x8000000) + 0x20000000;
    }

    cm3GenSwitchRegisterGet(addr, &data, 0); /*TX enable*/
    data = data | 0x80000000;
    cm3GenSwitchRegisterSet(addr,data,0);


    addr = addr + 0x800; /*RX enable*/
    cm3GenSwitchRegisterGet(addr, &data, 0);
    data = data | 0x80000000;
    cm3GenSwitchRegisterSet(addr,data,0);

    return GT_OK;

}


/**
 *
 * simCm3FalconMacPortEnable (copy form  mv_MAC_portEnable)
*
*
 * @param port
 * @param enable
 *
 * @return GT_STATUS
 */
GT_STATUS simCm3FalconMacPortEnable(MV_NET_PORT *port, GT_BOOL enable)
{

    GT_U32 addr, data;
    GT_U32 secondHalf;
    GT_U32 portNum;
    GT_U32 raven;
    GT_U32 tile = 0;

    if(microInitCm3HostPpFamilyGet() == CM3_SIM_PP_FAMILY_DXCH_FALCON_E)
    {
         raven = (port->ppPortNum) / 16;
         portNum = (port->ppPortNum) % 16;
         if (raven > 3) {
             tile = 1;
         }

         if (portNum > 7)
         {
             secondHalf = 1;
             portNum = portNum % 8;
         }
         else{
             secondHalf = 0;
         }

         addr = 0x444008 + (secondHalf * 0x80000) + (portNum * 0x1000);

         if (tile == 0 )
             addr = addr + ((raven%4) * 0x1000000);
         else
             addr = addr + (0x3000000 - ((raven%4) * 0x1000000)) + 0x20000000 ;

         cm3GenSwitchRegisterGet(addr, &data, 0);
         if (enable)
         {
             data = data | CM3_SIM_BIT0 | (CM3_SIM_BIT1);
         }
         else {
              data = data & (~(CM3_SIM_BIT0|CM3_SIM_BIT1));
         }
         cm3GenSwitchRegisterSet(addr,data,0);
    }
    else /*hawk*/
    {
        SIM_CM3_LOG("TBD simCm3FalconMacPortEnable\n");
    }





    return GT_OK;
}


GT_STATUS simCm3PortCtrlCreatePort ( MV_NET_PORT * port )
{
    GT_STATUS rc;

    /*D2D*/

    rc = simCm3FalconMacTxRxChannelEnable(port->devNum,0/*Eagle side*/,port->ppPortNum);
    if(rc!=GT_OK)
    {
        SIM_CM3_LOG(  "simCm3FalconMacTxRxChannelEnable failed for port  %d\n",port->ppPortNum);
        return rc;
    }

    rc = simCm3FalconMacTxRxChannelEnable(port->devNum,1/*Raven  side*/,port->ppPortNum);
    if(rc!=GT_OK)
    {
        SIM_CM3_LOG(  "simCm3FalconMacTxRxChannelEnable failed for port  %d\n",port->ppPortNum);
        return rc;
    }

    /*MAC command config*/
    rc = simCm3FalconMacPortEnable(port,GT_TRUE);
    if(rc!=GT_OK)
    {
        SIM_CM3_LOG(  " simCm3FalconMacPortEnable failed for port  %d\n",port->ppPortNum);
        return rc;
    }

     /*EGF force link up.Copy from mvNetPortEnableCfg*/
     rc = simCm3FalconPortEgfForceLinkUp(port,GT_TRUE);
     if(rc!=GT_OK)
     {
        SIM_CM3_LOG(  "simCm3FalconPortEgfForceLinkUp failed for port  %d\n",port->ppPortNum);
        return rc;
     }

     /*MAC link up*/
     rc = simCm3FalconMtiExtForceLinkUp(port,GT_TRUE);
     if(rc!=GT_OK)
     {
        SIM_CM3_LOG("simCm3FalconMtiExtForceLinkUpimCm3PortEgfForceLinkUp failed for port  %d\n",port->ppPortNum);
        return rc;
     }


    return rc;
}

/**
 * simCm3PollingInit (Copy from mvPollingInit)
 *
 * @return GT_STATUS
 */
GT_STATUS simCm3PollingInit( GT_VOID )
{
    /*do not create polling  task like real micro init ,just initialize ports*/
    GT_U32 i;
    GT_STATUS rc;

    /* loop all ports from unq table */
    for( i = 0 ; i<portArraySize ; i++ )
    {
        rc = simCm3PortCtrlCreatePort(portArray+i);
        if(rc!=GT_OK)
        {
            SIM_CM3_LOG("simCm3PortCtrlCreatePort failed for port index %d\n",i);
            return rc;
        }
    }


    return GT_OK;
}


GT_STATUS simCm3LinkUp( GT_VOID )
{
    /*do not create polling  task like real micro init ,just initialize ports*/
    GT_U32 i;
    GT_STATUS rc;

    /* loop all ports from unq table */
    for( i = 0 ; i<portArraySize ; i++ )
    {
         if(portArray[i].active == GT_TRUE)
         {
          /*EGF force link up.Copy from mvNetPortEnableCfg*/
          rc = simCm3FalconPortEgfForceLinkUp(portArray+i,GT_TRUE);
          if(rc!=GT_OK)
          {
             SIM_CM3_LOG(  "simCm3FalconPortEgfForceLinkUp failed for port  %d\n",portArray[i].ppPortNum);
             return rc;
          }
         }
    }


    return GT_OK;
}


/**
 * simCm3FalconPortEgfForceLinkUp (copied from mvNetPortEnableCfg)
 *
 * @param portNum
 * @param enable
 *
 * @return GT_STATUS
 */
GT_STATUS simCm3FalconPortEgfForceLinkUp(MV_NET_PORT *port, GT_BOOL enable)
{
    /*Egress Filter Configurations/Physical Port Link Status Mask <%n>*/
    GT_U32 regAddr[] = {0xcb06380 , 0x14B06380 , 0x2cb06380 , 0x34B06380 };
    GT_U32  wordIndex;  /*index in registers array*/
    GT_U32  bitIndex;   /*index in the register*/
    GT_U32 data = 0;
    GT_U32 addr = 0,portNum;
    GT_U32 i,regNum = 4 ;
    SKERNEL_DEVICE_OBJECT * devObjPtr;

    devObjPtr = smemTestDeviceIdToDevPtrConvert(SIM_CM3_DEVICE_ID);

    if((microInitCm3HostPpFamilyGet() == CM3_SIM_PP_FAMILY_DXCH_AC5P_E)||
        (microInitCm3HostPpFamilyGet() == CM3_SIM_PP_FAMILY_DXCH_HARRIER_E))
    {
        regAddr[0]=smemUnitBaseAddrByNameGet(devObjPtr,"UNIT_EGF_EFT",0);
        regAddr[0]+=0x6380;
        regNum=1;
    }

    if(port->active == GT_FALSE)
    {
        SIM_CM3_LOG( "Port  %d is not active .Link event ignored\n",port->ppPortNum);
        return GT_OK;
    }

    portNum =port->ppPortNum;

    if (portNum > 58) /*gap*/
        portNum += 5;

    wordIndex = portNum / 32 ;
    bitIndex = portNum % 32 ;

    for (i=0; i<regNum; i++) {

        addr = regAddr[i] + (0x4 * wordIndex);

        cm3GenSwitchRegisterGet(addr, &data, 0);

        if (enable)
        {
            data |= (1 << bitIndex);
        } else {
            data &= ~(1 << bitIndex);
        }

        cm3GenSwitchRegisterSet(addr,data,0xFFFFFFFF);
    }
    SIM_CM3_LOG( "EGF LINK %s  for port  %d\n",enable?"UP":"DOWN",port->ppPortNum);

    return GT_OK;
}

GT_STATUS simCm3BootChannelGetPortStatus(MV_NET_PORT *port, GT_BOOL *enablePtr)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr;

    devObjPtr = smemTestDeviceIdToDevPtrConvert(SIM_CM3_DEVICE_ID);
    if(snetChtPortMacFieldGet(devObjPtr, port->ppPortNum,
        SNET_CHT_PORT_MAC_FIELDS_LinkState_E))
    {
        *enablePtr = GT_TRUE;
    }
    else
    {
        *enablePtr = GT_FALSE;
    }

    return GT_OK;
}

/**
 * simCm3FalconMtiExtForceLinkUp
 *
 * @param portNum
 * @param enable
 *
 * @return GT_STATUS
 */
GT_STATUS simCm3FalconMtiExtForceLinkUp(MV_NET_PORT *port, GT_BOOL enable)
{
/*MTIP IP WRAPPER/<MTIP EXT> MTIP EXT/MTIP EXT Units %j/Port<%n> Status*/
    GT_U32 addr, data;
    GT_U32 secondHalf;
    GT_U32 portNum;
    GT_U32 raven;
    GT_U32 tile = 0;

    if(microInitCm3HostPpFamilyGet() == CM3_SIM_PP_FAMILY_DXCH_FALCON_E)
    {
        raven = (port->ppPortNum) / 16;
        portNum = (port->ppPortNum) % 16;
        if (raven > 3) {
            tile = 1;
        }

        if (portNum > 7)
        {
            secondHalf = 1;
            portNum = portNum % 8;
        }
        else{
            secondHalf = 0;
        }

        addr = 0x478088 + (secondHalf * 0x80000) + (portNum * 0x1000);

        if (tile == 0 )
            addr = addr + ((raven%4) * 0x1000000);
        else
            addr = addr + (0x3000000 - ((raven%4) * 0x1000000)) + 0x20000000 ;

        cm3GenSwitchRegisterGet(addr, &data, 0);
        if (enable)
        {
            data = data | CM3_SIM_BIT0 | (CM3_SIM_BIT1);
        }
        else {
             data = data & (~(CM3_SIM_BIT0|CM3_SIM_BIT1));
        }
        cm3GenSwitchRegisterSet(addr,data,0);
      }
      else /*hawk*/
      {
          SIM_CM3_LOG("TBD simCm3FalconMacPortEnable\n");
      }


    return GT_OK;
}

GT_STATUS cm3SimNetPortSetMode
(
    MV_NET_PORT *port,
    CM3_SIM_HWS_PORT_STANDARD mode
    /*MV_HWS_PORT_FEC_MODE fecMode*/
)
{
    GT_STATUS rc;
    GT_BOOL   enable = GT_TRUE;

    if(NON_SUP_MODE==mode)
    {
        enable = GT_FALSE;
    }

    rc = simCm3FalconPortEgfForceLinkUp(port,enable);
    if(rc!=GT_OK)
    {
       SIM_CM3_LOG(  "simCm3FalconPortEgfForceLinkUp failed for port  %d\n",port->ppPortNum);
       return rc;
    }

    return GT_OK;
}


GT_STATUS cm3SimNetPortLbSet
(
    GT_U32  ppPortNum,
    GT_BOOL loopback
)
{
    GT_U32 unitAddr[]={0x0d900000,0x0f900000,0x01500000};/*PRV_CPSS_DXCH_UNIT_PCS_400G_0/1/2_E */
    GT_U32 address;
    GT_U32 regOffset = 0x2000;
    GT_U32 offsetFormula = 0x1000;
    GT_U32 ciderIndexInUnit= (ppPortNum%MV_HWS_HARRIER_GOP_PORT_NUM_CNS)>>1;
    GT_U32 ciderUnit = ppPortNum/MV_HWS_HARRIER_GOP_PORT_NUM_CNS;

    address = (unitAddr[ciderUnit] & 0xFFFFF000) +  regOffset + offsetFormula * ciderIndexInUnit;

    return cm3GenSwitchRegisterSet(address,(loopback?1:0)<<14,1<<14);
}


