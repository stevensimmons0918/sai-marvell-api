/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtbDbDxFalconSpeedControl.c
*
* @brief this file includes logic of Falcon 12.8 DB fan managment
* task which runs in the background- configurate the board fans
* speed according to the device temperature using PID controller for
* each of the bord's sensors.
*
* @version
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/boardConfig/gtDbDxFalcon.h>
#include <appDemo/userExit/userEventHandler.h>
#include <appDemo/boardConfig/gtbDbDxFalconSpeedControl.h>
#include <appDemo/sysHwConfig/gtAppDemoI2cConfig.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortDiag.h>
#include <extUtils/common/cpssEnablerUtils.h>

#include <gtOs/gtOsTimer.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMapping.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#ifdef INCLUDE_UTF
#include <common/tgfCommon.h>
#include <port/prvTgfPortFWS.h>
#endif

#ifndef ASIC_SIMULATION

/****************************/
/*   PID defines            */
/****************************/

#ifndef ABS
#define ABS(a)       (((a) < 0) ? -(a) : (a))
#endif

#define WINDUP_ON(_pid)         (_pid->features & PID_ENABLE_WINDUP)
#define DEBUG_ON(_pid)          (_pid->features & PID_DEBUG)
#define SAT_MIN_ON(_pid)        (_pid->features & PID_OUTPUT_SAT_MIN)
#define SAT_MAX_ON(_pid)        (_pid->features & PID_OUTPUT_SAT_MAX)
#define HIST_ON(_pid)           (_pid->features & PID_INPUT_HIST)

/***************************/
/*   SMI Drivers Creation  */
/***************************/
#define FAN_SPEED_MNG_TASK_PRIO_CNS     200

#if (__ARM_ARCH == 8) && defined(LINUX)
extern void soc_init_a7k(void);
#define SOC_INIT_A7K() soc_init_a7k()
#else
#define SOC_INIT_A7K()
#endif

/* SMI driver info */
static CPSS_HW_DRIVER_STC *Cpu_Smi_drv;
/* SMI adress of FPGA Phy containing the fan control register */
static GT_U8 fanFPGASmiAddr = 0x11;
/* fan control register adress in the FPGA */
static GT_U8 fanRegAddrInFPGA = 0x2;
/* maximum fan speed */
static GT_U32 default_fan_speed = 0x7;
/* fan speed control register mask */
static GT_U32 mask = 0x7;

/********************************************/
/* Task global configurations and variables */
/********************************************/

/* Task Id */
GT_TASK cpss_db_falcon_thermal_task_id;

/* cpss_db_falcon_thermal_en                             */
/* NOTE:  true - enable tempratue fan controlling task   */
/*        false - disable tempratue fan controlling task */
static GT_BOOL  cpss_db_falcon_thermal_en = GT_TRUE;

/* flag that task is running */
static GT_BOOL   fanManagementTaskCreated = GT_FALSE;

/* flag for killing task proccess due error in the task */
static GT_BOOL   fanManagementTaskKill = GT_FALSE;

/* cpss_thermal_debug                           */
/* NOTE: true - to enable debugging option      */
/*       false - to disable debugging option    */
static GT_BOOL cpss_thermal_debug = GT_FALSE;

#define RAVEN_MAX_NUMBER 16
#define PORT_NOT_FOUND -1

/* array that holds one port of each raven */
static GT_32   ravenPortsArr[RAVEN_MAX_NUMBER] = {PORT_NOT_FOUND, PORT_NOT_FOUND, PORT_NOT_FOUND, PORT_NOT_FOUND,
                                                  PORT_NOT_FOUND, PORT_NOT_FOUND, PORT_NOT_FOUND, PORT_NOT_FOUND,
                                                  PORT_NOT_FOUND, PORT_NOT_FOUND, PORT_NOT_FOUND, PORT_NOT_FOUND,
                                                  PORT_NOT_FOUND, PORT_NOT_FOUND, PORT_NOT_FOUND, PORT_NOT_FOUND};

/********************************************/
/* CPU sensors Params                 */
/********************************************/

static CPSS_OS_FILE_TYPE_STC    *thermalZone0Fp;
static CPSS_OS_FILE_TYPE_STC    *thermalZone1Fp;
/* assume CPU temp range is [0:120,000] - 6 char long + 1 /EOF*/
#define BUFFER_SIZE 7

/********************************************/
/* Transiver sensors Params                 */
/********************************************/

/*Temperature*/
#define APP_DEMO_AUTO_DETECT_TEMPERATURE_OFFSET             14
#define APP_DEMO_AUTO_DETECT_TEMPERATURE_LEN                2
#define APP_DEMO_AUTO_DETECT_TEMPERATURE_PAGE               0
#define APP_DEMO_AUTO_DETECT_TEMPERATURE_SUPPORT_OFFSET     159
#define APP_DEMO_AUTO_DETECT_TEMPERATURE_SUPPORT_LEN        1
#define APP_DEMO_AUTO_DETECT_TEMPERATURE_SUPPORT_PAGE       1

/*MUX address*/
#define APP_DEMO_AUTO_DETECT_MODULE_EEPROM_BASE_ADDRESS     0x50
#define APP_DEMO_AUTO_DETECT_MODULE_EEPROM_BASE_ADDRESS_1   0x51
#define APP_DEMO_AUTO_DETECT_MODULE_MUX9548_BASE_ADDRESS    0x70

/*MUX address*/
#define APP_DEMO_AUTO_DETECT_MODULE_FALCON_PRESENT_STATUS_BASE_ADDRESS      0x27

/* number of the cages in falcon 12.8 board */
#define APP_DEMO_FALCON_12_8_CAGE_ARR_SIZE      32

/********************************/
/*   PID structs and enums      */
/********************************/

/* time interval between temperature reading */
#define TIME_INTERVAL 10

/* fan management task PID parameters*/
#define PID_P_PARAM 5
#define PID_I_PARAM 5
#define PID_D_PARAM 0

/* array that holds the sensors names
   must be the same order as in APP_DEMO_PID_SENSORS_ENT enum*/
static const GT_CHAR* sensorsNamesArr[] = { "APP_DEMO_EAGLES_RAVENS_THERMAL_SENSOR_E", "APP_DEMO_CPU_THERMAL_SENSOR_E",
                                            "APP_DEMO_BOARD_THERMAL_SENSOR_E","APP_DEMO_TRANSIVER_THERMAL_SENSOR_E"};
/* maximum and minimum for PID output.
   used for conversion from PID output to fan speed */
static GT_FLOAT64  maxOutput = 0,minOutput = 0;

/**
* @enum PID_FEATURES_E
 *
 * @brief This enum use for the PID configurations
*/
typedef enum
{
    PID_ENABLE_WINDUP   =   (1<<0),
    PID_DEBUG           =   (1<<1),
    PID_OUTPUT_SAT_MIN  =   (1<<2),
    PID_OUTPUT_SAT_MAX  =   (1<<3),
    PID_INPUT_HIST      =   (1<<4)
}PID_FEATURES_E;

/**
* @enum FAN_SPEED_STATE_ENT
 *
 * @brief Falcon_DB 12.8 fan speeds.
*/
typedef enum {
    FAN_SPEED_DEFAULT_E = 0 ,
    FAN_SPEED_10_E ,
    FAN_SPEED_20_E ,
    FAN_SPEED_30_E ,
    FAN_SPEED_40_E ,
    FAN_SPEED_50_E ,
    FAN_SPEED_60_E ,
    FAN_SPEED_70_E
} FAN_SPEED_STATE_ENT;

