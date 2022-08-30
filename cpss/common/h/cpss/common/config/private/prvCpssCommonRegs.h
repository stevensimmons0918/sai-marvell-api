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
* @file prvCpssCommonRegs.h
*
* @brief This file includes the declaration of the structure to hold the
* addresses of Common PP registers.
*
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssCommonRegsh
#define __prvCpssCommonRegsh

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Maximal number of DFX tiles in multicore system */
#define PRV_CPSS_DFX_TILE_MAX_CNS                                           4
/* Maximal number of DFX chiplets in tile */
#define PRV_CPSS_DFX_CHIPLET_IN_TILE_CNS                                    4

#pragma pack(push, 4)
typedef struct{

   struct /*DFXServerUnitsDeviceSpecificRegs*/{

       GT_U32    deviceResetCtrl;
       /* SkipInitializationMatrixes registers related to legacy, SIP5 and SIP6 devices           */
       /* For SIP6 devices DFX units are multiple and present in each Tile (2 Processing Cores    */
       /* container) and in each GOP.                                                             */
       /* SIP6 SkipInitializationMatrixes registers of Tile and of GOP are similar, but there are */
       /* some different registers with the same offsets.                                         */
       /* The registers DB contains all SkipInitializationMatrixes registers both common, both of */
       /* registers of Tile only, both registers of GOP only.                                     */
       /* addresses in DB adjusted to Tile DFX address range.                                     */
       /* For access to GOP DFX these Registers Addresses should be recalculated using register   */
       /* offset inside DFX unit from this DB only and ajusting it to GOP address range.          */
       /*                                                                                         */
       /* SIP5 devices and SIP6 Processing Core (Eagle) and SIP6 GOP SkipInitializationMatrixes */
       GT_U32    configSkipInitializationMatrix;
       GT_U32    RAMInitSkipInitializationMatrix;
       GT_U32    trafficManagerSkipInitializationMatrix;
       GT_U32    tableSkipInitializationMatrix;
       GT_U32    SERDESSkipInitializationMatrix;
       GT_U32    EEPROMSkipInitializationMatrix;
       GT_U32    PCIeSkipInitializationMatrix;
       GT_U32    DDRSkipInitializationMatrix;
       GT_U32    metalFixRegister;
       /* SIP6 devices Processing Core (Eagle) and GOP SkipInitializationMatrixes */
       GT_U32    SRRSkipInitializationMatrix;
       GT_U32    BISTSkipInitializationMatrix;
       GT_U32    SoftRepairSkipInitializationMatrix;
       GT_U32    RegxGenxSkipInitializationMatrix;
       GT_U32    MultiActionSkipInitializationMatrix;
       GT_U32    DeviceEEPROMSkipInitializationMatrix;
       GT_U32    D2DLinkSkipInitializationMatrix;
       GT_U32    DFXRegistersSkipInitializationMatrix;
       GT_U32    DFXPipeSkipInitializationMatrix;
       /* SIP6 devices Processing Core only (Eagle) SkipInitializationMatrixes */
       GT_U32    Core_TileSkipInitializationMatrix;
       GT_U32    Core_MngSkipInitializationMatrix;
       GT_U32    Core_D2DSkipInitializationMatrix;
       GT_U32    Core_RavenSkipInitializationMatrix;
       /* SIP6 devices Processing GOP only (Raven) SkipInitializationMatrixes */
       GT_U32    GOP_D2DSkipInitializationMatrix;
       GT_U32    GOP_MainSkipInitializationMatrix;
       GT_U32    GOP_CNMSkipInitializationMatrix;
       GT_U32    GOP_GWSkipInitializationMatrix;

       GT_U32    deviceSAR1;
       GT_U32    deviceSAR2;
       GT_U32    deviceSAR1Override;
       GT_U32    deviceSAR2Override;
       GT_U32    deviceCtrl2;
       GT_U32    deviceCtrl6;
       GT_U32    deviceCtrl7;
       GT_U32    deviceCtrl9;
       GT_U32    deviceCtrl0;
       GT_U32    deviceCtrl1;
       GT_U32    deviceCtrl3;
       GT_U32    deviceCtrl4;
       GT_U32    deviceCtrl5;
       GT_U32    deviceCtrl8;
       GT_U32    deviceCtrl10;
       GT_U32    deviceCtrl11;
       GT_U32    deviceCtrl12;
       GT_U32    deviceCtrl13;
       GT_U32    deviceCtrl14;
       GT_U32    deviceCtrl15;
       GT_U32    deviceCtrl16;
       GT_U32    deviceCtrl17;
       GT_U32    deviceCtrl18;
       GT_U32    deviceCtrl19;
       GT_U32    deviceCtrl20;
       GT_U32    deviceCtrl21;
       GT_U32    deviceCtrl23;
       GT_U32    deviceCtrl24;
       GT_U32    deviceCtrl25;
       GT_U32    deviceCtrl26;
       GT_U32    deviceCtrl27;
       GT_U32    deviceCtrl28;
       GT_U32    deviceCtrl29;
       GT_U32    deviceCtrl30;
       GT_U32    deviceCtrl31;
       GT_U32    deviceCtrl32;
       GT_U32    deviceCtrl33;
       GT_U32    deviceCtrl34;
       GT_U32    deviceCtrl38;
       GT_U32    deviceCtrl44;
       GT_U32    deviceCtrl48;
       GT_U32    deviceCtrl49;
       GT_U32    deviceCtrl50;
       GT_U32    deviceStatus0;
       GT_U32    deviceStatus1;
       GT_U32    initializationStatusDone;
       GT_U32    pllMiscConfig;
       GT_U32    pllMiscParameters;
       GT_U32    DFXSkipInitializationMatrix;
       GT_U32    CPUSkipInitializationMatrix;
       GT_U32    pllPTPConfig;
       GT_U32    pllPTPParams;
       GT_U32    pllPTPParams1;
       GT_U32    cnmSkipInitializationMatrix;
       GT_U32    coreSkipInitializationMatrix;
       GT_U32    poeSkipInitializationMatrix;
       GT_U32    mppSkipInitializationMatrix;
   }DFXServerUnitsDeviceSpecificRegs;

   struct /*DFXServerUnits*/{

       struct /*DFXServerRegs*/{

           GT_U32    AVSDisabledCtrl2[1]/*AVS*/;
           GT_U32    AVSDisabledCtrl1[1]/*AVS*/;
           GT_U32    temperatureSensor28nmCtrlLSB;/* sip6 not use it. instead use externalTemperatureSensorControlLSB */
           GT_U32    serverXBARTargetPortConfig[4]/*Target Port*/;
           GT_U32    serverStatus;
           GT_U32    serverInterruptSummaryCause;
           GT_U32    serverInterruptSummaryMask;
           GT_U32    serverInterruptMask;
           GT_U32    serverInterruptCause;
           GT_U32    serverAddrSpace;
           GT_U32    pipeSelect;
           GT_U32    temperatureSensor28nmCtrlMSB;/* sip6 not use it. instead use externalTemperatureSensorControlMSB */
           GT_U32    temperatureSensorStatus;     /* sip6 not use it. instead use externalTemperatureSensorStatus     */
           GT_U32    AVSStatus[1]/*AVS*/;
           GT_U32    AVSMinStatus[1]/*AVS*/;
           GT_U32    AVSMaxStatus[1]/*AVS*/;
           GT_U32    AVSEnabledCtrl[1]/*AVS*/;
           GT_U32    snoopBusStatus;
           GT_U32    deviceIDStatus;
           GT_U32    JTAGDeviceIDStatus;
           GT_U32    PLLCoreParameters;
           GT_U32    PLLCoreConfig;

           GT_U32    externalTemperatureSensorControlLSB;/* sip6 used instead of temperatureSensor28nmCtrlLSB */
           GT_U32    externalTemperatureSensorControlMSB;/* sip6 used instead of temperatureSensor28nmCtrlMSB */
           GT_U32    externalTemperatureSensorStatus;    /* sip6 used instead of temperatureSensorStatus      */
       }DFXServerRegs;

   }DFXServerUnits;

   /* bookmark to skip this struct during registers printing */
   GT_U32    dfxClientBookmark;     /* PRV_CPSS_SW_PTR_BOOKMARK_CNS */
   GT_U32    dfxClientBookmarkType; /* PRV_CPSS_SW_TYPE_WRONLY_CNS */
   GT_U32    dfxClientBookmarkSize;

   struct /*DFXClientUnits*/{
       GT_U32    clientControl;
       GT_U32    clientDataControl;
       GT_U32    clientRamBISTInfo[4];
       GT_U32    BISTControl/*Target Port*/;
       GT_U32    BISTOpCode;
       GT_U32    BISTMaxAddress;
       GT_U32    dummyWrite;
       GT_U32    clientRamMC_Group[4];
       GT_U32    clientStatus;

   }DFXClientUnits;

    /* bookmark to skip this struct during registers printing */
   GT_U32    dfxRamBookmark;     /* PRV_CPSS_SW_PTR_BOOKMARK_CNS */
   GT_U32    dfxRamBookmarkType; /* PRV_CPSS_SW_TYPE_WRONLY_CNS */
   GT_U32    dfxRamBookmarkSize;

   struct /*DFXRam*/{
       GT_U32   memoryControl;
   }DFXRam;


    struct
    {
        GT_U32 Match_Sample;
        GT_U32 Peak_Logging_Value;
        GT_U32 General_Configurations;
        GT_U32 iDebug_Interrupt_Mask;
        GT_U32 iDebug_Interrupt_Cause;
        GT_U32 Xsb_Info;
        GT_U32 Sampling_Control;
        GT_U32 Time_Stamping_Status;
        GT_U32 Rate_Byte_Counter_LSB;
        GT_U32 Rate_Byte_Counter_MSB;
        GT_U32 Rate_Desc_Counter;
        GT_U32 Rate_Counter_Control;
        GT_U32 Rate_Desc_Lower_Bound;
        GT_U32 Rate_Desc_Upper_Bound;
        GT_U32 Rate_Byte_Lower_Bound_lsb;
        GT_U32 Rate_Byte_Upper_Bound;
        GT_U32 Rate_Counter_Status;
        GT_U32 idebug_Feature_Status;
        GT_U32 Rate_Byte_Upper_Bound_MSB;
        GT_U32 Rate_Byte_Upper_Bound_LSB;
        GT_U32 Rate_Time_Window;
        GT_U32 Match_Mask;
        GT_U32 Match_Counter;
        GT_U32 Match_Key;
    }
    DFX_XSB;

    struct /*DFXServerUnits - core*/{

        struct /*DFXServerRegs*/{

            GT_U32    AVSDisabledCtrl2[1]/*AVS*/;
            GT_U32    AVSDisabledCtrl1[1]/*AVS*/;
            GT_U32    temperatureSensor28nmCtrlLSB;
            GT_U32    serverXBARTargetPortConfig[4]/*Target Port*/;
            GT_U32    serverStatus;
            GT_U32    serverInterruptSummaryCause;
            GT_U32    serverInterruptSummaryMask;
            GT_U32    serverInterruptMask;
            GT_U32    serverInterruptCause;
            GT_U32    serverAddrSpace;
            GT_U32    pipeSelect;
            GT_U32    temperatureSensor28nmCtrlMSB;
            GT_U32    temperatureSensorStatus;
            GT_U32    AVSStatus[1]/*AVS*/;
            GT_U32    AVSMinStatus[1]/*AVS*/;
            GT_U32    AVSMaxStatus[1]/*AVS*/;
            GT_U32    AVSEnabledCtrl[1]/*AVS*/;
            GT_U32    snoopBusStatus;
            GT_U32    deviceIDStatus;
            GT_U32    JTAGDeviceIDStatus;
            GT_U32    PLLCoreParameters;
            GT_U32    PLLCoreConfig;
            GT_U32    externalTemperatureSensorStatus;
        }DFXServerRegs;

    }sip6_tile_DFXServerUnits[PRV_CPSS_DFX_TILE_MAX_CNS];

    struct /*DFXServerUnits - chiplet*/{

        struct /*DFXServerRegs*/{

            GT_U32    AVSDisabledCtrl2[1]/*AVS*/;
            GT_U32    AVSDisabledCtrl1[1]/*AVS*/;
            GT_U32    temperatureSensor28nmCtrlLSB;
            GT_U32    serverXBARTargetPortConfig[4]/*Target Port*/;
            GT_U32    serverStatus;
            GT_U32    serverInterruptSummaryCause;
            GT_U32    serverInterruptSummaryMask;
            GT_U32    serverInterruptMask;
            GT_U32    serverInterruptCause;
            GT_U32    serverAddrSpace;
            GT_U32    pipeSelect;
            GT_U32    temperatureSensor28nmCtrlMSB;
            GT_U32    temperatureSensorStatus;
            GT_U32    AVSStatus[1]/*AVS*/;
            GT_U32    AVSMinStatus[1]/*AVS*/;
            GT_U32    AVSMaxStatus[1]/*AVS*/;
            GT_U32    AVSEnabledCtrl[1]/*AVS*/;
            GT_U32    snoopBusStatus;
            GT_U32    deviceIDStatus;
            GT_U32    JTAGDeviceIDStatus;
            GT_U32    PLLCoreParameters;
            GT_U32    PLLCoreConfig;
            GT_U32    externalTemperatureSensorStatus;
        }DFXServerRegs;

    }sip6_chiplet_DFXServerUnits[PRV_CPSS_DFX_CHIPLET_IN_TILE_CNS * PRV_CPSS_DFX_TILE_MAX_CNS];

   /* bookmark to skip the structures below during registers printing */
   GT_U32   sip6_dfxRamBookmark;       /* PRV_CPSS_SW_PTR_BOOKMARK_CNS */
   GT_U32   sip6_dfxRamBookmarkType;    /* PRV_CPSS_SW_TYPE_WRONLY_CNS */
   GT_U32   sip6_dfxRamBookmarkSize;

    struct /*DFXClientUnits - core*/{
        GT_U32    clientControl;
        GT_U32    clientDataControl;
        GT_U32    clientRamBISTInfo[4];
        GT_U32    BISTControl/*Target Port*/;
        GT_U32    BISTOpCode;
        GT_U32    BISTMaxAddress;
        GT_U32    dummyWrite;
        GT_U32    clientRamMC_Group[4];
        GT_U32    clientStatus;
        GT_U32    clientInterruptCause;
        GT_U32    clientInterruptMask;
        GT_U32    clientMemoriesInterruptStatus[4];
    }sip6_tile_DFXClientUnits[PRV_CPSS_DFX_TILE_MAX_CNS];

    struct /*DFXClientUnits - chiplet*/{
        GT_U32    clientControl;
        GT_U32    clientDataControl;
        GT_U32    clientRamBISTInfo[4];
        GT_U32    BISTControl/*Target Port*/;
        GT_U32    BISTOpCode;
        GT_U32    BISTMaxAddress;
        GT_U32    dummyWrite;
        GT_U32    clientRamMC_Group[4];
        GT_U32    clientStatus;
        GT_U32    clientInterruptCause;
        GT_U32    clientInterruptMask;
        GT_U32    clientMemoriesInterruptStatus[4];
    }sip6_chiplet_DFXClientUnits[PRV_CPSS_DFX_CHIPLET_IN_TILE_CNS * PRV_CPSS_DFX_TILE_MAX_CNS];

    struct /*DFXRam*/{
        GT_U32   memoryErroneousAddress;
        GT_U32   memoryEccSyndrome;
        GT_U32   memoryErroneousSegment;
        GT_U32   memoryControl;
        GT_U32   memoryInterruptMask;
        GT_U32   memoryInterruptCause;
    }sip6_tile_DFXRam[PRV_CPSS_DFX_TILE_MAX_CNS];

    struct /*DFXRam*/{
        GT_U32   memoryErroneousAddress;
        GT_U32   memoryEccSyndrome;
        GT_U32   memoryErroneousSegment;
        GT_U32   memoryControl;
        GT_U32   memoryInterruptMask;
        GT_U32   memoryInterruptCause;
    }sip6_chiplet_DFXRam[PRV_CPSS_DFX_CHIPLET_IN_TILE_CNS * PRV_CPSS_DFX_TILE_MAX_CNS];

}PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC;
/* restore alignment setting */
#pragma pack(pop)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssCommonRegsh */

