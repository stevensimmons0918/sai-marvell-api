/* adapter_init_warmboot.c
 *
 * WarmBoot related functionality related to devices.
 */

/*****************************************************************************
* Copyright (c) 2016-2019 INSIDE Secure B.V. All Rights Reserved.
*
* This confidential and proprietary software may be used only as authorized
* by a licensing agreement from INSIDE Secure.
*
* The entire notice above must be reproduced on all authorized copies that
* may only be made to the extent permitted by a licensing agreement from
* INSIDE Secure.
*
* For more information or support, please go to our online support system at
* https://customersupport.insidesecure.com.
* In case you do not have an account for this system, please send an e-mail
* to ESSEmbeddedHW-Support@insidesecure.com.
*****************************************************************************/

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

/* Init WarmBoot API */
#include <Integration/Adapter_EIP164/incl/adapter_init_warmboot.h>

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include <Integration/Adapter_EIP164/incl/c_adapter_eip164.h>     /* Configuration */

#include <Kit/DriverFramework/incl/clib.h>                        /* C Run-time Library API */

#include <Kit/DriverFramework/incl/device_mgmt.h>                 /* Device Management API. */

#include <Integration/Adapter_EIP164/incl/adapter_warmboot_support.h> /* WamrBoot support API. */

#include <Integration/Adapter_EIP164/incl/adapter_init_support.h> /* Adapter init support API. */

#include <Kit/Log/incl/log.h>                                     /* Log API */

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <Integration/Adapter_EIP164/incl/adapter_alloc.h>        /* Adapter_Alloc/Adapter_Free */

/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#ifdef ADAPTER_WARMBOOT_ENABLE

/* Each WarmBoot area contains a single flag byte immediately followed */
/* by a record depending on the type: */
/* Flag byte: BIT_7 and BIT_6 both set to indicate Driver Framework area type. */
/*            BIT_0 cleared for single device, set for data path. */

/* Data structure that represents a device in a WarmBoot area. */
typedef struct
{
    char Name[ADAPTER_MAX_DEVICE_NAME_LENGTH+1];
    unsigned char Flags;
    unsigned short Index;
    unsigned int StartByteOffset;
    unsigned int LastByteOffset;
} WarmBoot_DeviceRec_t;


/* Data structure that represents a data path in a WarmBoot area. */
typedef struct
{
    unsigned int StartByteOffset1;
    unsigned int LastByteOffset1;
    unsigned int StartByteOffset2;
    unsigned int LastByteOffset2;
    unsigned short Index;
    unsigned char Flags1;
    unsigned char Flags2;
} WarmBoot_DataPathRec_t;


/* Data structure to represent a WarmBoot device in memory, */
/* indexed by Device Index. */
typedef struct
{
    unsigned int AreaId; /* WarmBoot area where device info is stored. */
    unsigned int DataPathIndex; /* Index of data path to which device belongs. */
    bool fWarmBoot; /* Device has a valid WarmBoot Area. */
    bool fDataPath; /* Device is part of a data path chain. */
} WarmBoot_DevDescr_t;

/* Data structure to represent a data path in memory. */
typedef struct
{
    unsigned int AreaId;  /* WarmBoot area where data path is stored. */
    bool fWarmBoot;       /* Data path has a valid WarmBoot area. */
    unsigned int StartIndex; /* First device index. */
    unsigned int LastIndex;  /* Last device index. */
} WarmBoot_DataPathDescr_t;

#define INIT_WARMBOOT_ERROR 1

/*----------------------------------------------------------------------------
 * Local variables
 */

/* Information related to single devices. */
static WarmBoot_DataPathDescr_t
      WarmBoot_DataPaths[ADAPTER_EIP164_MAX_NOF_DEVICES];

/* Information related to complete data paths. */
static WarmBoot_DevDescr_t *WarmBoot_Devices;


/*----------------------------------------------------------------------------
 * Adapter_InitLib_WarmBoot_Areas_Init
 *
 * Initialize the internal data structures of this module.
 */