/**
* @enum PID_STC
 *
 * @brief Falcon_DB 12.8 PID controller sturct.
*/
typedef struct
{
    /* PID parameters */
    GT_FLOAT64 kp;
    GT_FLOAT64 ki;
    GT_FLOAT64 kd;

    /* PID values */
    GT_FLOAT64 referenceTemp;
    GT_FLOAT64 integral;
    GT_FLOAT64 error_previous;

    /* PID configurations */
    GT_U32      features;
    GT_FLOAT64  intmax;
    GT_FLOAT64  sat_max;
    GT_FLOAT64  sat_min;

    /* Board Sensor */
    APP_DEMO_PID_SENSORS_ENT sensor;
}PID_STC;

/* array that holds the task's PIDs */
static PID_STC pidList[4];

/****************************/
/*   PID functions           */
/****************************/
/**
* @internal falcon_DB_PIDSetSP function
* @endinternal
*
* @brief set the the SetPoint tempertaure.
*
*/
static GT_VOID falcon_DB_PIDSetSP
(
    IN PID_STC      *pid,
    IN GT_FLOAT64   referenceTemp
)
{
    pid->referenceTemp = referenceTemp;
    pid->error_previous = 0;
    pid->integral = 0;
}

static GT_VOID falcon_DB_pidEnableFeature
(
    IN PID_STC          *pid,
    IN PID_FEATURES_E    feature,
    IN GT_FLOAT64       featureValue
)
{
    pid->features |= feature;

    switch (feature) {
        case PID_ENABLE_WINDUP:
            /* integral windup is in absolute output units, so scale to input units */
            pid->intmax = ABS(featureValue / pid->ki ); /*ABS(featureValue );*/
            break;
        case PID_DEBUG:
            break;
        case PID_OUTPUT_SAT_MIN:
            pid->sat_min = featureValue;
            break;
        case PID_OUTPUT_SAT_MAX:
            pid->sat_max = featureValue;
            break;
        case PID_INPUT_HIST:
            break;
    }
}

/**
* @internal falcon_DB_setMaxFanSpeedAndExit function
* @endinternal
*
* @brief   This routine clean info for the soon to be killed task 'falcon_DB_ThermalTask'.
*
*/
static void falcon_DB_setMaxFanSpeedAndExit
(
    IN GT_VOID *killTaskEnVoidPtr
)
{
    GT_STATUS   rc;
    GT_BOOL *killTaskEnPtr = (GT_BOOL*)killTaskEnVoidPtr;

    if (fanManagementTaskCreated)
    {
        cpssOsPrintf("Fan Management task killed - set maximum fan speed\n");

        /* set maximum fan speed in case task is killed */
        rc = Cpu_Smi_drv->writeMask(Cpu_Smi_drv, fanFPGASmiAddr, fanRegAddrInFPGA, &default_fan_speed , 1 /*count*/, mask);
        if(rc != GT_OK)
        {
            cpssOsPrintf("extDrvDirectSmiWriteReg failed \n");
        }

        /* mark that fan management task killed to avoid killing again */
        fanManagementTaskCreated = 0;

        /* turn on I2C error printing */
        appDemoHostI2cPrintErrorEnableSet(GT_TRUE);

        /* close all CPU thermal_zone file pointers and free memory */
        if (thermalZone0Fp != NULL)
        {
            cpssOsFclose(thermalZone0Fp);
            cpssOsFree(thermalZone0Fp);
        }
        if (thermalZone1Fp != NULL)
        {
            cpssOsFclose(thermalZone1Fp);
            cpssOsFree(thermalZone1Fp);
        }

        if (killTaskEnPtr)
        {
            fanManagementTaskKill = GT_TRUE;
        }
    }

    return;
}

/**
* @internal falcon_DB_PIDInit function
* @endinternal
*
* @brief    Initiallize the sturcture that holds the PID data,
*           set the appropriate SetPoint tempertaure for each of the sensors.
*
*/
static GT_VOID falcon_DB_PIDInit
(
    IN PID_STC      *pidPtr,
    IN GT_FLOAT64   kp,
    IN GT_FLOAT64   ki,
    IN GT_FLOAT64   kd,
    APP_DEMO_PID_SENSORS_ENT   sensor
)
{
    GT_FLOAT64  setTemp;

    pidPtr->kp = kp;
    pidPtr->ki = ki;
    pidPtr->kd = kd;

    pidPtr->referenceTemp = 0;
    pidPtr->error_previous = 0;
    pidPtr->integral = 0;

    pidPtr->features = 0;
    pidPtr->sensor = sensor;

    switch(sensor)
    {
        case APP_DEMO_EAGLES_RAVENS_THERMAL_SENSOR_E:
            setTemp = 80;
            break;
        case APP_DEMO_CPU_THERMAL_SENSOR_E:
            setTemp = 80;
            break;
        case APP_DEMO_BOARD_THERMAL_SENSOR_E:
            setTemp = 70;
            break;
        case APP_DEMO_TRANSIVER_THERMAL_SENSOR_E:
            setTemp = 60;
            break;
        default:
            setTemp = 0;
    }

    /* enable windup feature
       - value was manually chosen for best control */
    falcon_DB_pidEnableFeature(pidPtr,PID_ENABLE_WINDUP,(maxOutput - 5*maxOutput/8));

    /* output value of PID can be positive - we dont want to heat the board just cool it */
    falcon_DB_pidEnableFeature(pidPtr,PID_OUTPUT_SAT_MAX,0);
    falcon_DB_PIDSetSP(pidPtr, setTemp);
}

/**
* @internal falcon_DB_PIDPOutputCalculate function
* @endinternal
*
* @brief returns the output of the PID
*
*/
static GT_FLOAT64 falcon_DB_PIDPOutputCalculate
(
    IN PID_STC      *pid,
    IN GT_FLOAT64   currTemp,
    IN GT_FLOAT64   deltaT
)
{
    float newIntergral,newDerivative, error, total;

    error = pid->referenceTemp - currTemp;
    newIntergral = pid->integral + (error * deltaT);
    newDerivative = (error - pid->error_previous) / deltaT;

    total = (error * pid->kp) + (newIntergral * pid->ki) + (newDerivative * pid->kd);

    if (WINDUP_ON(pid))
    {
        if ( newIntergral < 0 )
            newIntergral = ( newIntergral < -pid->intmax ? -pid->intmax : newIntergral );
        else
            newIntergral = ( newIntergral < pid->intmax ? newIntergral : pid->intmax );
    }
    pid->integral = newIntergral;

    if ( SAT_MIN_ON(pid) && (total < pid->sat_min) )
        total =  pid->sat_min;
    if ( SAT_MAX_ON(pid) && (total > pid->sat_max) )
        total = pid->sat_max;

    if (cpss_thermal_debug)
        printf("Sensor: %s - current temp = %.1f, referenceTemp = %.1f, val = %.1f\n",
               sensorsNamesArr[pid->sensor], currTemp, pid->referenceTemp, total);

    pid->error_previous = error;

    /* total will always be negative - the more negative the higher speed should be set */
    return ABS(total);
}

/****************************************/
/* DB_falcon thermal task main function */
/****************************************/

/**
* @internal falcon_DB_MaxRavensTemperature function
* @endinternal
*
* @brief returns the maximum temperature of the Ravens
*
*/
static GT_U32 falcon_DB_MaxRavensTemperature
(
    IN GT_U8 devNum
)
{
    GT_U32 raven;
    GT_U32 maxTemp = 0;
    GT_32 currTemp;
    GT_STATUS rc;

    /* get maximum temperature of the Ravens */
    for (raven=0 ; raven< RAVEN_MAX_NUMBER ; raven++)
    {
        if (ravenPortsArr[raven] != PORT_NOT_FOUND)
        {
            rc = cpssDxChPortDiagTemperatureGet(devNum, (GT_U32)ravenPortsArr[raven], &currTemp);
            if(rc != GT_OK)
            {
                cpssOsPrintf("cpssDxChPortDiagTemperatureGet failed - set maximum fan speed\n");
                falcon_DB_setMaxFanSpeedAndExit((GT_VOID*)GT_TRUE);
            }
            if ((GT_U32)currTemp > maxTemp)
                maxTemp = (GT_U32)currTemp;
        }
    }
    return (GT_U32)maxTemp;
}

