/* adapter_init_support.c
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
#include <Integration/Adapter_EIP164/incl/adapter_init_support.h>

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include <Integration/Adapter_EIP164/incl/c_adapter_eip164.h> /* Configuration */

#include <Kit/DriverFramework/incl/clib.h> /*  C Run-time Library API */

#include <Kit/DriverFramework/incl/device_mgmt.h> /* Device Management API. */

#include <Kit/Log/incl/log.h>         /* Log API */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
typedef struct
{
    const char *Name_p;
    unsigned int StartByteOffset;
    unsigned int LastByteOffset;
    unsigned int Flags;
} Adapter_ChainProperties_t;

#define ADAPTER_INIT_INVALID_INDEX 0xffffffff

/*----------------------------------------------------------------------------
 * Local variables
 */

/* Template for the data path chain. */
/* The device names will have a % character so they can be expanded into */
/* the real device names. */
/* The 'Flags' field in this table is used to distinguish between */
/* the two chains (flags==0 for chain 1, flags==1 for chain 2). */
/* */
/* The actual flags values with which the devices are added are provided */
/* by the caller of the Adapter_Init_DataPath_Add function. */
#define ADAPTER_EIP164_DEVICE_ADD(_name,_devnr,_start,_end,_flags)  \
    {_name, _start, _end, _flags}

static const Adapter_ChainProperties_t  Adapter_EIP164_DataPath[] =
{
    ADAPTER_EIP164_DATAPATH(%,0,0,0,0,0,0,0,1)
};

#define ADAPTER_DATAPATH_DEVICE_COUNT \
    (sizeof(Adapter_EIP164_DataPath)/sizeof(Adapter_EIP164_DataPath[0]))

/* Index within the data path list of the EIP164 and EIP163 devices. */
static unsigned int Adapter_EIP164_Index = ADAPTER_INIT_INVALID_INDEX;
static unsigned int Adapter_EIP163_Index = ADAPTER_INIT_INVALID_INDEX;


/*----------------------------------------------------------------------------
 * Adapter_InitLib_Entry_Find
 *
 * This function finds the index of a given (template) name within the
 * Adapter_EIP164_DataPath array.
 *
 * Returns the index at which the name is found, ADAPTER_INIT_INVALID_INDEX
 * if the name is not found.
 */
static unsigned int
Adapter_InitLib_Entry_Find(
        const char * Name)
{
    unsigned int i;
    for (i=0; i<ADAPTER_DATAPATH_DEVICE_COUNT; i++)
    {
        if (cpssOsStrCmp(Adapter_EIP164_DataPath[i].Name_p, Name) == 0)
        {
            return i;
        }
    }
    return ADAPTER_INIT_INVALID_INDEX;
}

/*----------------------------------------------------------------------------
 * Adapter_InitLib_Name_Expand
 *
 * This function expands a template device name (with a single %) into
 * the actual device name belonging to the device with the given index.
 *
 * Template (input)
 *     Null terminated string that represents a template device name.
 *
 * Index (input)
 *     The integer index that must be substituted into the device name.
 *
 * ExpandedName (output)
 *     The expanded device name. This must point to a buffer of size
 *     ADAPTER_MAX_DEVICE_NAME_LENGTH + 1.
 *
 * Returns 0 for success and -1 for failure.
 */
static int
Adapter_InitLib_Name_Expand(
        const char *Template,
        const unsigned int Index,
        char * ExpandedName)
{
    unsigned i,j;

    j=0;
    for (i=0; Template[i]!=0; i++)
    {
        if (j>=ADAPTER_MAX_DEVICE_NAME_LENGTH)
            return -1;
        if (Template[i]=='%')
        {
            /* Expand the decimal number (up to 3 digits) in place of % */
            if (Index >= 100)
            {
                ExpandedName[j++] = '0' + Index / 100;
                if (j>=ADAPTER_MAX_DEVICE_NAME_LENGTH)
                    return -1;
            }
            if (Index >= 10)
            {
                ExpandedName[j++] = '0' + (Index % 100)  / 10;
                if (j>=ADAPTER_MAX_DEVICE_NAME_LENGTH)
                    return -1;
            }
            ExpandedName[j++] = '0' + Index % 10;
        }
        else
        {
            ExpandedName[j++] = Template[i];
        }
    }
    ExpandedName[j]=0;
    return 0;
}