static bool
Adapter_InitLib_WarmBoot_Areas_Init(void)
{
    unsigned int Count = Device_GetCount();
    unsigned int i, j;
    if (WarmBoot_Devices != NULL)
    {
        LOG_CRIT("%s: already allocated\n",__func__);
        return false;
    }

    WarmBoot_Devices = Adapter_Alloc(Count * sizeof(WarmBoot_DevDescr_t));
    if (WarmBoot_Devices == NULL)
    {
        LOG_CRIT("%s: failed to allocate device array\n",__func__);
        return false;
    }

    cpssOsMemSet(WarmBoot_Devices, 0, Count * sizeof(WarmBoot_DevDescr_t));
    ZEROINIT(WarmBoot_DataPaths);

    for (i=0; i<ADAPTER_EIP164_MAX_NOF_DEVICES; i++)
    {
        if (Adapter_Init_DataPath_GetIndexRange(
                i,
                &WarmBoot_DataPaths[i].StartIndex,
                &WarmBoot_DataPaths[i].LastIndex) < 0)
        {
            LOG_CRIT("%s: failed to obtain datapath index\n",__func__);
            return false;
        }
        for (j = WarmBoot_DataPaths[i].StartIndex;
             j < WarmBoot_DataPaths[i].LastIndex;
             j++)
        {
            if (j < Count)
            {
                WarmBoot_Devices[j].fDataPath = true;
                WarmBoot_Devices[j].DataPathIndex = i;
            }
        }
    }
    return true;
}

/*----------------------------------------------------------------------------
 * Adapter_Init_WarmBoot_MaxSize_Get
 */
bool
Adapter_Init_WarmBoot_MaxSize_Get(
        unsigned int * const MaxByteCount_p)
{
    *MaxByteCount_p = 1 +
        MAX(sizeof(WarmBoot_DeviceRec_t), sizeof(WarmBoot_DataPathRec_t));
    return true;
}


/*----------------------------------------------------------------------------
 * Adapter_Init_WarmBoot_Device_Update
 */
bool
Adapter_Init_WarmBoot_Device_Update(
        unsigned int Index,
        const Device_Properties_t *Props_p)
{

    if (WarmBoot_Devices == NULL)
    {
        LOG_CRIT("%s: Warmboot subsystem not initialized\n",__func__);
        return false;
    }

    if (Props_p == NULL)
    {
        LOG_INFO("%s: removing device record %d\n",__func__,Index);

        if (WarmBoot_Devices[Index].fWarmBoot)
        {
            LOG_INFO("%s: freeing up device WB area\n",__func__);
            WarmBoot_Devices[Index].fWarmBoot = false;
            if (!WarmBootLib_Free(WarmBoot_Devices[Index].AreaId))
            {
                LOG_CRIT("WarmBoot_Free failed\n");
                return false;
            }
        }
        else if (WarmBoot_Devices[Index].fDataPath)
        {
            /* Device is marked as part of a data path, remove */
            /* the data path WB record if any. */
            unsigned int dpIndex = WarmBoot_Devices[Index].DataPathIndex;

            if (WarmBoot_DataPaths[dpIndex].fWarmBoot)
            {
                LOG_INFO("%s: Remove WB record for data path %d",
                         __func__, dpIndex);
                WarmBoot_DataPaths[dpIndex].fWarmBoot = false;
                if (!WarmBootLib_Free(WarmBoot_DataPaths[dpIndex].AreaId))
                {
                    LOG_CRIT("WarmBoot_Free failed\n");
                    return false;
                }
            }
        }
    }
    else
    {
        WarmBoot_DeviceRec_t DeviceRec;
        unsigned char FlagByte = BIT_7|BIT_6;
        LOG_INFO("%s: adding device record %d for device %s\n",
                 __func__,Index,Props_p->Name_p);

        if (strlen(Props_p->Name_p) > ADAPTER_MAX_DEVICE_NAME_LENGTH)
        {
            LOG_CRIT("%s: Name too long: %s\n",__func__,Props_p->Name_p);
            return false;
        }
        strcpy(DeviceRec.Name, Props_p->Name_p);
        DeviceRec.StartByteOffset = Props_p->StartByteOffset;
        DeviceRec.LastByteOffset = Props_p->LastByteOffset;
        DeviceRec.Flags = Props_p->Flags;
        DeviceRec.Index = Index;

        if(!WarmBoot_Devices[Index].fWarmBoot)
        {
            if(!WarmBootLib_Alloc(1 + sizeof(WarmBoot_DeviceRec_t),
                                  &WarmBoot_Devices[Index].AreaId))
            {
                LOG_CRIT("%s: alloc failed\n",__func__);
                return false;
            }
            WarmBoot_Devices[Index].fWarmBoot = true;
        }
        if(!WarmBootLib_Write(WarmBoot_Devices[Index].AreaId,
                              &FlagByte,
                              0,
                              1))
        {
            LOG_CRIT("%s: write failed\n",__func__);
            return false;
        }

        if(!WarmBootLib_Write(WarmBoot_Devices[Index].AreaId,
                              (unsigned char *)&DeviceRec,
                              1,
                              sizeof(WarmBoot_DeviceRec_t)))
        {
            LOG_CRIT("%s: write failed\n",__func__);
            return false;
        }
    }
    return true;
}