/**
* @internal falcon_DB_initPortPerRavenArray function
* @endinternal
*
* @brief initialize array with port numbers where there is only one port from each raven.
*
*/
static GT_STATUS falcon_DB_initPortPerRavenArray
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc;
    GT_U32  portIterator; /* port iterator */
    GT_U32  macNum = 0; /* port mac number */
    GT_U32  ravenNumber; /* raven number */
    CPSS_DXCH_PORT_MAP_STC  portMap; /* port information */

    /* iterate over all ports in the device and map them to raven number */
    for(portIterator = 0; portIterator < appDemoPpConfigList[devNum].maxPortNumber ; portIterator++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portIterator);

        /* get port's information */
        rc = cpssDxChPortPhysicalPortMapGet(devNum, portIterator, 1, /*OUT*/&portMap);
        if (rc != GT_OK || portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
            continue;
        }

        macNum = portMap.interfaceNum;

        if((64 * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles) <= macNum)
        {
            continue;
        }

        ravenNumber = macNum / 16;

        /* add raven to the ports array if it doesn't exsist in the array */
        if (ravenPortsArr[ravenNumber] == PORT_NOT_FOUND)
        {
            ravenPortsArr[ravenNumber] =  portIterator;
        }
    }

    return GT_OK;
}

/**
* @internal falcon_DB_MaxEaglesTemperature function
* @endinternal
*
* @brief returns the maximum temperature of the Eagle
*
*/
static GT_U32 falcon_DB_MaxEaglesTemperature
(
    IN GT_U8 devNum
)
{
    GT_U32 sensor;
    GT_32 maxTemp = 0;
    GT_32 currTemp;
    GT_STATUS rc;

    /* get maximum temperature of the eagles */
    for (sensor=0 ; sensor <= CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_1_E ; sensor++)
    {
        rc = cpssDxChDiagDeviceTemperatureSensorsSelectSet(devNum, sensor);
        if(rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChDiagDeviceTemperatureSensorsSelectSet failed - set maximum fan speed\n");
            falcon_DB_setMaxFanSpeedAndExit((GT_VOID*)GT_TRUE);
        }

        rc = cpssDxChDiagDeviceTemperatureGet(devNum, &currTemp);
        if(rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChDiagDeviceTemperatureGet failed - set maximum fan speed\n");
            falcon_DB_setMaxFanSpeedAndExit((GT_VOID*)GT_TRUE);
        }

        if (currTemp > maxTemp)
        {
            maxTemp = currTemp;
        }
    }
    return (GT_U32)maxTemp;
}

/**
* @internal falcon_DB_CPUThermalSensor function
* @endinternal
*
* @brief returns the maximum temperature of the CPU
*
* @note  can be used to read all armV8 CPU temperature
*
*/
static GT_U32 falcon_DB_CPUThermalSensor
(
    GT_VOID
)
{
    GT_U32  temp1, maxTemp;
    GT_U32  nmemb;
    GT_CHAR buffer[BUFFER_SIZE];
    int     retVal;

    if (thermalZone1Fp == NULL || thermalZone0Fp == NULL)
    {
        cpssOsPrintf("Failed to read CPU temperature - return dummy value\n");
        return 400;
    }

    /* read thermal_zone0 temperature */
    nmemb = cpssOsFgetLength(thermalZone0Fp->fd);
    if (nmemb <= 0)
    {
        cpssOsPrintf("Failed to read CPU temperature\n");
        falcon_DB_setMaxFanSpeedAndExit((GT_VOID*)GT_TRUE);
    }

    retVal = cpssOsFread(buffer, 1, nmemb, thermalZone0Fp);
    if (retVal < 0)
    {
        cpssOsPrintf("Failed to read CPU temperature\n");
        falcon_DB_setMaxFanSpeedAndExit((GT_VOID*)GT_TRUE);
    }

    maxTemp = osStrTo32((char*)buffer);

    /* read thermal_zone1 temperature */
    nmemb = cpssOsFgetLength(thermalZone1Fp->fd);
    if (nmemb <= 0)
    {
        cpssOsPrintf("Failed to read CPU temperature\n");
        falcon_DB_setMaxFanSpeedAndExit((GT_VOID*)GT_TRUE);
    }

    retVal = cpssOsFread(buffer, 1, nmemb, thermalZone1Fp);
    if (retVal < 0)
    {
        cpssOsPrintf("Failed to read CPU temperature\n");
        falcon_DB_setMaxFanSpeedAndExit((GT_VOID*)GT_TRUE);
    }

    temp1 = osStrTo32((char*)buffer);

    /* check maximum temperature */
    if (temp1 > maxTemp)
    {
        maxTemp = temp1;
    }

    /* move file pointers back to the begging of the file */
    cpssOsRewind(thermalZone0Fp->fd);
    cpssOsRewind(thermalZone1Fp->fd);

    return maxTemp/1000; /* max CPU temperature in degrees*/
}

/**
* @internal falcon_DB_BoardThermalSensor function
* @endinternal
*
* @brief returns the temperature of the Board Thermal Sensor
*
*/
static GT_U16 falcon_DB_BoardThermalSensor
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U8 tempArr[2]; /* Board Thermal Temp is 2B long */
    GT_U16 currTemp;

    rc = appDemoHostI2cWrite(0/*bus id*/,0x77/*slave address*/,0/*offset_type: 8bit*/, 0 /*offset ??????*/,1/*number of args*/,0x7/*arg1*/,0/*arg2*/,0/*arg3*/,0/*arg4*/);
    if(rc != GT_OK)
    {
        falcon_DB_setMaxFanSpeedAndExit((GT_VOID*)GT_TRUE);
        /* failed accessing to board's thermal mux return dummy value to exit this function */
        return 400;
    }
    rc = appDemoHostI2cRead(0/*bus id*/,0x48/*slave address*/,0/*offset_type: 8bit*/, 0 /*offset ??????*/,8/*size - number of Bytes*/, tempArr);
    if(rc != GT_OK)
    {
        falcon_DB_setMaxFanSpeedAndExit((GT_VOID*)GT_TRUE);
    }

    currTemp = ((((GT_U16)tempArr[0] << 8) & 0xFF00 ) | (tempArr[1] & 0x00FF)) >> 5;/* takes only 10 MSBs from the register */

    /*bit 10 of currTemp is Positive/Negative bit*/
    if (currTemp & 0x400)
    {
        currTemp = 0;
    }
    else
    {
        currTemp &= 0x3FF; /*bit 10 of currTemp is Positive/Negative bit*/
    }

    return currTemp * 0.125; /* according to FS */
}

/**
* @internal falcon_DB_TransceiversFalconSetChannel function
* @endinternal
*
* @brief set PCA0548 MUX according to the board cage index
*
*/
static GT_STATUS falcon_DB_TransceiversFalconSetChannel
(
    IN  GT_U32  cageIndex,
    IN  GT_U8   data
)
{
    GT_U8 i, muxNum, activeMux;

    muxNum = 4; /*number of MUXs (PCA0548)*/
    activeMux = cageIndex/8; /*MUX index*/

    /*open the relevant MUX, close all other*/
    for (i = 0; i<muxNum ; i++)
    {
        if (i == activeMux)
        {
            return appDemoHostI2cWrite(0/*bus id*/,APP_DEMO_AUTO_DETECT_MODULE_MUX9548_BASE_ADDRESS+i/*slave address*/,1/*offset_type: 8bit*/,0x0/*offset*/,1/*number of args*/,data/*data*/,0,0,0);
        }
        else
        {
            return appDemoHostI2cWrite(0/*bus id*/,APP_DEMO_AUTO_DETECT_MODULE_MUX9548_BASE_ADDRESS+i/*slave address*/,1/*offset_type: 8bit*/,0x0/*offset*/,1/*number of args*/,0x00/*data*/,0,0,0);
        }
    }

    return GT_OK;
}