/*----------------------------------------------------------------------------
 * Adapter_InitLib_Index_Initialize
 *
 * Initialize the Adapter_EIP164_Index and Adapter_EIP163_Index
 * variables. Do not look up the names again if they are already found.
 *
 * Returns 0 for success and -1 for failure.
 */
static int
Adapter_InitLib_Index_Initialize(void)
{
    if (Adapter_EIP164_Index == ADAPTER_INIT_INVALID_INDEX)
    {
        Adapter_EIP164_Index =
            Adapter_InitLib_Entry_Find(ADAPTER_EIP164_NAME(%));
        if (Adapter_EIP164_Index == ADAPTER_INIT_INVALID_INDEX)
            return -1;
        /* Adapter_EIP163_Index is allowed to be invalid if the device */
        /* data path does contain an EIP163. */
        Adapter_EIP163_Index =
            Adapter_InitLib_Entry_Find(ADAPTER_EIP163_NAME(%));
    }
    return 0;
}


/*----------------------------------------------------------------------------
 * Adapter_Init_DataPath_Add
 */
int
Adapter_Init_DataPath_Add(
        const unsigned int Index,
        const Driver164_DataPath_t * const DataPath_p)
{
    Device_Properties_t Props;
    char Name[ADAPTER_MAX_DEVICE_NAME_LENGTH+1];
    unsigned int i;
    unsigned int StartIndex;

    if (Adapter_InitLib_Index_Initialize() < 0)
        return -1;

    StartIndex = ADAPTER_EIP164_DEV_ID(Index) - Adapter_EIP164_Index;

    LOG_INFO("%s: Adding devices from index %u to %u"
             " (EIP164 at %u, EIP163 at %u)\n",
             __func__,
             StartIndex,
             StartIndex + (unsigned int)ADAPTER_DATAPATH_DEVICE_COUNT,
             Adapter_EIP164_Index + StartIndex,
             Adapter_EIP163_Index + StartIndex);

    for (i = 0; i < ADAPTER_DATAPATH_DEVICE_COUNT; i++)
    {
        if (Adapter_InitLib_Name_Expand(
                Adapter_EIP164_DataPath[i].Name_p,
                Index,
                Name) < 0)
        {
            return -1;
        }
        LOG_INFO("Expanded Name = %s\n",Name);
        Props.Name_p = Name;

        if (Adapter_EIP164_DataPath[i].Flags == 0)
        {
            /* Device is on chain 1. */
            Props.StartByteOffset = DataPath_p->StartByteOffset1 +
                Adapter_EIP164_DataPath[i].StartByteOffset;
            if (i == Adapter_EIP164_Index || i == Adapter_EIP163_Index)
            {
                /* It's the main device on the chain (EIP164 or EIP163) */
                Props.LastByteOffset = DataPath_p->LastByteOffset1;
            }
            else
            {
                Props.LastByteOffset = DataPath_p->StartByteOffset1 +
                    Adapter_EIP164_DataPath[i].LastByteOffset;
            }
            Props.Flags = DataPath_p->Flags1;
        }
        else
        {
            /* Device is on chain 2. */
            Props.StartByteOffset = DataPath_p->StartByteOffset2 +
                Adapter_EIP164_DataPath[i].StartByteOffset;
            if (i == Adapter_EIP164_Index || i == Adapter_EIP163_Index)
            {
                /* It's the main device on the chain (EIP164 or EIP163) */
                Props.LastByteOffset = DataPath_p->LastByteOffset2;
            }
            else
            {
                Props.LastByteOffset = DataPath_p->StartByteOffset2 +
                    Adapter_EIP164_DataPath[i].LastByteOffset;
            }
            Props.Flags = DataPath_p->Flags1;
        }
        if (Device_Add(StartIndex + i, &Props) < 0)
        {
            return -1;
        }
    }

    return 0;
}


/*----------------------------------------------------------------------------
 * Adapter_Init_DataPath_Remove
 */