/*----------------------------------------------------------------------------
 * Adapter_Init_WarmBoot_DataPath_Update
 */
bool
Adapter_Init_WarmBoot_DataPath_Update(
        unsigned int Index,
        const Driver164_DataPath_t * Props_p)
{
    unsigned int Count = Device_GetCount();
    unsigned int i;
    unsigned char FlagByte = BIT_7|BIT_6|BIT_0;
    WarmBoot_DataPathRec_t DataPathRec;

    if (WarmBoot_Devices == NULL)
    {
        LOG_CRIT("%s: Warmboot subsystem not initialized\n",__func__);
        return false;
    }

    /* Remove the WB storage areas of any of the constituent single devices. */
    for (i = WarmBoot_DataPaths[Index].StartIndex;
         i < WarmBoot_DataPaths[Index].LastIndex;
         i++)
    {
        if (i < Count)
        {
            if (WarmBoot_Devices[i].fWarmBoot)
            {
                WarmBootLib_Free(WarmBoot_Devices[i].AreaId);
                WarmBoot_Devices[i].fWarmBoot = false;
            }
         }
    }

    if (Props_p == NULL)
    {
        LOG_INFO("%s: removing data path record %d\n",__func__,Index);
        if (WarmBoot_DataPaths[Index].fWarmBoot)
        {
            WarmBoot_DataPaths[Index].fWarmBoot = false;
            if (!WarmBootLib_Free(WarmBoot_DataPaths[Index].AreaId))
            {
                LOG_CRIT("WarmBoot_Free failed\n");
                return false;
            }
        }
    }
    else
    {
        LOG_INFO("%s: adding data path record %d\n",__func__,Index);
        if (!WarmBoot_DataPaths[Index].fWarmBoot)
        {
            if(!WarmBootLib_Alloc(
                   1 + sizeof(WarmBoot_DataPathRec_t),
                   &WarmBoot_DataPaths[Index].AreaId))
            {
                LOG_CRIT("%s: alloc failed\n",__func__);
                return false;
            }
            WarmBoot_DataPaths[Index].fWarmBoot = true;
        }
        DataPathRec.Index = Index;
        DataPathRec.StartByteOffset1 = Props_p->StartByteOffset1;
        DataPathRec.LastByteOffset1  = Props_p->LastByteOffset1;
        DataPathRec.Flags1           = Props_p->Flags1;
        DataPathRec.StartByteOffset2 = Props_p->StartByteOffset2;
        DataPathRec.LastByteOffset2  = Props_p->LastByteOffset2;
        DataPathRec.Flags2           = Props_p->Flags2;

        if(!WarmBootLib_Write(WarmBoot_DataPaths[Index].AreaId,
                              (const unsigned char *)&FlagByte,
                              0,
                              1))
        {
            LOG_CRIT("%s: write failed\n",__func__);
        }

        if(!WarmBootLib_Write(WarmBoot_DataPaths[Index].AreaId,
                              (const unsigned char *)&DataPathRec,
                              1,
                              sizeof(WarmBoot_DataPathRec_t)))
        {
            LOG_CRIT("%s: write failed\n",__func__);
        }
    }
    return true;
}