/**
* @internal falcon_DB_TransceiversPresentStatusGet function
* @endinternal
*
* @brief return GT_TRUE if there is QSFP inside speciefic cage.
*        return GT_FALSE otherwise.
*
*/
static GT_STATUS falcon_DB_TransceiversPresentStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U8                   cageIndex,
    OUT GT_BOOL                 *presentStatus
)
{
    GT_U8 val=0;
    GT_STATUS rc;
    devNum = devNum;

    if(presentStatus == NULL)
    {
        return GT_BAD_PARAM;
    }

    /*Set Active MUX*/
    rc = falcon_DB_TransceiversFalconSetChannel(cageIndex,1);

    if(rc != GT_OK) {
        return rc;
    }

    /*read present bits*/
    rc = appDemoHostI2cRead(0,APP_DEMO_AUTO_DETECT_MODULE_FALCON_PRESENT_STATUS_BASE_ADDRESS,
                            1 /*offset type: 8 bit*/,
                            0 /*offset size*/,
                            4 /*size: 1 Byte*/,&val);

    if(rc == GT_OK)
    {
        /*Check status for the current cage - status is true if 0 (active low)*/
        cageIndex = cageIndex % 8; /* cage index in the presentStatus bits */
        *presentStatus = (((1 << cageIndex) & ((~val) & 0xFF)) == 0) ? GT_FALSE : GT_TRUE;
    }

    return rc;
}

/**
* @internal falcon_DB_appDemoTransceiversFieldInfoGet function
* @endinternal
*
* @brief gets info from moudle in selected QSFP cage
*/
static GT_STATUS falcon_DB_appDemoTransceiversFieldInfoGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U8                   cageIndex,
    IN  GT_U8                   pageNum,
    IN  GT_U8                   offset,
    IN  GT_U8                   size,
    OUT GT_U8                   *data
)
{
    GT_BOOL presentStatus = GT_FALSE;
    GT_STATUS rc = GT_OK;

    if(data == NULL)
    {
        return GT_BAD_PARAM;
    }

    /*Check Transceiver In*/
    rc = falcon_DB_TransceiversPresentStatusGet(devNum, cageIndex, &presentStatus);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(presentStatus)
    {
        /*Set Active PCA0548 MUX*/
        rc = falcon_DB_TransceiversFalconSetChannel(cageIndex,
                                                    (0x01 << (cageIndex%8) /* cage index in the presentStatus bits */ ));
        if(rc != GT_OK)
        {
            return rc;
        }

        /*Set Page Num*/
        if(pageNum)
        {
            rc = appDemoHostI2cWrite(0/*bus id*/,
                                     APP_DEMO_AUTO_DETECT_MODULE_EEPROM_BASE_ADDRESS/*slave address*/,
                                     1/*offset_type: 8bit*/,0x7f/*offset*/,
                                     1/*number of args*/,pageNum/*data*/,0,0,0);
        }
        if(rc != GT_OK)
        {
            return rc;
        }

        /*Read Data*/
        rc = appDemoHostI2cRead(0,APP_DEMO_AUTO_DETECT_MODULE_EEPROM_BASE_ADDRESS,
                                1/*offset type: 8 bit*/,
                                offset,size/*size: 1 Byte*/,data);
        if(rc != GT_OK)
        {
            return rc;
        }

        /*Set Page Num back to 0*/
        if(pageNum)
        {
            rc = appDemoHostI2cWrite(0/*bus id*/,
                                     APP_DEMO_AUTO_DETECT_MODULE_EEPROM_BASE_ADDRESS/*slave address*/,
                                     1/*offset_type: 8bit*/,0x7f/*offset*/,
                                     1/*number of args*/,0/*data*/,0,0,0);
        }
    }
    else
    {
        return GT_NOT_FOUND;
    }

    return rc;
}

/**
* @internal falcon_DB_MaxTransceiversSensor function
* @endinternal
*
* @brief    Returns the maximum temperature out of all the
*           transceivers which inserted to the board.
*           This function does not kill the task on any failure!
*/
static GT_U16 falcon_DB_MaxTransceiversSensor
(
    IN  GT_SW_DEV_NUM           devNum
)
{
    GT_STATUS   rc;
    GT_U16      data = 0;
    GT_U8       temperature = 0;
    GT_U8       MaxTemperature = 0;
    GT_U8       cageIndex;

    /* iterate over all cages */
    for (cageIndex = 0 ; cageIndex < APP_DEMO_FALCON_12_8_CAGE_ARR_SIZE; cageIndex++ )
    {
        /*Check if Temperature is supported (page 1, byte 159, bit 0)*/
        rc = falcon_DB_appDemoTransceiversFieldInfoGet(devNum,cageIndex,
                                                       APP_DEMO_AUTO_DETECT_TEMPERATURE_SUPPORT_PAGE,
                                                       APP_DEMO_AUTO_DETECT_TEMPERATURE_SUPPORT_OFFSET,
                                                       APP_DEMO_AUTO_DETECT_TEMPERATURE_SUPPORT_LEN,
                                                       (GT_U8 *)&data);
        if(rc == GT_OK)
        {
            if (!(data & 0x01))
            {
                /* Temperature not supported */
                continue;
            }
        }
        else if (rc == GT_NOT_FOUND)
        {
            continue;

        }
        else
        {
            /* disable I2C error printing to avoid multipy error prints for the same transceiver */
            appDemoHostI2cPrintErrorEnableSet(GT_FALSE);
            continue;
        }

        /*Get Temperature (Lower page, byte 14-15)*/
        rc = falcon_DB_appDemoTransceiversFieldInfoGet(devNum,cageIndex,
                                                       APP_DEMO_AUTO_DETECT_TEMPERATURE_PAGE,
                                                       14, 1, &temperature);
        if(rc == GT_OK)
        {
            /*byte 14 -> Temperature MSB; byte 15 -> Temperature LSB; signed 2's complement*/
            MaxTemperature = (temperature > MaxTemperature)? temperature: MaxTemperature;
        }
        else
        {
            /*appDemoTransceiversAutoDetectGetTemperature failed*/
            continue;
        }
    }
    return MaxTemperature;
}

