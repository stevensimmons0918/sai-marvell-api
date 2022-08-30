
#if   defined(WIN32)

#pragma warning(push)
#pragma warning(disable: 4996) /*To avoid depricated function warinings*/

#endif



#include <stdint.h>
#include <string.h>
#include <gtOs/gtGenTypes.h>
#include "ctype.h"
#include <cm3NetPort.h>
#include <cm3ConfigFileParser.h>
#include <cm3FileOps.h>
#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* strtoul */
#include <asicSimulation/SCIB/scib.h>

#if defined(LINUX)
#include <strings.h>
#define stricmp strcasecmp
#endif


#define cm3IsSpace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#define local_isprint(c)     ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))

#define copyValue( b, v, t ) (( b != NULL ) ? (*( t * )( b ) = ( v )) : 0)

#define DFX_REG_BASE                          0xa0100000

#define DELIMETERS   " "
#define copyValue( b, v, t ) (( b != NULL ) ? (*( t * )( b ) = ( v )) : 0)

#ifdef CM3_SIM_SUPPORT_DFX_COMMAND
#define GETMEM( x, o )                (*((uintptr_t) (x + o) ))
#endif

#define END_OF_PARAMS   { NULL, NULL, GT_FALSE, 0 }
typedef char *pchar;

typedef enum {
    UINT_PARAM,
    BOOL_PARAM,
    STRING_PARAM
} CM3_SIM_CONFIG_PARAM_TYPE;

/* the current deviceId that hold the CM3 that doing actions */
GT_U32 current_SIM_CM3_DEVICE_ID = 0;
GT_STATUS simCm3CurrentDeviceIdSet(IN GT_U32 deviceId/*the deviceIde as in the INI file*/)
{
    current_SIM_CM3_DEVICE_ID = deviceId;
    return GT_OK;
}

typedef struct {
    const char *name;
    const char *value;
    const GT_BOOL isMandatory;
    const CM3_SIM_CONFIG_PARAM_TYPE type;
} CM3_SIM_CONFIG_PARAM;

static CM3_SIM_CONFIG_PARAM apParams[] = {
    { "fec_sup",        NULL, GT_TRUE,  STRING_PARAM },
    { "fec_req",        NULL, GT_TRUE,  STRING_PARAM },
    { "mode1",          NULL, GT_TRUE,  STRING_PARAM },
    { "speed1",         NULL, GT_TRUE,  STRING_PARAM },
    { "lane",           NULL, GT_TRUE,  UINT_PARAM },
    END_OF_PARAMS
};

static CM3_SIM_CONFIG_PARAM speedParams[] = {
    { "__speed", NULL, GT_TRUE, UINT_PARAM },
    { "mode", NULL, GT_TRUE, STRING_PARAM },
    { "fec", NULL, GT_FALSE, STRING_PARAM },
    { "inBand", NULL, GT_FALSE, STRING_PARAM },
    { "anSpeed", NULL, GT_FALSE, STRING_PARAM },
    { "anDplx", NULL, GT_FALSE, STRING_PARAM },
    { "anFcEn", NULL, GT_FALSE, STRING_PARAM },
    { "anBypsEn", NULL, GT_FALSE, STRING_PARAM },

    END_OF_PARAMS
};

MI_LOOPBACK_MODES loopback_modes[] = {
    { "no_lb",    0,       DISABLE_LB},
    { "sd_tx2rx", HWS_PMA, TX_2_RX_LB},
    { "sd_rx2tx", HWS_PMA, RX_2_TX_LB},
    { "mac_tx2rx",HWS_MAC, TX_2_RX_LB}
};

/* addresses that even in the supper image , will be skipped (should not be in the supper image) */
/* this array to fix JIRA : CPSS-13768 : ASIM, WM simulation, port manager link flapping 1G_SGMII */
static GT_U32   harrier_forbiddenAddr[] = {
    0x0D600088 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Status         */
    0x0D600094 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Interrupt Cause*/
    0x0D6000A0 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Status         */
    0x0D6000AC ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Interrupt Cause*/
    0x0D6000E8 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Status         */
    0x0D6000F4 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Interrupt Cause*/
    0x0F6000A0 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Status         */
    0x0F6000AC ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Interrupt Cause*/
    0x0F6000B8 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Status         */
    0x0F6000C4 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Interrupt Cause*/
    0x0F600130 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Status         */
    0x0F60013C ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Interrupt Cause*/
    0x01200088 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Status         */
    0x01200094 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Interrupt Cause*/
    0x012000B8 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Status         */
    0x012000C4 ,  /*<400_MAC> <MTIP EXT> MTIP EXT/MTIP EXT Units/Port<%n> Interrupt Cause*/
    0xFFFFFFFF};

static GT_U32   harrier_isValidAddr(/*IN*/ GT_U32  addr)
{
    GT_U32  ii;
    for(ii = 0 ; harrier_forbiddenAddr[ii] != 0xFFFFFFFF ; ii++)
    {
        if(harrier_forbiddenAddr[ii] == addr)
        {
            return 0;/* not valid*/
        }
    }

    return 1;
}