/*----------------------------------------------------------------------------
 * Adapter_Init_WarmBoot_Init
 */
bool
Adapter_Init_WarmBoot_Init(void)
{
    unsigned int Count = Device_GetCount();
    Device_Properties_t Props;
    Driver164_DataPath_t DataPath;
    bool fValid;
    int Device_Rc;
    unsigned int i;

    if(!Adapter_InitLib_WarmBoot_Areas_Init())
    {
        return false;
    }

    /* Collect the status of all data paths. */
    for (i=0; i<ADAPTER_EIP164_MAX_NOF_DEVICES; i++)
    {
        if (Adapter_Init_DataPath_GetProperties(i, &DataPath, &fValid) < 0)
        {
            Adapter_Init_WarmBoot_Uninit();
            return false;
        }

        if (fValid)
        {
            if (! Adapter_Init_WarmBoot_DataPath_Update(i, &DataPath))
            {
                Adapter_Init_WarmBoot_Uninit();
                return false;
            }
        }
    }

    /* Collect the status of all non-datapath devices. */
    for (i = 0; i < Count; i++)
    {
        if (!WarmBoot_Devices[i].fDataPath)
        {
            Device_Rc = Device_GetProperties(i, &Props, &fValid);
            if (Device_Rc < 0)
            {
                LOG_CRIT("%s: Device_GetProperties failed\n",__func__);
                Adapter_Init_WarmBoot_Uninit();
                return false;
            }
            if (fValid)
            {
                if (! Adapter_Init_WarmBoot_Device_Update(i, &Props))
                {
                    Adapter_Init_WarmBoot_Uninit();
                    return false;
                }
            }
            else
            {
                LOG_INFO("%s: Device_GetProperties no device at %d\n",__func__,i);
            }
        }
    }

    return true;
}


/*----------------------------------------------------------------------------
 * Adapter_Init_WarmBoot_Uninit
 */
bool
Adapter_Init_WarmBoot_Uninit(void)
{
    unsigned int i;
    unsigned int Count = Device_GetCount();
    if (WarmBoot_Devices == NULL)
    {
        LOG_CRIT("%s: Warmboot subsystem not initialized\n",__func__);
        return false;
    }
    for (i = 0; i < Count; i++)
    {
        if (WarmBoot_Devices[i].fWarmBoot)
        {
            WarmBootLib_Free(WarmBoot_Devices[i].AreaId);
        }
    }
    for (i = 0; i < ADAPTER_EIP164_MAX_NOF_DEVICES; i++)
    {
        if (WarmBoot_DataPaths[i].fWarmBoot)
        {
            WarmBootLib_Free(WarmBoot_DataPaths[i].AreaId);
        }
    }
    Adapter_Free(WarmBoot_Devices);
    WarmBoot_Devices = NULL;
    return true;
}


/*----------------------------------------------------------------------------
 * Adapter_Init_WarmBoot_Shutdown
 */
int
Adapter_Init_WarmBoot_Shutdown(
        unsigned int AreaId)
{
    IDENTIFIER_NOT_USED(AreaId);

    LOG_INFO("%s: started\n",__func__);

    if (WarmBoot_Devices != NULL)
    {
        Adapter_Free(WarmBoot_Devices);
        WarmBoot_Devices = NULL;
    }

    return 0;
}


/*----------------------------------------------------------------------------
 * Adapter_Init_WarmBoot_Restore
 */