/**
* @internal appDemoFalcon_DB_sensorTemperaturePrint function
* @endinternal
*
* @brief Print selected or all falcon DB 12.8 temperature sensors
*
* @param[in] devNum         - device number
* @param[in] sensor         - selected sensor
*
*/
GT_STATUS appDemoFalcon_DB_sensorTemperaturePrint
(
    GT_U8           devNum,
    APP_DEMO_PID_SENSORS_ENT   sensor
)
{
    GT_U32 temp;
    GT_U8 buffer;
    GT_STATUS rc;

    switch(sensor)
    {
        case APP_DEMO_EAGLES_RAVENS_THERMAL_SENSOR_E:
            temp = falcon_DB_MaxEaglesTemperature(devNum);
            cpssOsPrintf("falcon_DB_MaxEaglesTemperature = [%d]\n", temp);
            temp = falcon_DB_MaxRavensTemperature(devNum);
            cpssOsPrintf("falcon_DB_MaxRavensTemperature = [%d]\n", temp);
            break;
        case APP_DEMO_CPU_THERMAL_SENSOR_E:
            temp = falcon_DB_CPUThermalSensor();
            cpssOsPrintf("falcon_DB_CPUThermalSensor = [%d]\n", temp);
            break;
        case APP_DEMO_BOARD_THERMAL_SENSOR_E:
            /* check if JP10/JP11 jumper is connected */
            rc = appDemoHostI2cRead(0/*bus id*/,0x77/*slave address*/,0/*offset_type: 8bit*/, 0 /*offset */,4/*size - number of Bytes*/, &buffer);
            if(rc != GT_OK)
            {
                cpssOsPrintf("ERROR: can't read board's thermal sensor \n");
                break;
            }
            temp = falcon_DB_BoardThermalSensor();
            cpssOsPrintf("falcon_DB_BoardThermalSensor = [%d]\n", temp);
            break;
        case APP_DEMO_TRANSIVER_THERMAL_SENSOR_E:
            temp = (GT_U16)falcon_DB_MaxTransceiversSensor(devNum);
            cpssOsPrintf("falcon_DB_MaxTransceiversSensor = [%d]\n", temp);
            break;
        default:
            temp = falcon_DB_MaxEaglesTemperature(devNum);
            cpssOsPrintf("falcon_DB_MaxEaglesTemperature = [%d]\n", temp);
            temp = falcon_DB_CPUThermalSensor();
            cpssOsPrintf("falcon CPU temperature = [%d]\n", temp);
            temp = falcon_DB_MaxRavensTemperature(devNum);
            cpssOsPrintf("falcon_DB_MaxRavensTemperature = [%d]\n", temp);
            temp = falcon_DB_MaxTransceiversSensor(devNum);
            cpssOsPrintf("falcon_DB_MaxTransceiversSensor = [%d]\n", temp);
            /* check if JP10/JP11 jumper is connected */
            rc = appDemoHostI2cRead(0/*bus id*/,0x77/*slave address*/,0/*offset_type: 8bit*/, 0 /*offset */,4/*size - number of Bytes*/, &buffer);
            if(rc != GT_OK)
            {
                cpssOsPrintf("ERROR: can't read board's thermal sensor \n");
                break;
            }
            temp = falcon_DB_BoardThermalSensor();
            cpssOsPrintf("falcon_DB_BoardThermalSensor = [%d]\n", temp);
            break;
    }
    return GT_OK;
}

/**
 *@internal appDemoFalcon_DB_ThermalTaskDebugEnableSet function
 *@endinternal
 *
 * @brief enable/disable falcon DB 12.8 Fan Mangment task debug.
 *
 * @param[in] en -  true - enable thermal task debugging
 *                  false - disable thermal task debugging
 *
 * @return GT_OK
 *
 * @note for debug purposes only
 *
 */
GT_STATUS  appDemoFalcon_DB_ThermalTaskDebugEnableSet
(
    IN GT_BOOL en
)
{
    cpss_thermal_debug = en;
    return GT_OK;
}

/**
 *@internal appDemoFalcon_DB_ThermalTaskEnableSet function
 *@endinternal
 *
 * @brief enable/disable falcon DB 12.8 Fan Mangment task.
 *
 * @param [in] en - true - enable thermal task
 *                  false - disable thermal task
 *
 * @return GT_OK
 *
 * @note for debug purposes only. The task will still run in backround but will
 *       not change fan speed.
 */
GT_STATUS appDemoFalcon_DB_ThermalTaskEnableSet
(
    IN GT_BOOL en
)
{
    cpss_db_falcon_thermal_en = en;
    return GT_OK;
}

/**
 * @internal falcon_DB_PidFanSpeedGet function
 *
 * @brief calculate the fan speed, for each of the sensors.
 *
 * @param [in]  devNum   -   device number.
 * @param [in] *pidPtr -    (pointer to) PID data of the sensor.
 *
 * @return THERMAL_STATE
 *
 */
static FAN_SPEED_STATE_ENT falcon_DB_PidFanSpeedGet
(
    GT_U8   devNum,
    PID_STC *pidPtr
)
{
    GT_U32      temp;
    GT_U32      maxTemp = 0;
    GT_FLOAT64  pidOutput;
    FAN_SPEED_STATE_ENT speedState = FAN_SPEED_70_E;

    switch(pidPtr->sensor)
    {
        case APP_DEMO_EAGLES_RAVENS_THERMAL_SENSOR_E:
            maxTemp = falcon_DB_MaxEaglesTemperature(devNum);
            temp = falcon_DB_CPUThermalSensor();
            maxTemp = (temp > maxTemp)? temp : maxTemp;
            break;
        case APP_DEMO_CPU_THERMAL_SENSOR_E:
            maxTemp = falcon_DB_MaxRavensTemperature(devNum);
            break;
        case APP_DEMO_BOARD_THERMAL_SENSOR_E:
            maxTemp = falcon_DB_BoardThermalSensor();
            break;
        case APP_DEMO_TRANSIVER_THERMAL_SENSOR_E:
            maxTemp = (GT_U16)falcon_DB_MaxTransceiversSensor(devNum);
            break;
    }

    pidOutput =  falcon_DB_PIDPOutputCalculate(pidPtr, maxTemp, TIME_INTERVAL);

    /* convert PID output to fan speed using the PID max output global variable
       which is calculated in falcon_DB_MaxPidOutputGet() func */
    speedState =    (pidOutput > (maxOutput - 2*maxOutput/8)) ? FAN_SPEED_70_E :
                    (pidOutput > (maxOutput - 3*maxOutput/8)) ? FAN_SPEED_60_E :
                    (pidOutput > (maxOutput - 4*maxOutput/8)) ? FAN_SPEED_50_E :
                    (pidOutput > (maxOutput - 5*maxOutput/8)) ? FAN_SPEED_40_E :
                    (pidOutput > (maxOutput - 6*maxOutput/8)) ? FAN_SPEED_30_E :
                    (pidOutput > (maxOutput - 7*maxOutput/8)) ? FAN_SPEED_20_E :
                    (pidOutput > (maxOutput - 8*maxOutput/8)) ? FAN_SPEED_10_E :
                                                                FAN_SPEED_DEFAULT_E;
    return speedState;
}

/**
 * @internal falcon_DB_appendArrToFile function
 *
 * @brief save array into file
 *
 */
#ifdef INCLUDE_UTF
static GT_VOID falcon_DB_appendArrToFile
(
    CPSS_OS_FILE_TYPE_STC *fp,
    GT_U32 inputArr[],
    GT_U32 arrSize
)
{
    GT_U32 ii;

    cpssOsFprintf (fp->fd, "[");     /* print closing tag */
    for (ii = 0; ii < arrSize; ii++)            /* print each integer   */
        if (ii == 0)
            cpssOsFprintf (fp->fd, " %d", inputArr[ii]);
        else
            cpssOsFprintf (fp->fd, ", %d", inputArr[ii]);
    cpssOsFprintf (fp->fd, " ]\n");     /* print closing tag */
}
#endif

/**
 * @internal falcon_DB_PidFanSpeedGet function
 *
 * @brief   this function get the PID parameters, simulate rising temperature,
 *          and set the global maxOutput variable.
 *
 */