GT_STATUS cm3GenSwitchRegisterSet
(
    GT_U32 address,
    GT_U32 data,
    GT_U32 mask
)
{
    GT_U32  oldData,newData;

    if(!harrier_isValidAddr(address))
    {
        return GT_OK;
    }

    scibReadMemory(SIM_CM3_DEVICE_ID, address, 1, &oldData);
    if(mask!=0)
    {
        data    &= mask;
        oldData &= (~mask );
        newData = oldData |data;
    }
    else
    {
        newData = oldData;
    }
    scibWriteMemory(SIM_CM3_DEVICE_ID,address,1,&newData);

    return GT_OK;
}


GT_STATUS cm3GenSwitchRegisterGet(GT_U32 address, GT_U32 *data, GT_U32 mask)
{
    GT_U32  currentData;
    if(mask == 0)
        mask = 0xFFFFFFFF;

    scibReadMemory(SIM_CM3_DEVICE_ID, address, 1, &currentData);

    *data   = currentData& mask;

    return GT_OK;
}
static CM3_SIM_CONFIG_PARAM *findParam( CM3_SIM_CONFIG_PARAM *params, const char *name )
{
    CM3_SIM_CONFIG_PARAM *param = params;

    while ( param->name ) {
        if ( stricmp( name, param->name ) == 0 )
            return param;
        param++;
    }
    return NULL;
}

static CM3_SIM_CONFIG_PARAM *resetValues( CM3_SIM_CONFIG_PARAM *params )
{
    CM3_SIM_CONFIG_PARAM *param = params;

    while ( param->name ) {
        param->value = NULL;
        param++;
    }
    return params;
}

static GT_BOOL checkMandatoryParams( CM3_SIM_CONFIG_PARAM *params )
{
    CM3_SIM_CONFIG_PARAM *param = params;

    while ( param->name ) {
        if ( param->isMandatory == GT_TRUE && param->value == NULL )
            return GT_FALSE;
        param++;
    }
    return GT_TRUE;
}