int
Adapter_Init_WarmBoot_Restore(
        unsigned int AreaId)
{
    WarmBoot_DeviceRec_t DeviceRec;
    Device_Properties_t Props;
    Driver164_DataPath_t DataPathProps;
    WarmBoot_DataPathRec_t DataPathRec;
    bool fValid;
    unsigned char FlagByte;
    LOG_INFO("%s: started\n",__func__);

    /* The first call to a restore function re-initializes the data structures. */
    if (WarmBoot_Devices == NULL)
    {
        if(!Adapter_InitLib_WarmBoot_Areas_Init())
        {
            return INIT_WARMBOOT_ERROR;
        }
    }

    if(!WarmBootLib_Read(AreaId,
                         (unsigned char*)&FlagByte,
                         0,
                         1))
    {
        LOG_CRIT("%s: error reading WB area \n",__func__);
        return INIT_WARMBOOT_ERROR;
    }
    if ( (FlagByte & BIT_0) != 0 )
    {
        /* This is a data path. */
        if(!WarmBootLib_Read(AreaId,
                             (unsigned char*)&DataPathRec,
                             1,
                             sizeof(WarmBoot_DataPathRec_t)))
        {
            LOG_CRIT("%s: error reading WB area \n",__func__);
            return INIT_WARMBOOT_ERROR;
        }
        WarmBoot_DataPaths[DataPathRec.Index].fWarmBoot = true;
        WarmBoot_DataPaths[DataPathRec.Index].AreaId = AreaId;
        DataPathProps.StartByteOffset1 = DataPathRec.StartByteOffset1;
        DataPathProps.LastByteOffset1  = DataPathRec.LastByteOffset1;
        DataPathProps.Flags1           = DataPathRec.Flags1;
        DataPathProps.StartByteOffset2 = DataPathRec.StartByteOffset2;
        DataPathProps.LastByteOffset2  = DataPathRec.LastByteOffset2;
        DataPathProps.Flags2           = DataPathRec.Flags2;
        LOG_INFO("%s restoring data path index %d\n",
                 __func__,
                 DataPathRec.Index);

        Adapter_Init_DataPath_Remove(DataPathRec.Index);
        if (Adapter_Init_DataPath_Add(DataPathRec.Index, &DataPathProps) < 0)
        {
            LOG_CRIT("%s: error adding data path \n",__func__);
            return INIT_WARMBOOT_ERROR;
        }
    }
    else
    {
        /* This is an entry for a single device. */
        if(!WarmBootLib_Read(AreaId,
                             (unsigned char*)&DeviceRec,
                             1,
                             sizeof(WarmBoot_DeviceRec_t)))
        {
            LOG_CRIT("%s: error reading WB area \n",__func__);
            return INIT_WARMBOOT_ERROR;
        }
        LOG_INFO("%s: Found device with name %s at index  %d\n",__func__,
                 DeviceRec.Name,DeviceRec.Index);

        WarmBoot_Devices[DeviceRec.Index].fWarmBoot = true;
        WarmBoot_Devices[DeviceRec.Index].AreaId = AreaId;
        if (Device_GetProperties(DeviceRec.Index, &Props, &fValid) < 0)
        {
            LOG_CRIT("%s: error reading device props at index %d\n",
                     __func__,DeviceRec.Index);
            return INIT_WARMBOOT_ERROR;
        }
        if (fValid)
        {
            Device_Remove(DeviceRec.Index);
        }
        Props.Name_p = DeviceRec.Name;
        Props.StartByteOffset = DeviceRec.StartByteOffset;
        Props.LastByteOffset = DeviceRec.LastByteOffset;
        Props.Flags = DeviceRec.Flags;
        if (Device_Add(DeviceRec.Index, &Props) < 0)
        {
            LOG_CRIT("%s: error adding device at index %d\n",
                     __func__,
                     DeviceRec.Index);
            return INIT_WARMBOOT_ERROR;
        }
    }
    return 0;
}


#endif /* ADAPTER_WARMBOOT_ENABLE */

/* end of file adapter_init_warmboot.c */