static GT_VOID falcon_DB_MaxPidOutputGet
(
    IN GT_FLOAT64  pIn,
    IN GT_FLOAT64  iIn,
    IN GT_FLOAT64  dIn
)
{
    PID_STC falcon_DB_MaxPidOutputGet; /*={eaglesRavensPID,cpuPID,boardPID,transiversPID}; *//* array contains all the PID controllers */
    GT_U32 temp[] = {60,70,80,90,90,90,90,100};
    GT_FLOAT64  pidOutput;
    GT_U32 kk;

    cpssOsMemSet(&falcon_DB_MaxPidOutputGet, 0, sizeof(falcon_DB_MaxPidOutputGet));
    falcon_DB_PIDInit(&falcon_DB_MaxPidOutputGet, pIn /*p_factor*/, iIn/*i_factor*/,
                      dIn/*d_factor*/, APP_DEMO_EAGLES_RAVENS_THERMAL_SENSOR_E );
    /* get maximun fan speed from all the sensors */
    for (kk=0;kk<sizeof(temp)/sizeof(temp[0]);kk++)
    {
        pidOutput =  falcon_DB_PIDPOutputCalculate(&falcon_DB_MaxPidOutputGet, temp[kk], 10);
        maxOutput = (pidOutput > maxOutput)? pidOutput: maxOutput;
        minOutput = (pidOutput < minOutput)? pidOutput: minOutput;
    }
    return;
}

/**
 * @internal falcon_DB_PIDtest function
 *
 * @brief   PID test - the test has three stages:
 *          - Idle with  ports up (15min duration)
 *          - Full traffic on all the ports  (30min duration)
 *          - Idle with  ports up (15min duration)
 *
 *          1)  The test configures the PID parametes and save all the sensors temperature.
 *          2)  Export all the data into PID_test.txt file in JSON format in the same dir the appDemo is running
 *          3)  To plot graphs of the data use external pyhton script located in /tools dir
 *
 * @note    Delete PID_test.txt from appDemo dir before running the test
 *
 */
GT_VOID falcon_DB_PIDtest
(
    GT_VOID
)
{
#ifdef INCLUDE_UTF
    GT_FLOAT64  p[] = {8};
    GT_FLOAT64  i[] = {4};
    GT_FLOAT64  d= 5;
    GT_U32 ii,kk,pp;

    /* the temperature arrays size formula is
       test time in seconds / deltaT */
    GT_U32 CPUTemp[90+180+90];
    GT_U32 eagleTemp[90+180+90];
    GT_U32 ravensTemp[90+180+90];
    GT_U32 boardTemp[90+180+90];
    GT_U32 transiversTemp[90+180+90];
    GT_U32 speed[90+180+90];

    /*open file in append mode*/
    char filePath[]= "PID_test.txt";
    CPSS_OS_FILE_TYPE_STC    *fp;
    fp = cpssOsMalloc (sizeof(CPSS_OS_FILE_TYPE_STC));
    fp->type = CPSS_OS_FILE_REGULAR;
    fp->fd = cpssOsFopen(filePath, "a+",fp);
    if (0 == fp->fd)
    {
        cpssOsPrintf("\nUnable to open '%s' file.\n", filePath);

    }

    cpssOsFprintf(fp->fd,"{\"PID list\":[\n"); /*open tags 1+2*/

    for (pp=0;pp<sizeof(p)/sizeof(p[0]);pp++)
    {
        for (ii=0;ii<sizeof(i)/sizeof(i[0]);ii++)
        {
            /* PID output to speed update per given PID params */
            falcon_DB_MaxPidOutputGet(p[pp] /*p_factor*/, i[ii] /*i_factor*/, d/*d_factor*/);

            cpssOsMemSet(pidList, 0, sizeof(pidList));
            falcon_DB_PIDInit(&pidList[0], p[pp] /*p_factor*/, i[ii] /*i_factor*/, d/*d_factor*/, APP_DEMO_EAGLES_RAVENS_THERMAL_SENSOR_E);
            falcon_DB_PIDInit(&pidList[1], p[pp] /*p_factor*/, i[ii] /*i_factor*/, d/*d_factor*/, APP_DEMO_CPU_THERMAL_SENSOR_E);
            falcon_DB_PIDInit(&pidList[2], p[pp] /*p_factor*/, i[ii] /*i_factor*/, d/*d_factor*/, APP_DEMO_BOARD_THERMAL_SENSOR_E);
            falcon_DB_PIDInit(&pidList[3], p[pp] /*p_factor*/, i[ii] /*i_factor*/, d/*d_factor*/, APP_DEMO_TRANSIVER_THERMAL_SENSOR_E);

            /* cool the device */
            /* stop Thermal task */
            appDemoFalcon_DB_ThermalTaskEnableSet(GT_FALSE);
            /* set max speed for 2 minuts */
            appDemoFalcon_DB_FanSpeedSet(7);
            prvTgfFWSRestore();
            osTimerWkAfter(60000);

            /* enable Thermal task */
            appDemoFalcon_DB_ThermalTaskEnableSet(GT_TRUE);

            cpssOsPrintf("\nPart '1' - IDLE for 15 min + ports up \n");
            /* save Bias Temp Every 10 sec for 15 minutes */
            for (kk=0; kk<90 ;kk++)
            {
                eagleTemp[kk] = falcon_DB_MaxEaglesTemperature(0);
                CPUTemp[kk] = falcon_DB_CPUThermalSensor();
                ravensTemp[kk] = falcon_DB_MaxRavensTemperature(0);
                boardTemp[kk] = falcon_DB_BoardThermalSensor();
                transiversTemp[kk] = (GT_U16)falcon_DB_MaxTransceiversSensor(0);
                speed[kk] = appDemoFalcon_DB_FanSpeedDump();
                osTimerWkAfter(10000); /* check temperature every 10 sec */
            }

            /* run test */
            prvTgfFWSTest(1);

            cpssOsPrintf("\nPart '2' - full traffic in all ports for 30 min \n");
            /* save Bias Temp Every 10 sec for 30 minutes */
            for (; kk<90+180 ;kk++)
            {
                eagleTemp[kk] = falcon_DB_MaxEaglesTemperature(0);
                CPUTemp[kk] = falcon_DB_CPUThermalSensor();
                ravensTemp[kk] = falcon_DB_MaxRavensTemperature(0);
                boardTemp[kk] = falcon_DB_BoardThermalSensor();
                transiversTemp[kk] = (GT_U16)falcon_DB_MaxTransceiversSensor(0);
                speed[kk] = appDemoFalcon_DB_FanSpeedDump();
                osTimerWkAfter(10000); /* check temperature every 10 sec */
            }

            /* restore configurations */
            prvTgfFWSRestore();

            cpssOsPrintf("\nPart '3' - IDLE for 15 min + ports up \n");
            /* save Bias Temp Every 10 sec for 15 minutes */
            for (; kk<sizeof(CPUTemp)/sizeof(CPUTemp[0]) ;kk++)
            {
                eagleTemp[kk] = falcon_DB_MaxEaglesTemperature(0);
                CPUTemp[kk] = falcon_DB_CPUThermalSensor();
                ravensTemp[kk] = falcon_DB_MaxRavensTemperature(0);
                boardTemp[kk] = falcon_DB_BoardThermalSensor();
                transiversTemp[kk] = (GT_U16)falcon_DB_MaxTransceiversSensor(0);
                speed[kk] = appDemoFalcon_DB_FanSpeedDump();
                osTimerWkAfter(10000); /* check temperature every 10 sec */
            }

            /* save results into file*/

            cpssOsFprintf(fp->fd,"{\"name\": \"p = %0.1f , i = %0.1f, d = %0.1f\",\n\"sensors\":[\n",p[pp],i[ii],d);/* open tags 3+4*/

            cpssOsFprintf(fp->fd,"{\"name\":\"eagleTemp\",\n\"temperature\":\n");
            falcon_DB_appendArrToFile(fp, eagleTemp,sizeof(eagleTemp)/sizeof(eagleTemp[0]));
            cpssOsFprintf (fp->fd, "},\n");     /* print closing tag */

            cpssOsFprintf(fp->fd,"{\"name\":\"CPUTemp\",\n\"temperature\":\n");
            falcon_DB_appendArrToFile(fp, CPUTemp,sizeof(CPUTemp)/sizeof(CPUTemp[0]));
            cpssOsFprintf (fp->fd, "},\n");     /* print closing tag */

            cpssOsFprintf(fp->fd,"{\"name\":\"ravensTemp\",\n\"temperature\":\n");
            falcon_DB_appendArrToFile(fp, ravensTemp,sizeof(ravensTemp)/sizeof(ravensTemp[0]));
            cpssOsFprintf (fp->fd, "},\n");     /* print closing tag */

            cpssOsFprintf(fp->fd,"{\"name\":\"boardTemp\",\n\"temperature\":\n");
            falcon_DB_appendArrToFile(fp, boardTemp,sizeof(boardTemp)/sizeof(boardTemp[0]));
            cpssOsFprintf (fp->fd, "},\n");     /* print closing tag */

            cpssOsFprintf(fp->fd,"{\"name\":\"transiversTemp\",\n\"temperature\":\n");
            falcon_DB_appendArrToFile(fp, transiversTemp,sizeof(transiversTemp)/sizeof(transiversTemp[0]));
            cpssOsFprintf (fp->fd, "},\n");     /* print closing tag */

            cpssOsFprintf(fp->fd,"{\"name\":\"speed\",\n\"speed\":\n");
            falcon_DB_appendArrToFile(fp, speed,sizeof(speed)/sizeof(speed[0]));
            cpssOsFprintf (fp->fd, "}\n");     /* print closing tag */

            cpssOsFprintf (fp->fd, "]\n");  /*closing tag 4*/

            cpssOsFprintf (fp->fd, "}\n");   /*closing tag 3*/

            if ( (pp==sizeof(p)/sizeof(p[0]) -1 ) && (ii==sizeof(i)/sizeof(i[0])-1) )
            {
                cpssOsFprintf(fp->fd, "\n");
            }
            else
            {
                cpssOsFprintf(fp->fd, ",\n");
            }
        }
    }
    cpssOsFprintf (fp->fd, "]}\n"); /* closing tags 2+1*/

    cpssOsFclose(fp);
    cpssOsFree(fp);

    falcon_DB_PIDInit(&pidList[0], PID_P_PARAM /*p_factor*/, PID_I_PARAM /*i_factor*/, PID_D_PARAM /*d_factor*/, APP_DEMO_EAGLES_RAVENS_THERMAL_SENSOR_E);
    falcon_DB_PIDInit(&pidList[1], PID_P_PARAM /*p_factor*/, PID_I_PARAM /*i_factor*/, PID_D_PARAM /*d_factor*/, APP_DEMO_CPU_THERMAL_SENSOR_E);
    falcon_DB_PIDInit(&pidList[2], PID_P_PARAM /*p_factor*/, PID_I_PARAM /*i_factor*/, PID_D_PARAM /*d_factor*/, APP_DEMO_BOARD_THERMAL_SENSOR_E);
    falcon_DB_PIDInit(&pidList[3], PID_P_PARAM /*p_factor*/, PID_I_PARAM /*i_factor*/, PID_D_PARAM /*d_factor*/, APP_DEMO_TRANSIVER_THERMAL_SENSOR_E);
#else
    cpssOsPrintf("\nCan not run test - must inclued UTF\n");
#endif
}