static GT_STATUS getValue( CM3_SIM_CONFIG_PARAM *param, void *buf )
{
    char *next;

    if ( param == NULL )
        return GT_BAD_PTR;
    if (param->value == NULL)
        return GT_BAD_VALUE;
    if ( param->type == UINT_PARAM ) {
        if ( !isdigit( param->value[0] ) )
            return GT_BAD_VALUE;
        copyValue( buf, strtoul( param->value, &next, 0 ), GT_U32 );
    } else if ( param->type == BOOL_PARAM ) {
        GT_BOOL value;
        if ( stricmp( param->value, "true" ) == 0 ||
             stricmp( param->value, "enable" ) == 0 ) {
            value = GT_TRUE;
        } else if ( stricmp( param->value, "false" ) == 0 ||
                    stricmp( param->value, "disable" ) == 0 ) {
            value = GT_FALSE;
        } else {
            return GT_BAD_VALUE;
        }
        copyValue( buf, value, GT_BOOL );
    } else if ( param->type == STRING_PARAM ) {
        copyValue( buf, ( char * )param->value, pchar );
    } else {
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

GT_STATUS smemHarrierRxTxEnableSet
(
    IN GT_U32   devNum,
    IN GT_U32   portNum, /*macNum*/
    IN GT_U32   enable,  /*0 or 1*/
    IN GT_U32   isSegmentedPort /*0 or 1*/,
    IN GT_U32   numSerdeses,
    IN GT_U32   fecMode,
    IN GT_U32   sgmii_speed
);

static void getModeInfo(
    IN GT_U32  portNum,
    IN CM3_SIM_HWS_PORT_STANDARD mode,
    OUT GT_U32* isSegmentedPortPtr,
    OUT GT_U32* numLanesPtr,
    OUT CM3_SIM_MV_HWS_PORT_FEC_MODE * fecModePtr,
    OUT GT_U32* sgmii_speedPtr
)
{
    GT_U32  isSegmentedPort,numLanes;
    CM3_SIM_HWS_PORT_STANDARD fecMode;

    *sgmii_speedPtr = mode == SGMII ? 1000 : 0;

    switch(mode)
    {
        case  /* 77 */ _200GBase_KR4   :
        case  /* 78 */ _200GBase_KR8   :
        case  /* 79 */ _400GBase_KR8   :
        case  /* 87 */ _200GBase_CR4   :
        case  /* 88 */ _200GBase_CR8   :
        case  /* 89 */ _400GBase_CR8   :
        case  /* 90 */ _200GBase_SR_LR4:
        case  /* 91 */ _200GBase_SR_LR8:
        case  /* 92 */ _400GBase_SR_LR8:
            isSegmentedPort = 1;
            break;
        default:
            isSegmentedPort = 0;
            break;
    }

    * isSegmentedPortPtr = isSegmentedPort;

    switch(mode)
    {
        case /* 51 */ _24GBase_KR2          :
        case /* 53 */ _25GBase_KR2          :
        case /* 56 */ _50GBase_KR2          :
        case /* 59 */ _50GBase_SR2          :
        case /* 64 */ _50GBase_KR2_C        :
        case /* 65 */ _40GBase_KR2          :
        case /* 69 */ _50GBase_CR2          :
        case /* 74 */ _50GBase_CR2_C        :
        case /* 76 */ _100GBase_KR2         :
        case /* 81 */ _52_5GBase_KR2        :
        case /* 86 */ _100GBase_CR2         :
        case /* 94 */ _100GBase_SR_LR2      :
        case /* 109*/ _53GBase_KR2          :
        case /* 113*/ _106GBase_KR2         :
            numLanes = 2;
            break;

        case /* 54 */ _50GBase_KR4          :
        case /* 57 */ _100GBase_KR4         :
        case /* 60 */ _100GBase_SR4         :
        case /* 61 */ _100GBase_MLG         :
        case /* 62 */ _107GBase_KR4         :
        case /* 66 */ _29_09GBase_SR4       :
        case /* 67 */ _40GBase_CR4          :
        case /* 70 */ _100GBase_CR4         :
        case /* 77 */ _200GBase_KR4         :
        case /* 80 */ _102GBase_KR4         :
        case /* 82 */ _40GBase_KR4          :
        case /* 84 */ _40GBase_SR_LR4       :
        case /* 87 */ _200GBase_CR4         :
        case /* 90 */ _200GBase_SR_LR4      :
        case /* 96 */ _106GBase_KR4         :
        case /* 108*/ _42GBase_KR4          :
        case /* 112*/ _212GBase_KR4         :
            numLanes = 4;
            break;

        case /* 78 */ _200GBase_KR8         :
        case /* 79 */ _400GBase_KR8         :
        case /* 88 */ _200GBase_CR8         :
        case /* 89 */ _400GBase_CR8         :
        case /* 91 */ _200GBase_SR_LR8      :
        case /* 92 */ _400GBase_SR_LR8      :
        case /* 110*/ _424GBase_KR8         :
            numLanes = 8;
            break;

        default:
            numLanes = 1;
            break;
    }
    * numLanesPtr = numLanes;


#if 0
    RS-FEC modes taken from hwsHarrierPort0SupModes :
#endif
    switch(mode)
    {
        case _25GBase_KR       :
        case _25GBase_CR       :
        case _25GBase_KR_C     :
        case _25GBase_CR_C     :
        case _25GBase_KR_S     :
        case _25GBase_CR_S     :
        case _25GBase_SR       :
        case _40GBase_KR2      :
        case _50GBase_KR2      :
        case _50GBase_CR2      :
        case _50GBase_KR2_C    :
        case _50GBase_CR2_C    :
        case _50GBase_SR2      :
        case _100GBase_KR4     :
        case _100GBase_CR4     :
        case _100GBase_SR4     :
        case _107GBase_KR4     :
            fecMode = RS_FEC;
            break;

        case _50GBase_KR        :
        case _50GBase_CR        :
        case _50GBase_SR_LR     :
        case _100GBase_KR2      :
        case _100GBase_CR2      :
        case _100GBase_SR_LR2   :
        case _106GBase_KR2      :
        case _200GBase_KR4      :
        case _200GBase_CR4      :
        case _200GBase_SR_LR4   :
        case _212GBase_KR4      :
        case _400GBase_KR8      :
        case _400GBase_CR8      :
        case _400GBase_SR_LR8   :
        case _424GBase_KR8      :
            fecMode = RS_FEC_544_514;
            break;

        default :
            fecMode = FEC_OFF;
            break;
    }

    *fecModePtr = fecMode;

    return;
}


static GT_STATUS cm3SimSetInterfaceSpeedAndMode( MV_NET_PORT *port, CM3_SIM_CONFIG_PARAM *params , char*    modeStr)
{
    CM3_SIM_CONFIG_PARAM *param;
    CM3_SIM_HWS_PORT_STANDARD mode = NON_SUP_MODE;
    GT_STATUS ret = GT_OK;
    char *modeName = NULL;
    GT_BOOL apMode = GT_FALSE;

    if(0 == strcmp(modeStr,"mode1"))
    {
        apMode = GT_TRUE;
    }

    param = findParam( params,  modeStr );
    if ( param != NULL ) {


        ret = getValue( param, &modeName );
        if ( ret != GT_OK )
            goto bad_value;

        if ( (cm3SimInterfaceModeFromString(modeName, &mode) != GT_OK) ) {
            goto bad_value;
        }
    }

    if(modeName&&mode!=NON_SUP_MODE)
    {
        port->active = GT_TRUE;
        port->portMode = mode;

        SIM_CM3_LOG( "Parser info : Found activating request for port %d in mode %s\n",port->ppPortNum, modeName);

        if(GT_TRUE == apMode)
        {
            port->isAp = GT_TRUE;

            param = findParam( apParams, "fec_sup" );
            if(0 == strcmp(param->value,"DISABLE"))
            {
                port->fec_sup = GT_FALSE;
            }
            else
            {
                port->fec_sup = GT_TRUE;
            }

            param = findParam( apParams, "fec_req" );
            if(0 == strcmp(param->value,"DISABLE"))
            {
                port->fec_req = GT_FALSE;
            }
            else
            {
                port->fec_req = GT_TRUE;
            }

            param = findParam( apParams, "lane" );
            ret = getValue( param, &port->lane );
            if ( ret != GT_OK )
                goto bad_value;

            SIM_CM3_LOG( "Parser info : Found activating request for port %d in mode %s , AN :fec_sup[%d],fec_req[%d],lane[%d] \n",
                port->ppPortNum, modeName , port->fec_sup , port->fec_req , port->lane);
            fprintf( stderr,"Micro Init : portNum[%d] set as [%s] , AN :fec_sup[%d],fec_req[%d],lane[%d] (not created yet) \n",
                port->ppPortNum, modeName , port->fec_sup , port->fec_req , port->lane);
        }
        else
        {
            port->isAp = GT_FALSE;

            fprintf( stderr,"Micro Init : portNum[%d] set as [%s] (not created yet) \n",
                port->ppPortNum,modeName);
        }

        return GT_OK;
    }

bad_value:
    return GT_FAIL;
}


/**
 *
 * prepareApPortConfiguration
 *
 * @param port
 * @param params
 *
 * @return MV_STATUS
 */
static GT_STATUS cm3SimPrepareApPortConfiguration (MV_NET_PORT *port, CM3_SIM_CONFIG_PARAM *params)
{
    return cm3SimSetInterfaceSpeedAndMode(port,params,"mode1");
}

/*
 * Parses the ethernet ap configuration command in format:
 *
 */
static GT_STATUS cm3SimParseInterfaceApModeSet(MV_NET_PORT* port)
{
    enum { PARAM_NAME, PARAM_VALUE } state = 0;
    CM3_SIM_CONFIG_PARAM *param = NULL;
    char* token = NULL;
    resetValues(apParams);
    while ( ( token = strtok( NULL, DELIMETERS ) ) != NULL ) {
        switch(state) {
        case PARAM_NAME:
            param = findParam( apParams, token );
            if ( param == NULL ) {
                SIM_CM3_LOG( "Parser error :Invalid ap configuration parameter name '%s'.\n", token );
                return GT_FAIL;
                }
            state = PARAM_VALUE;
            break;
        case PARAM_VALUE:
            if ( param == NULL ) {
                SIM_CM3_LOG( "Parser error : Parameter value found before it's name.\n" );
                return GT_FAIL;
            }
            param->value = token;
            state = PARAM_NAME;
            break;
        }
    }

    if ( (checkMandatoryParams( apParams ) == GT_TRUE)
        /*&& (checkEthernetApOptionalParams() == MV_OK)*//*Speed is not used currently*/ ) {
        return cm3SimPrepareApPortConfiguration(port, apParams);

    } else {
        SIM_CM3_LOG( "Bad params. Mandatory check failed \n" );
        return GT_FAIL;
    }
}

/*
 * Parses the interface speed command in format:
 *      interface ethernet {device}/{interface #} speed SPEED mode MODE
 */
static GT_STATUS cm3SimParseInterfaceSpeed( MV_NET_PORT *port )
{
    enum { SPEED, PARAM_NAME, PARAM_VALUE } state = 0;
    CM3_SIM_CONFIG_PARAM *currentParam = NULL;
    char *token;

    resetValues( speedParams );
    while ( ( token = strtok( NULL, DELIMETERS ) ) != NULL ) {
        switch ( state ) {
        case SPEED:
            currentParam = findParam( speedParams, "__speed" );
            if ( currentParam != NULL )
                currentParam->value = token;
            state = PARAM_NAME;
            break;

        case PARAM_NAME:
            currentParam = findParam( speedParams, token );
            if ( currentParam == NULL ) {
                SIM_CM3_LOG(  "Parser error :Unknown speed parameter '%s'.\n", token );
                return GT_FAIL;
            }

            state = PARAM_VALUE;
            break;

        case PARAM_VALUE:
            if ( currentParam == NULL ) {
                SIM_CM3_LOG(  "Parser error :Parameter value found before it's name.\n" );
                return GT_FAIL;
            }
            currentParam->value = token;

            state = PARAM_NAME;
            break;
        }
    }

    if ( checkMandatoryParams( speedParams ) == GT_TRUE )
        return cm3SimSetInterfaceSpeedAndMode( port, speedParams ,"mode");

    SIM_CM3_LOG(  "Parser error :%s",
                 ( state == SPEED )       ? "Need to specify the speed value.\n" :
                 ( state == PARAM_NAME )  ? "Need to specify the speed mode.\n" :
                 ( state == PARAM_VALUE ) ? "Need to specify the speed mode value.\n" :
                                            "Unknown error.\n" );
    return GT_FAIL;
}


static GT_STATUS cm3ParseData( GT_U32 *data, GT_U32 *mask )
{
    char *value;

    /* Get the data to write. */
    value = strtok( NULL, DELIMETERS );
    if ( value == NULL )
        return GT_NO_MORE;
    if ( !isdigit( value[0] ) )
        goto parseData_bad_value;
    copyValue( data, strtoul( value, &value, 0 ), GT_U32 );

    /* Get the mask for data. */
    value = strtok( NULL, DELIMETERS );
    if ( value != NULL ) {
        if ( !isdigit( value[0] ) )
            goto parseData_bad_value;
        copyValue( mask, strtoul( value, &value, 0 ), GT_U32 );
    } else {
        copyValue( mask, 0xffffffff, GT_U32 );
    }

    return GT_OK;

parseData_bad_value:
    SIM_CM3_LOG(  "Parser error :parseData failed - Invalid Value\n");
    return GT_BAD_VALUE;
}

static GT_STATUS cm3ParseData1( GT_U32 *data, GT_U32 *mask, GT_BOOL *dfx )
{
    char *value;
    *dfx = GT_FALSE;

    /* Get the data to write. */
    value = strtok( NULL, DELIMETERS );
    if ( value == NULL )
        return GT_NO_MORE;
    if ( !isdigit( value[0] ) )
        goto parseData1_bad_value;

    copyValue( data, strtoul( value, &value, 0 ), GT_U32 );

    /* Get the mask for data. */
    copyValue( mask, 0xffffffff, GT_U32 );
    value = strtok( NULL, DELIMETERS );

    if (value != NULL)
    {
        if ( isdigit( value[0]) )
        {
            copyValue( mask, strtoul( value, &value, 0 ), GT_U32 );
        }
        else
        {
            if ((strcmp(value,"DFX") != 0) && (strcmp(value,"dfx") != 0))
                goto parseData1_bad_value;
            else{
                *dfx = GT_TRUE;
                return GT_OK;
            }
        }
        /* Get optional DFX. */
        value = strtok( NULL, DELIMETERS );
        if (value != NULL)
        {
            if ((strcmp(value,"DFX") != 0) && (strcmp(value,"dfx") != 0))
                goto parseData1_bad_value;
            else
                *dfx = GT_TRUE;
        }
    }

    return GT_OK;

parseData1_bad_value:
    printf("Parser error :parseData1 failed - Invalid Value\n");
    return GT_BAD_VALUE;
}

static GT_STATUS cm3ParseWriteCommand( void )
{
    GT_U32 mask = 0xFFFFFFFF, address = 0, data = 0;
    GT_STATUS ret;
    char *value;

    /* Get the destination addresess. */
    value = strtok( NULL, DELIMETERS );
    if ( value == NULL ) {
        SIM_CM3_LOG(  "Parser error :Need to specify the address of a register.\n");
        return GT_FAIL;
    }
    if ( !isdigit( value[0] ) ) {
        SIM_CM3_LOG(  "Parser error : Invalid address value '%s'.\n", value);
        return GT_FAIL;
    }
    address = strtoul( value, &value, 0 );


    ret = cm3ParseData( &data, &mask );
    if ( ret == GT_OK ) {
        if ( cm3GenSwitchRegisterSet( /*curDevNum,*/ address, data, mask ) != GT_OK ) {
            SIM_CM3_LOG( "Can't write data 0x%X with mask 0x%X to register 0x%X", data, mask, address );
            return GT_FAIL;
        }

        return GT_OK;
    }

    if ( ret == GT_NO_MORE )
        SIM_CM3_LOG("Parser error :Need to specify the data.\n");
    return ret;
}


static GT_STATUS cm3ParseWriteCommand1( void )
{
    GT_U32 mask = 0xFFFFFFFF, address = 0, data = 0;
    GT_STATUS ret;
    GT_BOOL   dfx;
    char *value;
#ifdef CM3_SIM_SUPPORT_DFX_COMMAND
    GT_U32 tmp;
#endif

    /*
       the format is :
       devNum  portGroup  0x000f8014 0xfffffff2 0xffffffff <optional:DFX>
    */

    /* devNum and portGroup are not in usage now. So the first 2 parameters are skiped */
    value = strtok( NULL, DELIMETERS );
    if ( value == NULL ) {
         ret = GT_FAIL;
         goto parser_error;
    }
    value = strtok( NULL, DELIMETERS );
    if ( value == NULL ) {
        ret = GT_FAIL;
        goto parser_error;
    }
    /* Get the destination addresess. */
    value = strtok( NULL, DELIMETERS );
    if ( value == NULL ) {
        ret = GT_FAIL;
        goto parser_error;
    }
    if ( !isdigit( value[0] ) ) {
        ret = GT_FAIL;
        goto parser_error;
    }
    address = strtoul( value, &value, 0 );
    ret = cm3ParseData1( &data, &mask, &dfx );
    if ( ret == GT_OK )
    {
        /* check if parseData encountered DFX */
        if (dfx == GT_TRUE)
        {
#ifdef CM3_SIM_SUPPORT_DFX_COMMAND
            /* change address to dfx full address */
            address = address | DFX_REG_BASE;
            if (mask == 0xFFFFFFFF)
            {
                GETMEM( address, 0 ) = data;
            }
            else
            {
                tmp = (GT_U32)(GETMEM( address, 0 ) & ~mask);
                tmp |= (data & mask);
                GETMEM( address, 0 ) = tmp;
            }

            return GT_OK;
#else
            SIM_CM3_LOG( "DFX direct mapping is not supported\n");
            return GT_FAIL;

#endif
        }
        if ( cm3GenSwitchRegisterSet( /*curDevNum,*/ address, data, mask ) != GT_OK ) {
            SIM_CM3_LOG( "write failed\n");
            return GT_FAIL;
        }
        return GT_OK;
    }

parser_error:
    SIM_CM3_LOG(" wrong params, usage: devNum  portGroup  addr data mask <optional:DFX>\n");
    return ret;
}

/**
 *
 * miGetloopbackModeParam
 *
 * @param value
 *
 * @return MV_U8
 */
static GT_U8 cm3MiGetloopbackModeParam( const char * value )
{
    GT_U32 i;
    for ( i=0; i < sizeof(loopback_modes)/sizeof(MI_LOOPBACK_MODES); i++ ) {
        if ( !stricmp(loopback_modes[i].paramName, value) ) {
            return i;
        }
    }
    return 0xFF;
}


/* Simulation of parseIfLoopbackSet*/
static GT_STATUS cm3ParseIfLoopbackSet
(
    MV_NET_PORT *port
)
{
    char * token;
    GT_U8 newLbMode = 0;

    while ( ( token = strtok( NULL, DELIMETERS ) ) != NULL ) {
        if ( stricmp( token, "mode" ) == 0 ) {
             if ( ( token = strtok( NULL, DELIMETERS ) ) != NULL ) {
                  if ( stricmp( token, "no_loopback" ) == 0 ) {

                      if((port->reqDiagMode&LB_MASK)== LB_MODE_MAC_TXRX)
                      {
                            port->reqDiagMode &= ~LB_MASK; /* set LB_MODE_NOLB */
                            SIM_CM3_LOG(  "UnConfigure loopback to port %d\n", port->ppPortNum);
                            return cm3SimNetPortLbSet(port->ppPortNum,GT_FALSE);
                      }
                      break;
                  } else if ( LB_MODE_NOLB != (LB_MASK & (port->diagMode)) ) {
                        return GT_ERROR;
                  } else {
                      newLbMode = cm3MiGetloopbackModeParam(token);
                      if ( 0xFF != newLbMode ) {
                          port->reqDiagMode &= ~LB_MASK;
                          port->reqDiagMode |= newLbMode;

                          if((port->reqDiagMode&LB_MASK)== LB_MODE_MAC_TXRX)
                          {
                            SIM_CM3_LOG(  "Configure loopback to port %d\n", port->ppPortNum);
                            return cm3SimNetPortLbSet(port->ppPortNum,GT_TRUE);
                          }
                          break;
                      } else {
                          return GT_ERROR;
                      }
                  }
             } else {
                 return GT_ERROR;
             }
        }
    }

    return GT_OK;
}

/* Simulation of 'Create' the port after we have speed and mode for it */
static GT_STATUS cm3SimParsePortCreate
(
    MV_NET_PORT *port
)
{
    GT_STATUS   rc;
    GT_U32  isSegmentedPort,numLanes,sgmii_speed;
    CM3_SIM_MV_HWS_PORT_FEC_MODE fecMode;
    if(port->ppPortNum == NON_SUP_MODE)
    {
        SIM_CM3_LOG("'create' not supported on port[%d] without speed,mode \n",port->ppPortNum);
        return GT_OK;
    }

    getModeInfo(port->ppPortNum,port->portMode,&isSegmentedPort,&numLanes,&fecMode,&sgmii_speed);

    fprintf( stderr,"Micro Init : portNum[%d] is now created \n",
        port->ppPortNum);

    smemHarrierRxTxEnableSet(SIM_CM3_DEVICE_ID,port->ppPortNum,GT_TRUE,isSegmentedPort,numLanes,fecMode,sgmii_speed);
    rc = cm3SimNetPortSetMode(port, port->portMode/*, FEC_NA*/);

    return rc;
}
extern GT_U32   supperImageSize;
/*
 * Parses the interface command in format:
 *      interface FAMILY DEVICE MODULE parameters...
 */
static GT_STATUS cm3ParseInterfaceCommand( void )
{
    GT_STATUS rc;
    enum { FAMILY, DEVICE, MODULE } state = FAMILY;
    char *token = NULL, *slash = NULL;
    MV_NET_PORT *port = NULL;
    GT_U8 devNum = 0;

    while ( ( token = strtok( NULL, DELIMETERS ) ) != NULL ) {
        switch ( state ) {
        case FAMILY:
            if ( stricmp( token, "ethernet" ) == 0 ) {
                state = DEVICE;
            } else {
                SIM_CM3_LOG(  "Parser error :Unknown family '%s' specified.\n", token);
                return GT_FAIL;
            }
            break;

        case DEVICE:

            if ( ( slash = strchr( token, '/' ) ) != NULL ) {
                devNum =strtoul( token, NULL, 0 );

                port = mvNetPortArrayGetPortByDevPPort(devNum, strtoul( slash + 1, NULL, 0 ) );

                if ( port == NULL ) {
                    SIM_CM3_LOG("Parser error : Bad device '%s'.\n", token );
                    return GT_FAIL;
                }

                state = MODULE;
            } else {
                SIM_CM3_LOG("Parser error : Device must be specified in a {device}/{interface #} form.\n" );
                return GT_FAIL;
            }
            break;

        case MODULE:

            if ( stricmp( token, "negotiation" ) == 0 ) {

                /*return parseInterfaceNegotiation( port );*/
                SIM_CM3_LOG("Token %s parsing is not supported\n",token);
                return GT_OK;
            } else if ( stricmp( token, "delete" ) == 0 ) {
                smemHarrierRxTxEnableSet(SIM_CM3_DEVICE_ID,port->ppPortNum,GT_FALSE,0,0,0,0);
                return cm3SimNetPortSetMode(port, NON_SUP_MODE/*, FEC_NA*/);
            } else if ( stricmp( token, "create" ) == 0 ) {
                return cm3SimParsePortCreate( port );
            } else if ( stricmp( token, "speed" ) == 0 ) {
                rc = cm3SimParseInterfaceSpeed( port );
                if(rc == GT_OK && (supperImageSize < (50*1024)))
                {
                    /* allow to integrate the code that will support the
                       'fake supper image' for the WM

                       because those files not hold the 'create' of a port (only speed,mode)
                    */
                    rc = cm3SimParsePortCreate( port );
                }
                return rc;
            } else if ( stricmp( token, "serdes" ) == 0 ) {
                /*return parseInterfaceSerdes( port );*/
               SIM_CM3_LOG("Token %s parsing is not supported\n",token);
                return GT_OK;
            } else if ( stricmp( token, "flow" ) == 0 ) {
                /*return parseInterfaceFlow( port );*/
                SIM_CM3_LOG("Token %s parsing is not supported\n",token);
                return GT_OK;
            } else if ( stricmp( token, "ap") == 0 ) {
                return cm3SimParseInterfaceApModeSet( port );
            } else if ( stricmp( token, "loopback") == 0 ) {
               return cm3ParseIfLoopbackSet( port );
            } else {
                SIM_CM3_LOG("Token %s parsing is not supported\n",token);
                return GT_FAIL;
            }
            break;
        }
    }

    SIM_CM3_LOG( "Parser error %s.\n",
                 ( state == FAMILY ) ? "Need to specify the interface family" :
                 ( state == DEVICE ) ? "Need to specify the device" :
                 ( state == MODULE ) ? "Need to specify the module" :
                                       "Unknown error" );
    return GT_FAIL;
}

GT_STATUS cm3ParseConfigCommand( void )
{
    GT_U32                      counter = 0;
    file_params_STC             file_params;
    file_params_STC            *file_params_ptr;
    char *type  = NULL;
    char *configNameOrId  = NULL;

    GT_BOOL fileFound = 0, restart = GT_TRUE;

    type = strtok( NULL, DELIMETERS  );

    /* Get the config type - "list" or "run" */
    if ( type == NULL ) {
        SIM_CM3_LOG("Parser error:Please specify the config type.\n");
        return GT_FALSE;
    }

    /*  if config type = "list" */
    else if( stricmp( type, "list" ) == 0 ) {
        SIM_CM3_LOG("Available configuration(s):_________________________\n");

        file_params_ptr = &file_params;
        while (cm3GetNextFile(file_params_ptr, restart) != GT_FALSE) {
            restart = GT_FALSE;
            switch (file_params_ptr->type) {
            case script:
            case EPROM:
                counter++;
                SIM_CM3_LOG("-- file: %12s,   type=%d,   useBmp=%d,   size=%6d,  address=0x%08x\n",
                        file_params_ptr->name, file_params_ptr->type, file_params_ptr->useBmp,
                        file_params_ptr->size, file_params_ptr->address);
                break;
            default:
                break;
            }
        }
        SIM_CM3_LOG("Total:__________________________________%d  file(s).\n", counter);
        return GT_FALSE;
    }


    /*  if config type = "run" */
    if ( stricmp( type, "run" ) != 0 &&
         stricmp( type, "rerun" ) != 0) {
        return GT_FALSE;
    }

    configNameOrId = strtok( NULL, DELIMETERS );
    if ( !configNameOrId ) {
        return GT_FALSE;
    }

    /* iterate thru all configuration scripts*/
    file_params_ptr = &file_params;
    while (cm3GetNextFile(file_params_ptr, restart) != GT_FALSE) {
        restart = GT_FALSE;
        switch (file_params_ptr->type) {
        case script:
        case EPROM:
            if ( ((stricmp( configNameOrId, file_params_ptr->name ) == 0) && (stricmp( type, "run" ) == 0)) ||
                 ((((1<<(configNameOrId[0] - '0')) & file_params_ptr->useBmp) != 0) && (stricmp( type, "rerun" ) == 0)) )
            {
                SIM_CM3_LOG("\n\nconfig run: %s, type=%d size=%d addr=0x%08x\n"
                           "======================================================\n",
                        file_params_ptr->name, file_params_ptr->type,
                        file_params_ptr->size, file_params_ptr->address);
                cm3lnConfigFileParserParse((const char *)file_params_ptr->address, file_params_ptr->size);

                 if ( stricmp( type, "run" ) == 0 )
                 {
                     return GT_FALSE;
                 }
                 else /* rerun */
                 {
                      fileFound = 1;
                 }
            }
            break;
        default:
            break;
        }
    }

    if ( stricmp( type, "rerun" ) == 0 && 0 == fileFound)
    {
        fprintf( stderr,"Micro Init : WARNING : can't find file/profile [%s] in the supper image \n",
            configNameOrId);

        return GT_NOT_FOUND;
    }

    return GT_OK;
}



GT_STATUS cm3ParseCommand( char *command )
{
    char *commandName = NULL;

    if(0 == strncmp("interface ",command,strlen("interface ")))
    {
        /* must print the line before doing strtok operations */
        fprintf(stderr,"process: %s \n",command);
    }

    commandName = strtok( command, DELIMETERS );
    /*only micro init required commands*/
    if ( commandName )
    {
       if ( stricmp( commandName, "w") == 0 ) {
           return cm3ParseWriteCommand();
       }
       else if ( stricmp( commandName, "wrd" ) == 0 ) {
           return cm3ParseWriteCommand1();
       }
       else if ( stricmp( commandName, "interface" ) == 0 ) {
           return cm3ParseInterfaceCommand();
       }
       else if ( stricmp( commandName, "config" ) == 0 ) {
           return cm3ParseConfigCommand();
        }
    }

    SIM_CM3_LOG( "error: Unknown command name %s.\n", commandName);
    return GT_BAD_PARAM;
}


static int cm3ParseOptions(char *command)
{

    return GT_OK;
}


/* Used to parse one of the following files:
*     script
*     EPROM
*     SDK options file
*
*/
GT_STATUS cm3lnConfigFileParserParse( const char *file, size_t size )
{
    static char command[256];
    const char *commandStart = NULL;
    GT_STATUS ret = GT_OK;

    if (( file == NULL ) || (size <= 8))
        return GT_BAD_PARAM;
    commandStart = file;

    do {
        const char *commandEnd = NULL;
        size_t length = 0;

        /* skip leading spaces and empty lines */
        while ( cm3IsSpace( *commandStart ) )
            commandStart++;

        /* finish if there is nothing more to parse */
        if ((size_t)(commandStart - file) >= size)
            break;

        commandEnd = strchr( commandStart, '\n' );

        /* When reaching end of file, strchr may fail to find the next '\n'
         * or will find it in the next file. We must make sure length remains in
         * our file's boundary */
        if (commandEnd == NULL)
            length = size + (size_t)(file - commandStart);
        else {
            length = (size_t)(commandEnd - commandStart);
            if (length > size + (size_t)(file - commandStart))
                length = size + (size_t)(file - commandStart);
        }

        /* cut the command if it's to long */
        if ( length > 255 )
            length = 255;

        memcpy( command, commandStart, length );
        command[length] = '\0';
        commandStart += length;

        while( length > 1 && (!local_isprint(command[length-1]))){
            /* fix for crash in VC10 */
            /* fix ignore of command in linux */
            command[--length] = '\0';
        }

        /* remove trailing spaces */
        while( length > 1 && isspace(command[length-1])) {
            command[--length] = '\0';
        }

        /* print out comments without slashes and prevent running as a commands */
        if ((length >= 2) && (((command[0] == '/') && (command[1] == '/')) || (command[0] == '#'))) {
            SIM_CM3_LOG("%s\n", &command[2]);
            continue;
        }
        /* print out comments without slashes or dash and prevent running as a commands */
        if ((length >= 2) && (((command[0] == '/') && (command[1] == '*')) )) {
            continue;
        }
        /* print out comments without slashes or dash and prevent running as a commands */
        if ((length >= 2) && (((command[0] == '-') && (command[1] == '-')) )) {
            continue;
        }

        /* Parser is determined by current file type */
        if (getCurrentFileType() == OPTIONS)
            ret = cm3ParseOptions(command);
        else
            ret = cm3ParseCommand(command);

        if (ret !=GT_OK) {
            SIM_CM3_LOG("Command: <%s> finished with error %d\n",command, ret);
            /*VIRTUAL_REG_FW_ERR_CODE_SET_BIT(0x400);    BIT 10 */
            break;
        }

    } while (1);

    return GT_OK;
}
#if   defined(WIN32)
#pragma warning(pop)
#endif