int
Adapter_Init_DataPath_Remove(
        const unsigned int Index)
{
    int Rc = 0;
    unsigned int i;
    unsigned int StartIndex;

    if(Adapter_InitLib_Index_Initialize() < 0)
        return -1;

    StartIndex = ADAPTER_EIP164_DEV_ID(Index) - Adapter_EIP164_Index;

    LOG_INFO("%s: Removing devices from index %u to %u\n",
             __func__,
             StartIndex,
             StartIndex + (unsigned int)ADAPTER_DATAPATH_DEVICE_COUNT);
    for (i = StartIndex; i < StartIndex + ADAPTER_DATAPATH_DEVICE_COUNT; i++)
    {
        if (Device_Remove(i) < 0)
        {
            Rc = -1; /* Report a failed call as a failure, but proceed anyway. */
        }
    }

    return Rc;
}


/*----------------------------------------------------------------------------
 * Adapter_Init_DataPath_GetProperties
 */
int
Adapter_Init_DataPath_GetProperties(
        unsigned int Index,
        Driver164_DataPath_t * const DataPath_p,
        bool * const fValid_p)
{
    unsigned int StartIndex;
    bool fValid;
    Device_Properties_t Props;

    if ( Adapter_InitLib_Index_Initialize() < 0)
        return -1;

    cpssOsMemSet(DataPath_p, 0, sizeof(Driver164_DataPath_t));

    StartIndex = ADAPTER_EIP164_DEV_ID(Index) - Adapter_EIP164_Index;

    if (Device_GetProperties(StartIndex + Adapter_EIP164_Index,
                             &Props,
                             &fValid) < 0)
    {
        return -1;
    }
    *fValid_p = fValid;
    if (fValid)
    {
        if (Adapter_EIP164_DataPath[Adapter_EIP164_Index].Flags == 0)
        {   /* Device = on chain 1. */
            DataPath_p->StartByteOffset1 = Props.StartByteOffset -
                Adapter_EIP164_DataPath[Adapter_EIP164_Index].StartByteOffset;
            DataPath_p->LastByteOffset1 = Props.LastByteOffset;
            DataPath_p->Flags1 = Props.Flags;
        }
        else
        {   /* Device = on chain 2. */
            DataPath_p->StartByteOffset2 = Props.StartByteOffset -
                Adapter_EIP164_DataPath[Adapter_EIP164_Index].StartByteOffset;
            DataPath_p->LastByteOffset2 = Props.LastByteOffset;
            DataPath_p->Flags2 = Props.Flags;
        }

        if (Adapter_EIP163_Index != ADAPTER_INIT_INVALID_INDEX)
        {
            if (Device_GetProperties(StartIndex + Adapter_EIP163_Index,
                                     &Props,
                                     &fValid) < 0)
            {
                return -1;
            }
            if (!fValid)
            {
                return -1;
            }
            if (Adapter_EIP164_DataPath[Adapter_EIP163_Index].Flags == 0)
            {   /* Device = on chain 1. */
                DataPath_p->StartByteOffset1 = Props.StartByteOffset -
                  Adapter_EIP164_DataPath[Adapter_EIP163_Index].StartByteOffset;
                DataPath_p->LastByteOffset1 = Props.LastByteOffset;
                DataPath_p->Flags1 = Props.Flags;
            }
            else
            {  /* Device = on chain 2. */
                DataPath_p->StartByteOffset2 = Props.StartByteOffset -
                  Adapter_EIP164_DataPath[Adapter_EIP163_Index].StartByteOffset;
                DataPath_p->LastByteOffset2 = Props.LastByteOffset;
                DataPath_p->Flags2 = Props.Flags;
            }
        }
    }

    return 0;
}


/*----------------------------------------------------------------------------
 * Adapter_Init_DataPath_GetPIndexRange
 */
int
Adapter_Init_DataPath_GetIndexRange(
        unsigned int Index,
        unsigned int * const FirstIndex_p,
        unsigned int * const LastIndex_p)
{
    unsigned int StartIndex;

    if (Adapter_InitLib_Index_Initialize() < 0)
        return -1;

    StartIndex = ADAPTER_EIP164_DEV_ID(Index) - Adapter_EIP164_Index;

    *FirstIndex_p = StartIndex;
    *LastIndex_p = StartIndex + ADAPTER_DATAPATH_DEVICE_COUNT;

    return 0;
}


/* end of file adapter_init_support.c */