/**
 * @internal falcon_DB_ThermalTask function
 *
 * @return unsigned __TASKCONV
 */
static unsigned __TASKCONV falcon_DB_ThermalTask
(
    GT_VOID *param
)
{
    GT_U8       devNum = (GT_U8)(GT_UINTPTR) param;
    GT_U8       ii;         /*iterator*/
    GT_STATUS   rc = GT_OK; /*return code*/

    /* PID parametes */
    GT_FLOAT64  pTerm = PID_P_PARAM;
    GT_FLOAT64  iTerm = PID_I_PARAM;
    GT_FLOAT64  dTerm = PID_D_PARAM;

    FAN_SPEED_STATE_ENT tempFanSpeed; /* temporary fan speed state - holds the speed calculated for each of the sensors */
    FAN_SPEED_STATE_ENT currentMaxFanSpeed; /* holds the maximum fan speed calculated out of all the sensors , for each iteration */
    FAN_SPEED_STATE_ENT prevFanSpeed = FAN_SPEED_DEFAULT_E; /* =0x3 - the default value when the board turns on */

    cpssOsMemSet(pidList, 0, sizeof(pidList));
    falcon_DB_PIDInit(&pidList[0], pTerm, iTerm, dTerm, APP_DEMO_EAGLES_RAVENS_THERMAL_SENSOR_E);
    falcon_DB_PIDInit(&pidList[1], pTerm, iTerm, dTerm, APP_DEMO_CPU_THERMAL_SENSOR_E);
    falcon_DB_PIDInit(&pidList[2], pTerm, iTerm, dTerm, APP_DEMO_BOARD_THERMAL_SENSOR_E);
    falcon_DB_PIDInit(&pidList[3], pTerm, iTerm, dTerm, APP_DEMO_TRANSIVER_THERMAL_SENSOR_E);

    /* PID output to speed update per given PID params */
    falcon_DB_MaxPidOutputGet(pTerm, iTerm, dTerm);

    cpssOsPrintf("Fan Management Task Created\n");

    /* state that the task supports 'Graceful exit' */
    appDemoTaskSupportGracefulExit(falcon_DB_setMaxFanSpeedAndExit,(GT_VOID*)GT_FALSE);
    /* state that the task should not generate info to the LOG , because it is doing 'polling' */
    appDemoForbidCpssLogOnSelfSet(1);
    /* state that the task should not generate info to the 'Register trace' , because it is doing 'polling' */
    appDemoForbidCpssRegisterTraceOnSelfSet(1);

    /* indicate that task start running */
    fanManagementTaskCreated = 1;

    /* indecate that task is not flaged as killed */
    fanManagementTaskKill = GT_FALSE;

    /* set initial speed */
    appDemoFalcon_DB_FanSpeedSet(FAN_SPEED_50_E);

    /* main task loop -
       In each iteration reads the temperature from all the sensors,
       for each sensor it calculates the speed,
       set the maximum speed out of all the calculated speeds */
    while (1)
    {
        /* check if task need termination */
        appDemoTaskCheckIfNeedTermination();

        /* check if user stopped the task */
        if(!cpss_db_falcon_thermal_en)
        {
           osTimerWkAfter(1000);
           continue;
        }

        /* max fan speed does not depend on the previous speed */
        currentMaxFanSpeed = 0;
        /* get maximun fan speed from all the sensors */
        for (ii=0;ii<sizeof(pidList)/sizeof(pidList[0]);ii++)
        {
            tempFanSpeed = falcon_DB_PidFanSpeedGet(devNum, &pidList[ii]);
            if ( tempFanSpeed > currentMaxFanSpeed )
            {
                currentMaxFanSpeed = tempFanSpeed;
            }

            /* check if task is killed due error in the task*/
            if (fanManagementTaskKill)
            {
                return 0;
            }
        }

        prevFanSpeed = appDemoFalcon_DB_FanSpeedDump();

        /* set new fan speed only if the new speed is different than the last - to save writings through SMI */
        if ( currentMaxFanSpeed != prevFanSpeed )
        {
            rc = appDemoFalcon_DB_FanSpeedSet(currentMaxFanSpeed);
            if(rc != GT_OK)
            {
                cpssOsPrintf("Error: Fan Management Task Failed to change speed\nKilling Fan Managment Task");
                return 0;
            }
            prevFanSpeed = currentMaxFanSpeed;
        }

        if(cpss_thermal_debug)
        {
            cpssOsPrintf("currentMaxFanSpeed =%d\n", currentMaxFanSpeed);
            cpssOsPrintf("prevFanSpeed =%d\n", prevFanSpeed);
        }

        osTimerWkAfter(10000); /* check temperature every 10 sec */
    }

    return 0;
}

/**
 * @internal falcon_DB_CreateThermalTask function
 *
 * @return GT_STATUS
 *
 */
static GT_STATUS falcon_DB_CreateThermalTask
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc;

    /* check if task is already running */
    if(fanManagementTaskCreated == 1)
    {
        return GT_OK;
    }

    /* cpssOsTaskCreate = appDemoWrap_osTaskCreate , to allow disabling LOG info
       of the polling every 2 seconds */
    rc = cpssOsTaskCreate("cpss_db_falcon_thermal_task",
                      FAN_SPEED_MNG_TASK_PRIO_CNS,
                     _8KB,
                     falcon_DB_ThermalTask,
                     (GT_VOID*)((GT_UINTPTR)(devNum)),
                     &cpss_db_falcon_thermal_task_id );
    if(rc != GT_OK)
    {
        cpssOsPrintf("spawn cpss_db_falcon_thermal_task fail \n");
    }
    return rc;
}

/**
* @internal appDemoFalcon_DB_FanSpeedSet function
* @endinternal
*
* @brief Set falcon DB 12.8 fan speed
*
* @param GT_U32 value - Applicable values: 0-7
*
* @note For debug purposes only. To keep fan speed as set, make sure to call
*       falcon_DB_EnableThermalTask with GT_FALSE.
*
*/
GT_STATUS appDemoFalcon_DB_FanSpeedSet
(
    IN GT_U32 value
)
{
    GT_STATUS rc = GT_OK;

    /* check smi driver before writing */
    if(Cpu_Smi_drv == NULL)
    {
        cpssOsPrintf("ERROR: SMI driver not found! can't set fan speed\n");
        return GT_OK;
    }

    /*set fan's speed*/
    rc = Cpu_Smi_drv->writeMask(Cpu_Smi_drv, fanFPGASmiAddr, fanRegAddrInFPGA, &value, 1 /*count*/, 0xf);
    if (rc != GT_OK)
    {
        cpssOsPrintf("extDrvDirectSmiWriteReg failed");
        return rc;
    }
    return GT_OK;
}

/**
* @internal appDemoFalcon_DB_FanSpeedDump function
* @endinternal
*
* @brief get falcon DB 12.8 fan speed
*
* @note For debug purposes only.
*/
GT_U32 appDemoFalcon_DB_FanSpeedDump
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 data;

    /*set fan's speed*/
    rc = Cpu_Smi_drv->read(Cpu_Smi_drv, fanFPGASmiAddr, fanRegAddrInFPGA, &data, 1 /*count*/);
    if (rc != GT_OK)
    {
        cpssOsPrintf("extDrvDirectSmiReadReg failed");
        return 0;
    }
    data = data & 0x7;
    return data;
}

/**
* @internal appDemoFalcon_DB_openCpuTempratureFiles function
* @endinternal
*
* @brief safely allocates cpu temprature files pointers and open the files.
*
*/
static GT_STATUS appDemoFalcon_DB_openCpuTempratureFiles
(
    GT_VOID
)
{
    /* CPU temperature variables initialization - thermal_zone0. */
    thermalZone0Fp = cpssOsMalloc (sizeof(CPSS_OS_FILE_TYPE_STC));
    if (thermalZone0Fp == NULL)
    {
        return GT_FAIL;
    }

    thermalZone0Fp->type = CPSS_OS_FILE_REGULAR;
    thermalZone0Fp->fd = cpssOsFopen("./sys/class/thermal/thermal_zone0/temp", "r",thermalZone0Fp);
    if (0 == thermalZone0Fp->fd)
    {
        cpssOsPrintf("Failed to read CPU temperature\n");
        /* free memory*/
        cpssOsFree(thermalZone0Fp);
        return GT_FAIL;
    }

    /* CPU temperature variables initialization - thermal_zone1. */
    thermalZone1Fp = cpssOsMalloc (sizeof(CPSS_OS_FILE_TYPE_STC));
    if (thermalZone1Fp == NULL)
    {
        cpssOsFclose(thermalZone0Fp);
        cpssOsFree(thermalZone0Fp);
        return GT_FAIL;
    }

    thermalZone1Fp->type = CPSS_OS_FILE_REGULAR;
    thermalZone1Fp->fd = cpssOsFopen("./sys/class/thermal/thermal_zone1/temp", "r",thermalZone1Fp);
    if (0 == thermalZone1Fp->fd)
    {
        cpssOsPrintf("Failed to read CPU temperature\n");
        /* close Thermal_zone0 file pointer and free memory*/
        cpssOsFclose(thermalZone0Fp);
        cpssOsFree(thermalZone0Fp);
        cpssOsFree(thermalZone1Fp);
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal falconDB_ThermalTaskInit function
* @endinternal
*
* @brief This function initialize Falcon 12.8 DB Fan Managment task
*
*/
GT_STATUS falconDB_ThermalTaskInit
(
    IN GT_U8   devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 data;
    GT_U8 buffer;

    /*initializing Driver*/
    if (cpssHwDriverLookup("/SoC") == NULL)
    {
        SOC_INIT_A7K();
    }

    if (cpssHwDriverLookup("/SoC/internal-regs/smi@0x12a200") != NULL)
    {
        CPSS_HW_DRIVER_STC *soc = cpssHwDriverLookup("/SoC/internal-regs");
        if (soc != NULL)
        {
            /* 0 - normal speed(/128), 1 - fast mode(/16), 2 - accel (/8) */

            /*
                PLEASE PAY ATTENTION:
                    bit_10 is disabled - "Enable Polling" disabled in order to solve congestion problem on SMI bus
            */

            /* current speed - fast(5.2Mhz)*/
            data = 0xA;
            soc->writeMask(soc, 0, 0x12a204, &data, 1, 0x40F);
        }
    }
    Cpu_Smi_drv = cpssHwDriverLookup("/smi0");
    if(Cpu_Smi_drv == NULL)
    {
        cpssOsPrintf("ERROR: SMI driver not found! will not start fan control thermal task\n\n");
        return GT_OK;
    }

    /* read default values and compare */
    rc = Cpu_Smi_drv->read(Cpu_Smi_drv, fanFPGASmiAddr, 0x0, &data, 1 /*count*/);
    if ((rc != GT_OK) || (data != 0x7))
    {
        cpssOsPrintf("read from SMI failed at reg 0x0, value stored = %d \n", data);
        return GT_OK;
    }

    /* check if JP10/JP11 jumper is connected */
    rc = appDemoHostI2cRead(0/*bus id*/,0x77/*slave address*/,0/*offset_type: 8bit*/, 0 /*offset */,4/*size - number of Bytes*/, &buffer);
    if(rc != GT_OK)
    {
        cpssOsPrintf("ERROR: Board's Thermal sensor is not reachable! will not start fan control thermal task\n\n");
        return GT_OK;
    }

    rc = falcon_DB_initPortPerRavenArray(devNum);
    if (rc != GT_OK)
    {
        return GT_OK;
    }

    rc = appDemoFalcon_DB_openCpuTempratureFiles();
    if (rc != GT_OK)
    {
        return GT_OK;
    }

    /* create thermal task */
    rc = falcon_DB_CreateThermalTask(devNum);
    if (rc != GT_OK)
    {
        cpssOsFclose(thermalZone0Fp);
        cpssOsFclose(thermalZone1Fp);
        cpssOsFree(thermalZone0Fp);
        cpssOsFree(thermalZone1Fp);
    }
    return rc;
}
#endif /*ASIC_SIMULATION*/
