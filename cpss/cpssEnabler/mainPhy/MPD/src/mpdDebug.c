/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/
/**
 * @file mpdDebug.c
 *	@brief PHY debug functions.
 *
 */

#include    <mtdApiTypes.h>
#include    <mtdAPI.h>
#include    <mtdAPIInternal.h>
#include    <mtdApiRegs.h>
#include    <mtdDiagnostics.h>
#include    <mtdDiagnosticsRegDumpData.h>

#include    <mpdTypes.h>
#include    <mpdPrv.h>
#include    <mpdDebug.h>


/* global string, used for debug */
char prvMpdDebugString[256];

#define MPD_DEBUG_ARRAY_SIZE_MAC(_arr) (sizeof(_arr) / sizeof(char *))

static char * prvMpdDebugConvertPhyOpToText_ARR[] = {
    "MPD_OP_CODE_SET_MDIX_E",
    "MPD_OP_CODE_GET_MDIX_E",
    "MPD_OP_CODE_GET_MDIX_ADMIN_E",
    "MPD_OP_CODE_SET_AUTONEG_E",
    "MPD_OP_CODE_GET_AUTONEG_ADMIN_E",
    "MPD_OP_CODE_SET_RESTART_AUTONEG_E",
    "MPD_OP_CODE_SET_DUPLEX_MODE_E",
    "MPD_OP_CODE_SET_SPEED_E",
    "MPD_OP_CODE_SET_VCT_TEST_E",
    "MPD_OP_CODE_GET_EXT_VCT_PARAMS_E",
    "MPD_OP_CODE_GET_CABLE_LEN_E",
    "MPD_OP_CODE_SET_RESET_PHY_E",
    "MPD_OP_CODE_SET_PHY_DISABLE_OPER_E",
    "MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E",
    "MPD_OP_CODE_SET_ADVERTISE_FC_E",
    "MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E",
    "MPD_OP_CODE_SET_POWER_MODULES_E",
    "MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E",
    "MPD_OP_CODE_GET_GREEN_READINESS_E",
    "MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E",
    "MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E",
    "MPD_OP_CODE_GET_I2C_READ_E",
    "MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E",
    "MPD_OP_CODE_SET_MDIO_ACCESS_E",
    "MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E",
    "MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E",
    "MPD_OP_CODE_GET_EEE_REMOTE_STATUS_E",
    "MPD_OP_CODE_SET_LPI_EXIT_TIME_E",
    "MPD_OP_CODE_SET_LPI_ENTER_TIME_E",
    "MPD_OP_CODE_GET_EEE_CAPABILITY_E",
    "MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E",
    "MPD_OP_CODE_GET_VCT_CAPABILITY_E",
    "MPD_OP_CODE_GET_DTE_STATUS_E",
    "MPD_OP_CODE_GET_TEMPERATURE_E",
    "MPD_OP_CODE_GET_REVISION_E",
    "MPD_OP_CODE_GET_AUTONEG_SUPPORT_E",
    "MPD_OP_CODE_SET_SPEED_EXT_E",
    "MPD_OP_CODE_GET_SPEED_EXT_E",
    "MPD_OP_CODE_SET_SERDES_TUNE_E",
    "MPD_OP_CODE_SET_SERDES_LANE_POLARITY_E",
    "MPD_OP_CODE_SET_AUTONEG_MULTISPEED_E"
    "PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E",
    "PRV_MPD_OP_CODE_SET_ENABLE_FIBER_PORT_STATUS_E",
    "PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E",
    "PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E",
    "PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E",
    "PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E",
    "PRV_MPD_OP_CODE_SET_LOOP_BACK_E",
    "PRV_MPD_OP_CODE_GET_VCT_OFFSET_E",
    "PRV_MPD_OP_CODE_INIT_E",
    "PRV_MPD_OP_CODE_SET_ERRATA_E",
    "PRV_MPD_OP_CODE_SET_FAST_LINK_DOWN_ENABLE_E"
    "PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E",
    "PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E",
    "PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E"
};

static  char *  prvMpdDebugConvertPhyTypeToText_ARR[MPD_TYPE_NUM_OF_TYPES_E] = {
    "NO PHY",
    "88E1543",
    "88E1545",
    "88E1548",
    "88E1680",
    "88E1680L",
    "88E151x",
    "88E3680",
    "88X32x0",
    "88X33x0",
    "88X20x0",
    "88X2180",
    "88E2540",
    "88X3540",
    "88E1780",
    "88E2580",
    "88X7120",
    "USR0",
    "USR1",
    "USR2",
    "USR3",
    "USR4",
    "USR5",
    "USR6",
    "USR7",
    "USR8",
    "USR9",
    "USR10",
    "USR11",
    "USR12",
    "USR13",
    "USR14",
    "USR15",
    "USR16",
    "USR17",
    "USR18",
    "USR19",
    "USR20",
    "USR21",
    "USR22",
    "USR23",
    "USR24",
    "USR25",
    "USR26",
    "USR27",
    "USR28",
    "USR29",
    "USR30",
    "USR31",
    "USR32"
};

static char * prvMpdDebugConvertPhyMdiModeToText_ARR[] = {
    "MPD_MDI_MODE_MEDIA_E",
    "MPD_MDIX_MODE_MEDIA_E",
    "MPD_AUTO_MODE_MEDIA_E"
};

static  char *  prvMpdDebugConvertComboModeToText_ARR[MPD_COMBO_MODE_LAST_E] = {
    "FORCE FIBER",
    "FORCE COPPER",
    "PREFER FIBER",
    "PREFER COPPER"
};

static char * prvMpdDebugConvertPhyMediaTypeToText_ARR[] = {
    "COPPER_MEDIA_E",
    "FIBER_MEDIA_E",
    "INVALID_MEDIA_E"
};



static  char *  prvMpdDebugConvertOpModeToText_ARR[MPD_OP_MODE_LAST_E] = {
    "FIBER",
    "DAC",
    "COPPER SFP",
    "UNKNOWN"
};

static char * prvMpdDebugConvertGreenSetToText_ARR[] = {
    "GREEN_NO_SET",
    "GREEN_ENABLE",
    "GREEN_DISABLE"
};

static char * prvMpdDebugConvertPortSpeedToText_ARR[] = {
    "PORT_SPEED_10",                            /* 0   */
    "PORT_SPEED_100",                           /* 1   */
    "PORT_SPEED_1000",                          /* 2   */
    "PORT_SPEED_10000",                         /* 3   */
    "PORT_SPEED_12000",                         /* 4   */
    "PORT_SPEED_2500",                          /* 5   */
    "PORT_SPEED_5000",                          /* 6   */
    "PORT_SPEED_13600",                         /* 7   */
    "PORT_SPEED_20000",                         /* 8   */
    "PORT_SPEED_40000",                         /* 9   */
    "PORT_SPEED_16000",                         /* 10  */
    "PORT_SPEED_15000",                         /* 11  */
    "PORT_SPEED_75000",                         /* 12  */
    "PORT_SPEED_100000",                        /* 13  */
    "PORT_SPEED_50000",                         /* 14  */
    "PORT_SPEED_140000",                        /* 15  */
    "PORT_SPEED_11800",                         /* 16  */
    "PORT_SPEED_47200",                         /* 17  */
    "PORT_SPEED_22000",                         /* 18  */
    "PORT_SPEED_23600",                         /* 19  */
    "PORT_SPEED_12500",                         /* 20  */
    "PORT_SPEED_25000",                         /* 21  */
    "PORT_SPEED_107000",                        /* 22  */
    "PORT_SPEED_29090",                         /* 23  */
    "PORT_SPEED_200000",                        /* 24  */
    "PORT_SPEED_400000",                        /* 25  */
    "PORT_SPEED_102000",						/* 26  */
    "PORT_SPEED_52500",						    /* 27  */
    "PORT_SPEED_26700",						    /* 28  */
    "PORT_SPEED_NA_CNS",                        /* 29  */ /* GT_SPEED_NA */
};


static char * prvMpdDebugConvertPhySpeedToText_ARR[] = {
    "PHY_SPEED_10",                            /* 0   */
    "PHY_SPEED_100",                           /* 1   */
    "PHY_SPEED_1000",                          /* 2   */
    "PHY_SPEED_10000",                         /* 3   */
    "PHY_SPEED_12000",                         /* 4   */
    "PHY_SPEED_2500",                          /* 5   */
    "PHY_SPEED_5000",                          /* 6   */
    "PHY_SPEED_13600",                         /* 7   */
    "PHY_SPEED_20000",                         /* 8   */
    "PHY_SPEED_40000",                         /* 9   */
    "PHY_SPEED_16000",                         /* 10  */
    "PHY_SPEED_15000",                         /* 11  */
    "PHY_SPEED_75000",                         /* 12  */
    "PHY_SPEED_100000",                        /* 13  */
    "PHY_SPEED_50000",                         /* 14  */
    "PHY_SPEED_140000",                        /* 15  */
    "PHY_SPEED_11800",                         /* 16  */
    "PHY_SPEED_47200",                         /* 17  */
    "PHY_SPEED_22000",                         /* 18  */
    "PHY_SPEED_23600",                         /* 19  */
    "PHY_SPEED_12500",                         /* 20  */
    "PHY_SPEED_25000",                         /* 21  */
    "PHY_SPEED_107000",                        /* 22  */
    "PHY_SPEED_29090",                         /* 23  */
    "PHY_SPEED_200000",                        /* 24  */
    "PHY_SPEED_400000",                        /* 25  */
    "PHY_SPEED_102G_E",                        /* 26  */ 
    "PHY_SPEED_53500M_E",                      /* 27  */
    "PHY_SPEED_NA_E",                          /* 28  */
    "PHY_SPEED_NA_E"                           /* 29  */
};

static char * prvMpdDebugConvertGreenReadinessToText_ARR[] = {
    "GREEN_READINESS_OPRNORMAL_CNS",
    "GREEN_READINESS_FIBER_CNS",
    "GREEN_READINESS_COMBO_FIBER_CNS",
    "GREEN_READINESS_NOT_SUPPORTED_CNS",
};

static char * prvMpdDebugConvertPhyKindToText_ARR[]= {
    "MPD_KIND_COPPER_E",
    "MPD_KIND_SFP_E",
    "MPD_KIND_COMBO_E ",
    "MPD_KIND_INVALID_E"
};

static char * prvMpdDebugConvertEEESpeedToText_ARR[] = {
    "PHY_EEE_SPEED_10M_CNS",
    "PHY_EEE_SPEED_100M_CNS",
    "PHY_EEE_SPEED_1G_CNS",
    "PHY_EEE_SPEED_10G_CNS",
    "PHY_EEE_SPEED_MAX_CNS"
};

static char * prvMpdDebugConvertEEESCapabilityToText_ARR[] = {
    "BIT_MAP_100BASE_TX_CNS",
    "BIT_MAP_1000BASE_T_CNS",
    "BIT_MAP_10GBASE_T_CNS",
    "BIT_MAP_1000BASE_KX_CNS",
    "BIT_MAP_10GBASE_KX4_CNS",
    "BIT_MAP_10GBASE_KR_CNS",
    "BIT_MAP_2500BASE_T_CNS",
    "BIT_MAP_5GBASE_T_CNS"
};


static  char *  prvMpdDebugConvertMacOnPhyToText_ARR[3] = {
    "NO_BYPASS",
    "BYPASS",
    "NOT INITIALIZED"
};

static  char *  prvMpdDebugConvertUsxTypeToText_ARR[MPD_PHY_USX_MAX_TYPES] = {
    "NO_USX",
    "DXGMII 10G",
    "DXGMII 20G",
    "QXGMII 10G",
    "QXGMII 20G",
    "OXGMII"
};

static PRV_MPD_DEBUG_CONVERT_STC prvDebugConertDb_ARR[MPD_DEBUG_CONVERT_LAST_E] = {
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhyTypeToText_ARR),                prvMpdDebugConvertPhyTypeToText_ARR         },  /*  MPD_DEBUG_CONVERT_PHY_TYPE_E                */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhyOpToText_ARR),                  prvMpdDebugConvertPhyOpToText_ARR           },  /*  MPD_DEBUG_CONVERT_OP_ID_E                   */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhyMdiModeToText_ARR),             prvMpdDebugConvertPhyMdiModeToText_ARR      },  /*  MPD_DEBUG_CONVERT_MDI_MODE_E                */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertComboModeToText_ARR),              prvMpdDebugConvertComboModeToText_ARR       },  /*  MPD_DEBUG_CONVERT_CONBO_MODE_E              */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhyMediaTypeToText_ARR),           prvMpdDebugConvertPhyMediaTypeToText_ARR    },  /*  MPD_DEBUG_CONVERT_MEDIA_TYPE_E              */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertGreenSetToText_ARR),               prvMpdDebugConvertGreenSetToText_ARR        },  /*  MPD_DEBUG_CONVERT_GREEN_SET_E               */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPortSpeedToText_ARR),              prvMpdDebugConvertPortSpeedToText_ARR       },  /*  MPD_DEBUG_CONVERT_PORT_SPEED_E              */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertGreenReadinessToText_ARR),         prvMpdDebugConvertGreenReadinessToText_ARR  },  /*  MPD_DEBUG_CONVERT_GREEN_READINESS_E         */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhyKindToText_ARR),                prvMpdDebugConvertPhyKindToText_ARR         },  /*  MPD_DEBUG_CONVERT_PHY_KIND_E                */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertPhySpeedToText_ARR),               prvMpdDebugConvertPhySpeedToText_ARR        },  /*  MPD_DEBUG_CONVERT_PHY_SPEED_E               */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertEEESpeedToText_ARR),               prvMpdDebugConvertEEESpeedToText_ARR        },  /*  MPD_DEBUG_CONVERT_PHY_EEE_SPEED_E           */
    { MPD_DEBUG_ARRAY_SIZE_MAC(prvMpdDebugConvertEEESCapabilityToText_ARR),         prvMpdDebugConvertEEESCapabilityToText_ARR  }   /*  MPD_DEBUG_CONVERT_PHY_EEE_CAPABILITY_E      */
}; 


extern char * prvMpdDebugConvert(
/*!     INPUTS:             */
    MPD_DEBUG_CONVERT_ENT   conv_type,
    UINT_32                 value
/*!     INPUTS / OUTPUTS:   */
/*!     OUTPUTS:            */
)
{

    PRV_MPD_DEBUG_CONVERT_STC * deb_entry_PTR;

    if (conv_type >= MPD_DEBUG_CONVERT_LAST_E) {
        sprintf(prvMpdDebugString, "Conv_type %d is too high", conv_type);
        return prvMpdDebugString;
    }
   
    deb_entry_PTR = & prvDebugConertDb_ARR[conv_type];
    if (value >= deb_entry_PTR->num_of_entries) {
        sprintf(prvMpdDebugString, "%u - Out of enum range", value);
        return prvMpdDebugString;
    }

    return deb_entry_PTR->text_ARR[value];
}
/*$ END OF  prvMpdDebugConvert */


static BOOLEAN prvMpdIsMtdPhyType(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;
    BOOLEAN ret = FALSE;

    portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (portEntry_PTR) {
        switch (portEntry_PTR->initData_PTR->phyType) {
        case MPD_TYPE_88X32x0_E:
        case MPD_TYPE_88X33x0_E:
        case MPD_TYPE_88X20x0_E:
        case MPD_TYPE_88X2180_E:
        case MPD_TYPE_88E2540_E:
        case MPD_TYPE_88X3540_E:
        case MPD_TYPE_88E2580_E:
            ret = TRUE;
            break;
        default:
            ret = FALSE;
        }
    }

    return ret;
}

static BOOLEAN prvMpdIs1GPhyType(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR;
    BOOLEAN ret = FALSE;

    portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (portEntry_PTR) {
        switch (portEntry_PTR->initData_PTR->phyType) {
        case MPD_TYPE_88E1780_E:
        case MPD_TYPE_88E1543_E:
        case MPD_TYPE_88E1545_E:
        case MPD_TYPE_88E1548_E:
        case MPD_TYPE_88E1680_E:
        case MPD_TYPE_88E1680L_E:
        case MPD_TYPE_88E151x_E:
        case MPD_TYPE_88E3680_E:
            ret = TRUE;
            break;
        default:
            ret = FALSE;
        }
    }

    return ret;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMtdDebugRegDump
 *
 * DESCRIPTION: print commonly used registers
 *
 * APPLICABLE PHY:
 *                  MPD_TYPE_88X32x0_E
 *                  MPD_TYPE_88X33x0_E
 *                  MPD_TYPE_88X20x0_E
 *                  MPD_TYPE_88X2180_E
 *                  MPD_TYPE_88E2540_E
 *                  MPD_TYPE_88X3540_E
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMtdDebugRegDump(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MTD_reg_debug_STC reg_ARR[] = {
             {"CUNIT",MTD_CUNIT_NUM_REGS, cUnitRegData}
            ,{"F2R",MTD_F2R_NUM_REGS, f2rRegData}
            ,{"HUNIT_10GBASER",MTD_HUNIT_10GBASER_NUM_REGS, hUnit10grRegData}
            ,{"HUNIT_1000BX",MTD_HUNIT_1000BX_NUM_REGS, hUnit1000bxRegData}
            ,{"HUNIT_COMMON",MTD_HUNIT_COMMON_NUM_REGS, hUnitCommonRegData}
            ,{"HUNIT_RXAUI",MTD_HUNIT_RXAUI_NUM_REGS, hUnitRxauiRegData}
            ,{"TUNIT_MMD1",MTD_TUNIT_MMD1_NUM_REGS, tUnitMmd1RegData}
            ,{"TUNIT_MMD3",MTD_TUNIT_MMD3_NUM_REGS, tUnitMmd3RegData}
            ,{"TUNIT_MMD3_2",MTD_TUNIT_MMD3_2_NUM_REGS ,tUnitMmd3RegData2}
            ,{"TUNIT_MMD3_3",MTD_TUNIT_MMD3_3_NUM_REGS ,tUnitMmd3RegData3}
            ,{"TUNIT_MMD7",MTD_TUNIT_MMD7_NUM_REGS ,tUnitMmd7RegData}
            ,{"XUNIT_10GBASER",MTD_XUNIT_10GBASER_NUM_REGS, xUnit10grRegData}
            ,{"XUNIT_1000BX",MTD_XUNIT_1000BX_NUM_REGS, xUnit1000bxRegData}
            ,{"XUNIT_COMMON",MTD_XUNIT_COMMON_NUM_REGS, xUnitCommonRegData}
            ,{"",0,NULL}
    };

    UINT_32 i = 0;
    MTD_STATUS mtdStatus;
    MTD_8 outputBuf[MTD_DUMP_MAX_REGISTERS*MTD_SIZEOF_OUTPUT];
    MTD_U16 startLocation = 0;

    if (!prvMpdIsMtdPhyType(rel_ifIndex)) {
        sprintf(prvMpdDebugString, "received not MTD rel_ifIndex %d\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(prvMpdDebugString);
        return MPD_NOT_SUPPORTED_E;
    }

    sprintf(prvMpdDebugString, "Register Dump for rel_ifIndex\n", rel_ifIndex);
    PRV_MPD_LOGGING_MAC(prvMpdDebugString);

    while (reg_ARR[i].numOfRegs != 0) {

        sprintf(prvMpdDebugString, "%s\n=====================================\n", reg_ARR[i].description_STR);
        PRV_MPD_LOGGING_MAC(prvMpdDebugString);

        mtdStatus = mtdCopyRegsToBuf (PRV_MPD_MTD_PORT_OBJECT_MAC(rel_ifIndex),
                reg_ARR[i].regs_ARR,
                reg_ARR[i].numOfRegs,
                outputBuf,
                MTD_DUMP_MAX_REGISTERS*MTD_SIZEOF_OUTPUT,
                &startLocation);

        if (mtdStatus == MTD_OK) {
        	PRV_MPD_LOGGING_MAC((char *)outputBuf);
        } else {
            PRV_MPD_LOGGING_MAC("Some kind of error occurred.");
        }
        i++;
        startLocation = 0;
    }

    return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDebugConvertSpeedToString
 *
 * DESCRIPTION: convert MPD speed to string
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdDebugConvertSpeedToString(
    /*!     INPUTS:             */
    MPD_SPEED_ENT speed,
/*!     INPUTS / OUTPUTS:   */
/*!     OUTPUTS:            */
    char *speedString_PTR
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    char * tmpSpeed_PTR;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                       */
/*!*************************************************************************/
    switch (speed) {
        case MPD_SPEED_10M_E   :
            tmpSpeed_PTR = "10M";
            break;
        case MPD_SPEED_100M_E  :
            tmpSpeed_PTR = "100M";
            break;
        case MPD_SPEED_1000M_E :
            tmpSpeed_PTR = "1G";
            break;
        case MPD_SPEED_10000M_E:
            tmpSpeed_PTR = "10G";
            break;
        case MPD_SPEED_2500M_E :
            tmpSpeed_PTR = "2.5G";
            break;
        case MPD_SPEED_5000M_E :
            tmpSpeed_PTR = "5G";
            break;
        case MPD_SPEED_20000M_E:
            tmpSpeed_PTR = "20G";
            break;
        case MPD_SPEED_100G_E  :
            tmpSpeed_PTR = "100G";
            break;
        case MPD_SPEED_50G_E   :
            tmpSpeed_PTR = "50G";
            break;
        case MPD_SPEED_25G_E   :
            tmpSpeed_PTR = "25G";
            break;
        case MPD_SPEED_200G_E  :
            tmpSpeed_PTR = "200G";
            break;
        case MPD_SPEED_400G_E  :
            tmpSpeed_PTR = "400G";
            break;
        case MPD_SPEED_LAST_E  :
        default:
            tmpSpeed_PTR = "INVALID";
    }
    if (speedString_PTR)
        sprintf(speedString_PTR,"%s",tmpSpeed_PTR);

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDebugConvertMtdCapabilitiesToString
 *
 * DESCRIPTION: convert MTD capabilities to string
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdDebugConvertMtdCapabilitiesToString(
    /*!     INPUTS:             */
    UINT_16 capabilities,
/*!     INPUTS / OUTPUTS:   */
/*!     OUTPUTS:            */
    char *capabilitiesString_PTR
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                       */
/*!*************************************************************************/
    if (capabilities == MTD_ADV_NONE) {
        strcat(capabilitiesString_PTR, "none");
    }
    if (capabilities & MTD_SPEED_10M_HD) {
        strcat(capabilitiesString_PTR, "10M_HD ");
    }
    if (capabilities & MTD_SPEED_10M_FD) {
        strcat(capabilitiesString_PTR, "10M_FD ");
    }
    if (capabilities & MTD_SPEED_100M_HD) {
        strcat(capabilitiesString_PTR, "100M_HD ");
    }
    if (capabilities & MTD_SPEED_100M_FD) {
        strcat(capabilitiesString_PTR, "100M_FD ");
    }
    if (capabilities & MTD_SPEED_1GIG_HD) {
        strcat(capabilitiesString_PTR, "1G_HD ");
    }
    if (capabilities & MTD_SPEED_1GIG_FD) {
        strcat(capabilitiesString_PTR, "1G ");
    }
    if (capabilities & MTD_SPEED_2P5GIG_FD) {
        strcat(capabilitiesString_PTR, "2.5G ");
    }
    if (capabilities & MTD_SPEED_5GIG_FD) {
        strcat(capabilitiesString_PTR, "5G ");
    }
    if (capabilities & MTD_SPEED_10GIG_FD) {
        strcat(capabilitiesString_PTR, "10G ");
    }
    strcat(capabilitiesString_PTR, "\n\0");

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdDebugPrintPortDb
 *
 * DESCRIPTION: print ports DB
 *
 * note: if rel_ifIndex=0 print all ports information
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdDebugPrintPortDb(
    /*!     INPUTS:             */
    UINT_32    rel_ifIndex,
    BOOLEAN    fullDb
/*!     INPUTS / OUTPUTS:   */
/*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32                         index = 0, indexParallel = 0, i = 0;
    UINT_8                          major, minor, inc, test;
    BOOLEAN                         all = FALSE;
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR;
    char parallelListArr[100] = {0}, fwVersion[20];
    char txLane0Serdes[53], txLane1Serdes[53];
	MTD_STATUS                      mtdStatus = MTD_OK;
    BOOLEAN                         isFwVersion = FALSE;
    static BOOLEAN                  legend = FALSE;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                       */
/*!*************************************************************************/
    if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
        all = TRUE;
    } else {
        index = rel_ifIndex;
    }
    
    if (!legend){
        PRV_MPD_LOGGING_MAC("\n");
        PRV_MPD_LOGGING_MAC(" port #->Rel_ifIndex.\n phy #->Phy number, represent the phy number which the port belong to.\n"); 
        PRV_MPD_LOGGING_MAC(" addr->MDIO address.\n busId->MDIO bus ID.\n isPrimary->Representative port.\n");
        PRV_MPD_LOGGING_MAC(" vct_offset->vct offset to get cable length.\n macOnPhyState->mac on phy bypass state.\n adminMode->Port Admin mode.\n");
        PRV_MPD_LOGGING_MAC(" usx type->PHY USX information.\n disableOnInit->if true PHY should be disabled on init.\n");
        PRV_MPD_LOGGING_MAC(" invertMdi->Is invert (mdi->mdix/mdix->mdi) required.\n doSwapAbcd->Swap pairs ABCD to DCBA.\n");
        PRV_MPD_LOGGING_MAC(" shortReachSerdes->Enable the SERDES short reach mode.\n opMode->Fiber port operation mode.\n");
        legend = TRUE;
    }
    PRV_MPD_LOGGING_MAC("\n");
    PRV_MPD_LOGGING_MAC(" port #|  phy type  | phy # | addr | busId | Admin |         speed        |   combo mode  |sfp present|   op mode   |  usx type  |\n");
    PRV_MPD_LOGGING_MAC("----------------------------------------------------------------------------------------------------------------------------------\n");
    do {
        portEntry_PTR = prvMpdGetPortEntry(index);
        if (portEntry_PTR == NULL) {
            index++;
            continue;
        }

        sprintf(prvMpdDebugString,"% -6d | %-10s |%-6d | %-4d | %-5d | %-5s | %-20s | %-13s |%-11s|  %-10s | %-10s |\n",
                    index,
                    (portEntry_PTR->initData_PTR->phyType < MPD_TYPE_NUM_OF_TYPES_E) ? prvMpdDebugConvertPhyTypeToText_ARR[portEntry_PTR->initData_PTR->phyType] : "INVALID",
                    portEntry_PTR->initData_PTR->phyNumber,
                    portEntry_PTR->initData_PTR->mdioInfo.mdioAddress,
                    portEntry_PTR->initData_PTR->mdioInfo.mdioBus,
                    (portEntry_PTR->runningData_PTR->adminMode == MPD_PORT_ADMIN_UP_E) ? "UP" : "DOWN",
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_SPEED_E, portEntry_PTR->runningData_PTR->speed),
                    (portEntry_PTR->runningData_PTR->comboMode < MPD_COMBO_MODE_LAST_E) ? prvMpdDebugConvertComboModeToText_ARR[portEntry_PTR->runningData_PTR->comboMode] : "INVALID",
                    portEntry_PTR->runningData_PTR->sfpPresent?"TRUE" : "FALSE",
                    (portEntry_PTR->runningData_PTR->opMode < MPD_OP_MODE_LAST_E) ? prvMpdDebugConvertOpModeToText_ARR[portEntry_PTR->runningData_PTR->opMode] : "INVALID",
                    (portEntry_PTR->initData_PTR->usxInfo.usxType < MPD_PHY_USX_MAX_TYPES) ? prvMpdDebugConvertUsxTypeToText_ARR[portEntry_PTR->initData_PTR->usxInfo.usxType] : "INVALID");
        PRV_MPD_LOGGING_MAC(prvMpdDebugString);
        index++;
    } while (all && index < MPD_MAX_PORT_NUMBER_CNS);

    PRV_MPD_LOGGING_MAC("\n\n");

    if (fullDb == TRUE){
        if (all == TRUE){
            index = 0;
            indexParallel = 0;
        }
        else {
            index = rel_ifIndex;
            indexParallel = rel_ifIndex;
        }
        PRV_MPD_LOGGING_MAC(" port #| Invert | Swap | Disable |      mac on      |  vct   | ShortReach |     TxSerdes lane0      |     TxSerdes lane1        |\n");
        PRV_MPD_LOGGING_MAC("       |   Mdi  | Abcd | On Init |    phy state     | offset |  Serdes    |                         |                           |\n");
        PRV_MPD_LOGGING_MAC("---------------------------------------------------------------------------------------------------------------------------------\n");
        do {
                portEntry_PTR = prvMpdGetPortEntry(index);
                if (portEntry_PTR == NULL) {
                    index++;
                    continue;
                } 
                sprintf(txLane0Serdes, "prTap %d,mTap %d,poTap %d" , 
                        portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_0.pre_tap, 
                        portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_0.main_tap,
                        portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_0.post_tap);
                sprintf(txLane1Serdes, "prTap %d, mTap %d, poTap %d " , 
                        portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_1.pre_tap, 
                        portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_1.main_tap,
                        portEntry_PTR->initData_PTR->phyTxSerdesParams.lane_1.post_tap);
                sprintf(prvMpdDebugString, "% -6d | %-5s | %-5s | %-7s | %-16s | %-6d | %-10s | %-23s | %-23s |\n", 
                index,
                (portEntry_PTR->initData_PTR->invertMdi) ? "TRUE" : "FALSE",
                (portEntry_PTR->initData_PTR->doSwapAbcd) ? "TRUE" : "FALSE",
                (portEntry_PTR->initData_PTR->disableOnInit) ? "TRUE" : "FALSE",
                (portEntry_PTR->runningData_PTR->macOnPhyState <= PRV_MPD_MAC_ON_PHY_NOT_INITIALIZED_E) ? prvMpdDebugConvertMacOnPhyToText_ARR[portEntry_PTR->runningData_PTR->macOnPhyState] : "INVALID",
                portEntry_PTR->runningData_PTR->vct_offset,
                (portEntry_PTR->initData_PTR->shortReachSerdes) ? "TRUE" : "FALSE",
                txLane0Serdes,
                txLane1Serdes);
                PRV_MPD_LOGGING_MAC(prvMpdDebugString);
                index++;
            } while (all && index < MPD_MAX_PORT_NUMBER_CNS);

        PRV_MPD_LOGGING_MAC("\n\n");
        PRV_MPD_LOGGING_MAC(" *** FW db ***\n");
        PRV_MPD_LOGGING_MAC(" port #| isPrimary | Downloaded |   Version    |                                       Parallel list - mdioAddress                                    |\n");
        PRV_MPD_LOGGING_MAC("-------------------------------------------------------------------------------------------------------------------------------------------------------\n");
        do {
            portEntry_PTR = prvMpdGetPortEntry(indexParallel);
            if (portEntry_PTR == NULL) {
                indexParallel++;
                continue;
            } 
            isFwVersion= FALSE;
            if (portEntry_PTR->runningData_PTR->phyFw_PTR != NULL){
                for (i = 0; i<portEntry_PTR->runningData_PTR->phyFw_PTR->num_of_parallel_ports; i++){
                    sprintf(parallelListArr, "%d, ", portEntry_PTR->runningData_PTR->phyFw_PTR->parallel_list[i]);
                }
              
                mtdStatus = mtdGetFirmwareVersion(	PRV_MPD_MTD_PORT_OBJECT_MAC(portEntry_PTR->rel_ifIndex),
                                                    &major,
                                                    &minor,
                                                    &inc,
                                                    &test);
                if (mtdStatus == MTD_OK){
                    sprintf(fwVersion, "%u.%u.%u.%u", major, minor, inc, test);
                    isFwVersion = TRUE;
                }
            }
            else {
                sprintf(parallelListArr, "NO FW");
            }
            sprintf(prvMpdDebugString, "%-6d | %-9s | %-10s | %-12s | %-100s |\n", 
             indexParallel,
             (portEntry_PTR->runningData_PTR->isRepresentative) ? "TRUE" : "FALSE",
             (portEntry_PTR->runningData_PTR->phyFw_PTR != NULL) ? (portEntry_PTR->runningData_PTR->phyFw_PTR->isDownloaded) ? "TRUE" : "FALSE" : "NO FW",
             (isFwVersion) ? fwVersion : "NO FW",
             parallelListArr);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            indexParallel++;
        } while (all && indexParallel < MPD_MAX_PORT_NUMBER_CNS);
    }

    PRV_MPD_LOGGING_MAC("\n\n");

    return MPD_OK_E;

}

/* ***************************************************************************
 * FUNCTION NAME: mpdMdioDebugCl45WriteRegister
 *
 * DESCRIPTION: relevant to 10G PHYs (Clause45)
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMdioDebugCl45WriteRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 device,
    UINT_16 address,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR;
    MPD_RESULT_ENT status = MPD_OK_E;

    if (prvMpdIs1GPhyType(rel_ifIndex)) {
        sprintf(prvMpdDebugString, "received not 10G PHY rel_ifIndex %d\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(prvMpdDebugString);
        return MPD_NOT_SUPPORTED_E;
    }

    portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);

    if (portEntry_PTR != NULL) {
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, device ,address ,PRV_MPD_MDIO_WRITE_ALL_MASK_CNS ,value);
        if (status == MPD_OK_E) {
            sprintf(prvMpdDebugString,"rel_ifIndex %d %d.0x%x=0x%x\n", rel_ifIndex, device, address, value);
        } else {
            sprintf(prvMpdDebugString,"rel_ifIndex %d Failed Cl45 write operation\n", rel_ifIndex);
            status = MPD_OP_FAILED_E;
        }
    } else {
        sprintf(prvMpdDebugString,"rel_ifIndex %d does not exist\n", rel_ifIndex);
        status = MPD_OP_FAILED_E;
    }

    PRV_MPD_LOGGING_MAC(prvMpdDebugString);

    return status;
}

/* ***************************************************************************
 * FUNCTION NAME: mpdMdioDebugCl45ReadRegister
 *
 * DESCRIPTION: relevant to 10G PHYs (Clause45)
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMdioDebugCl45ReadRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 device,
    UINT_16 address
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR;
    UINT_16 value;
    MPD_RESULT_ENT status = MPD_OK_E;

    if (prvMpdIs1GPhyType(rel_ifIndex)) {
        sprintf(prvMpdDebugString, "received not 10G PHY rel_ifIndex %d\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(prvMpdDebugString);
        return MPD_NOT_SUPPORTED_E;
    }

    portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (portEntry_PTR != NULL) {
        status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, device ,address ,&value);
        if (status == MPD_OK_E) {
            sprintf(prvMpdDebugString,"rel_ifIndex %d %d.0x%x=0x%x\n", rel_ifIndex, device, address, value);
        } else {
            sprintf(prvMpdDebugString,"rel_ifIndex %d Failed Cl45 read operation\n", rel_ifIndex);
            status = MPD_OP_FAILED_E;
        }
    } else {
        sprintf(prvMpdDebugString,"rel_ifIndex %d does not exist\n", rel_ifIndex);
        status = MPD_OP_FAILED_E;
    }

    PRV_MPD_LOGGING_MAC(prvMpdDebugString);

    return status;
}


/* ***************************************************************************
 * FUNCTION NAME: mpdMdioDebugCl22WriteRegister
 *
 * DESCRIPTION: relevant to 1G PHYs (Clause22)
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMdioDebugCl22WriteRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 page,
    UINT_16 address,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR;
    MPD_RESULT_ENT status = MPD_OK_E;
    BOOLEAN prev_state;

    if (!prvMpdIs1GPhyType(rel_ifIndex)) {
        sprintf(prvMpdDebugString, "received not 1G PHY rel_ifIndex %d\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(prvMpdDebugString);
        return MPD_NOT_SUPPORTED_E;
    }

    portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);

    if (portEntry_PTR != NULL) {
        /* 1G phy which support page select we need to disable PPU */
        PRV_MPD_SMI_AUTONEG_DISABLE_MAC(portEntry_PTR->rel_ifIndex, TRUE, &prev_state);
        status = prvMpdMdioWriteRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, page ,address ,PRV_MPD_MDIO_WRITE_ALL_MASK_CNS ,value);
        PRV_MPD_SMI_AUTONEG_DISABLE_MAC(portEntry_PTR->rel_ifIndex, prev_state, &prev_state);
        if (status == MPD_OK_E) {
            sprintf(prvMpdDebugString,"rel_ifIndex %d 0x%x=0x%x\n", rel_ifIndex, address, value);
        } else {
            sprintf(prvMpdDebugString,"rel_ifIndex %d Failed write operation\n", rel_ifIndex);
            status = MPD_OP_FAILED_E;
        }
    } else {
        sprintf(prvMpdDebugString,"rel_ifIndex %d does not exist\n", rel_ifIndex);
        status = MPD_OP_FAILED_E;
    }

    PRV_MPD_LOGGING_MAC(prvMpdDebugString);

    return status;
}

/* ***************************************************************************
 * FUNCTION NAME: mpdMdioDebugCl22ReadRegister
 *
 * DESCRIPTION: relevant to 1G PHYs (Clause22)
 *
 * note: if readAll=TRUE print all registers of the current page
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMdioDebugCl22ReadRegister(
    /*     INPUTS:             */
    BOOLEAN readAll,
    UINT_32 rel_ifIndex,
    UINT_16 page,
    UINT_16 address
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR;
    UINT_16 value;
    BOOLEAN prev_state;
    MPD_RESULT_ENT status = MPD_OP_FAILED_E;
    MPD_OPERATIONS_PARAMS_UNT phy_params;

    if (!prvMpdIs1GPhyType(rel_ifIndex)) {
        sprintf(prvMpdDebugString, "received not 1G PHY rel_ifIndex %d\n", rel_ifIndex);
        PRV_MPD_LOGGING_MAC(prvMpdDebugString);
        return MPD_NOT_SUPPORTED_E;
    }

    portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (portEntry_PTR != NULL) {
        /* 1G phy which support page select we need to disable PPU */
        PRV_MPD_SMI_AUTONEG_DISABLE_MAC(portEntry_PTR->rel_ifIndex, TRUE, &prev_state);
        if (readAll) {
            /* moving to new page */
            phy_params.internal.phyPageSelect.page = page;
            phy_params.internal.phyPageSelect.readPrevPage = TRUE;
            status = prvMpdPerformPhyOperation(portEntry_PTR,
                                                PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                                                &phy_params);
            if (status == MPD_OP_FAILED_E) {
                sprintf(prvMpdDebugString,"rel_ifIndex %d failed to set page %d\n", rel_ifIndex, page);
                PRV_MPD_LOGGING_MAC(prvMpdDebugString);
                return MPD_OP_FAILED_E;
            }

            sprintf(prvMpdDebugString,"RegisterName   PHY        RegOffset RegVal  [rel_ifIndex:%d]\n", rel_ifIndex);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            PRV_MPD_LOGGING_MAC("------------------------- -------   ---------\n");
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,0 ,&value);
            sprintf(prvMpdDebugString,"Control Register          :  %2d    0x%4x \n", 0, value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,1 ,&value);
            sprintf(prvMpdDebugString,"Status Register           :  %2d    0x%4x \n", 1, value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,2 ,&value);
            sprintf(prvMpdDebugString,"PHY Identifier            :  %2d    0x%4x \n", 2, value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,3 ,&value);
            sprintf(prvMpdDebugString,"PHY Identifier            :  %2d    0x%4x \n", 3, value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,4 ,&value);
            sprintf(prvMpdDebugString,"Auto-Negotiation          :  %2d    0x%4x \n", 4, value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,5 ,&value);
            sprintf(prvMpdDebugString,"Link Partner Ability      :  %2d    0x%4x \n", 5, value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,6 ,&value);
            sprintf(prvMpdDebugString,"Auto-Negotiation Ex       :  %2d    0x%4x \n", 6, value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,7 ,&value);
            sprintf(prvMpdDebugString,"Next Page Transmit        :  %2d    0x%4x \n", 7, value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,8 ,&value);
            sprintf(prvMpdDebugString,"Link Partner Next         :  %2d    0x%4x \n", 8, value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,9 ,&value);
            sprintf(prvMpdDebugString,"1000BASE-T Control        :  %2d    0x%4x \n", 9, value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,10 ,&value);
            sprintf(prvMpdDebugString,"1000BASE-T Status         :  %2d    0x%4x \n", 10,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,16 ,&value);
            sprintf(prvMpdDebugString,"PHY Specific Control      :  %2d    0x%4x \n", 16,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,17 ,&value);
            sprintf(prvMpdDebugString,"PHY Specific Status       :  %2d    0x%4x \n", 17,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,18 ,&value);
            sprintf(prvMpdDebugString,"Interrupt Enable          :  %2d    0x%4x \n", 18,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,19 ,&value);
            sprintf(prvMpdDebugString,"Interrupt Status          :  %2d    0x%4x \n", 19,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,20 ,&value);
            sprintf(prvMpdDebugString,"Extended PHY Specific     :  %2d    0x%4x \n", 20,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,21 ,&value);
            sprintf(prvMpdDebugString,"Receive Error Count       :  %2d    0x%4x \n", 21,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,22 ,&value);
            sprintf(prvMpdDebugString,"Extended address          :  %2d    0x%4x \n", 22,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,23 ,&value);
            sprintf(prvMpdDebugString,"Global Status             :  %2d    0x%4x \n", 23,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,24 ,&value);
            sprintf(prvMpdDebugString,"LED Control               :  %2d    0x%4x \n", 24,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,25 ,&value);
            sprintf(prvMpdDebugString,"PHY Manual Led Override   :  %2d    0x%4x \n", 25,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,26 ,&value);
            sprintf(prvMpdDebugString,"VCT Control               :  %2d    0x%4x \n", 26,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,27 ,&value);
            sprintf(prvMpdDebugString,"VCT Status                :  %2d    0x%4x \n", 27,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,28 ,&value);
            sprintf(prvMpdDebugString,"PHY Specific Control 2    :  %2d    0x%4x \n", 28,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,29 ,&value);
            sprintf(prvMpdDebugString,"Reserved                  :  %2d    0x%4x \n", 29,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,30 ,&value);
            sprintf(prvMpdDebugString,"Reserved                  :  %2d    0x%4x \n", 30,value);
            PRV_MPD_LOGGING_MAC(prvMpdDebugString);
            prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, PRV_MPD_IGNORE_PAGE_CNS ,31 ,&value);
            sprintf(prvMpdDebugString,"Reserved                  :  %2d    0x%4x \n", 31,value);

            /* moving to old page */
            phy_params.internal.phyPageSelect.page = phy_params.internal.phyPageSelect.prevPage;
            phy_params.internal.phyPageSelect.readPrevPage = FALSE;
            status = prvMpdPerformPhyOperation(portEntry_PTR,
                                                PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                                                &phy_params);
            if (status == MPD_OP_FAILED_E) {
                sprintf(prvMpdDebugString,"rel_ifIndex %d failed to set page %d\n", rel_ifIndex, page);
                PRV_MPD_LOGGING_MAC(prvMpdDebugString);
                return MPD_OP_FAILED_E;
            }
        } else {
            status = prvMpdMdioReadRegister(PRV_MPD_DEBUG_FUNC_NAME_MAC(),portEntry_PTR, page ,address ,&value);
            if (status == MPD_OK_E) {
                sprintf(prvMpdDebugString,"rel_ifIndex %d 0x%x=0x%x\n", rel_ifIndex, address, value);
            } else {
                sprintf(prvMpdDebugString,"rel_ifIndex %d Failed read operation\n", rel_ifIndex);
                status = MPD_OP_FAILED_E;
            }
        }
        PRV_MPD_SMI_AUTONEG_DISABLE_MAC(portEntry_PTR->rel_ifIndex, prev_state, &prev_state);
    } else {
        sprintf(prvMpdDebugString,"rel_ifIndex %d does not exist\n", rel_ifIndex);
        status = MPD_OP_FAILED_E;
    }

    PRV_MPD_LOGGING_MAC(prvMpdDebugString);

    return status;
}


MPD_RESULT_ENT prvMpdDebugPerformPhyOperation(
    /*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    MPD_OP_CODE_ENT                 op,
    MPD_OPERATIONS_PARAMS_UNT     * params_PTR,
    BOOLEAN                         before
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{

    BOOLEAN     debug_get, debug_set;
    PRV_MPD_OP_CODE_ENT prv_op_code;

    debug_set = before;
    debug_get = before?FALSE:TRUE;
    switch (op) {
    case MPD_OP_CODE_GET_MDIX_E:
        if (debug_get) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
            "rel_ifIndex %d, op:%s, phy type %s. returned: %s",
            portEntry_PTR->rel_ifIndex,
            prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
            prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
            prvMpdDebugConvert(MPD_DEBUG_CONVERT_MDI_MODE_E,params_PTR->phyMdix.mode));
        }
        break;
    case MPD_OP_CODE_SET_AUTONEG_E:
        if (debug_set) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. params: autoneg %s, capabilities (bitmap) %#x, set as %s",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyAutoneg.enable==MPD_AUTO_NEGOTIATION_ENABLE_E?"Enable":"Disable",
                params_PTR->phyAutoneg.capabilities,
                params_PTR->phyAutoneg.masterSlave==MPD_AUTONEGPREFERENCE_MASTER_E?"Master":"Slave");
        }
        break;
    case MPD_OP_CODE_SET_DUPLEX_MODE_E:
        if (debug_set) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. params: duplex %s",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyDuplex.mode == MPD_DUPLEX_ADMIN_MODE_FULL_E?"FULL":"HALF");
        }
        break;
    case MPD_OP_CODE_SET_SPEED_E:
        if (debug_set) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. params: speed %s",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_SPEED_E, params_PTR->phySpeed.speed));
        }
        break;
    case MPD_OP_CODE_SET_MDIX_E:
        if (debug_set) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. params: mdi/x mode %s",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_MDI_MODE_E,params_PTR->phyMdix.mode));
        }
        break;
    case MPD_OP_CODE_SET_VCT_TEST_E:
        if (debug_get) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. returned: result %d, cable length %d",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyVct.testResult,
                params_PTR->phyVct.cableLength);
        }
        break;
    case MPD_OP_CODE_GET_EXT_VCT_PARAMS_E:
        if (debug_get) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. returned: test type %d, result %d",
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyExtVct.testType,
                params_PTR->phyExtVct.result);
        }
        break;
    case MPD_OP_CODE_GET_CABLE_LEN_E:
        if (debug_get) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. returned: cable length (enum) %d",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyCableLen.cableLength);
        }
        break;
    case MPD_OP_CODE_SET_PHY_DISABLE_OPER_E:
        if (debug_set) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. force link down %d",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyDisable.forceLinkDown);
        }
        break;
    case MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E:
        if (debug_get) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. returned: capabilities (bitmap) %#x",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyRemoteAutoneg.capabilities);
        }
        break;
    case MPD_OP_CODE_SET_ADVERTISE_FC_E:
        if (debug_set) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. advertise FC %d",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyFc.advertiseFc);
        }
        break;
    case MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E:
        if (debug_get) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. returned: %s",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyLinkPartnerFc.pauseCapable?"Capable":"Not Capable");
        }
        break;

    case MPD_OP_CODE_SET_POWER_MODULES_E:
        if (debug_set) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. ED %s, SR %s, perform PHY reset %d",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_GREEN_SET_E, params_PTR->phyPowerModules.energyDetetct),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_GREEN_SET_E, params_PTR->phyPowerModules.shortReach),
                params_PTR->phyPowerModules.performPhyReset);
        }
        break;
    case MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E:
        if (debug_get) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. input params: ED %s, SR %s, port is %s, speed is %s, returned: green consumption = %dMw",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_GREEN_SET_E, params_PTR->phyPowerConsumptions.energyDetetct),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_GREEN_SET_E, params_PTR->phyPowerConsumptions.shortReach),
                (params_PTR->phyPowerConsumptions.portUp)?"Up":"Down",
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PORT_SPEED_E, params_PTR->phyPowerConsumptions.portSpeed),
                params_PTR->phyPowerConsumptions.greenConsumption);
        }
        break;
    case MPD_OP_CODE_GET_GREEN_READINESS_E:
        if (debug_get) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. returned: %s",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_GREEN_READINESS_E,params_PTR->phyGreen.readiness));
        }
        break;
    case MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E:
        if (debug_get) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. returned: %d",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyCableLenNoRange.cableLen);
        }
        break;
    case MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E:
        if (debug_get) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. returned: kind: %s, SFP: %s, media type: %s",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_KIND_E, params_PTR->phyKindAndMedia.phyKind),
                params_PTR->phyKindAndMedia.isSfpPresent?"Present":"Not Present",
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_MEDIA_TYPE_E,params_PTR->phyKindAndMedia.mediaType));
        }
        break;
    case MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E:
        if (debug_set) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. SFP is %s. op_mode %d",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phySfpPresentNotification.isSfpPresent?"Present":"Not Present",
                params_PTR->phySfpPresentNotification.opMode);
        }
        break;
    case MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E:
        if (debug_set) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. speed %s, advertise %s",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_EEE_CAPABILITY_E,params_PTR->phyEeeAdvertize.speedBitmap),
                params_PTR->phyEeeAdvertize.advEnable?"Enable":"Disable");
        }
        break;
    case MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E:
        if (debug_set) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. - %s",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyEeeMasterEnable.masterEnable?"Enable":"Disable");
        }
        break;
    case MPD_OP_CODE_GET_EEE_CAPABILITY_E:
    case MPD_OP_CODE_GET_EEE_REMOTE_STATUS_E:
        if (debug_get) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. returned: (bitmap) %#x",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyEeeCapabilities.enableBitmap);
        }
        break;
    case MPD_OP_CODE_SET_LPI_EXIT_TIME_E:
    case MPD_OP_CODE_SET_LPI_ENTER_TIME_E:
        if (debug_set) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. speed %s time_us %d",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_EEE_SPEED_E,params_PTR->phyEeeLpiTime.speed),
                params_PTR->phyEeeLpiTime.time_us);
        }
        break;
    case MPD_OP_CODE_SET_RESET_PHY_E:
        /* no parameters */
        if (debug_set) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s.",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType));
        }
        break;
    case MPD_OP_CODE_GET_TEMPERATURE_E:
        if (debug_get) {
            PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                "rel_ifIndex %d, op:%s, phy type %s. returned: %d",
                portEntry_PTR->rel_ifIndex,
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                params_PTR->phyTemperature.temperature);
        }
        break;
    case MPD_OP_CODE_GET_DTE_STATUS_E:
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
            "rel_ifIndex %d, op:%s, phy type %s. returned: %d",
            portEntry_PTR->rel_ifIndex,
            prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
            prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
            params_PTR->phyDteStatus.dteDetetcted);
        break;
    case MPD_OP_CODE_GET_REVISION_E:
        PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
            "rel_ifIndex %d, op:%s, phy type %s. revision: %d",
            prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
            params_PTR->phyRevision.revision);
        break;
    case MPD_OP_CODE_GET_I2C_READ_E:
    case MPD_OP_CODE_SET_MDIO_ACCESS_E:
    case MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E:
    case MPD_OP_CODE_GET_VCT_CAPABILITY_E:
        break;
    default:
        prv_op_code = (PRV_MPD_OP_CODE_ENT)op;
        switch (prv_op_code) {
        case PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                    "rel_ifIndex %d, op:%s, phy type %s. returned: %d, %d, %d, %d",
                    portEntry_PTR->rel_ifIndex,
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                    params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[0],
                    params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[1],
                    params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[2],
                    params_PTR->internal.phyEeeMaxTxVal.maxTxVal_ARR[3]);
            }
            break;
        case PRV_MPD_OP_CODE_SET_ENABLE_FIBER_PORT_STATUS_E:
        case PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                    "rel_ifIndex %d, op:%s, phy type %s. params: fiber is %s, port type is %s, media_type is %s",
                    portEntry_PTR->rel_ifIndex,
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                    params_PTR->internal.phyFiberParams.fiberPresent?"Present":"Not Present",
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_CONBO_MODE_E,params_PTR->internal.phyFiberParams.comboMode),
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_PORT_SPEED_E,params_PTR->internal.phyFiberParams.phySpeed),
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_MEDIA_TYPE_E, params_PTR->internal.phyFiberParams.mediaType));
            }
            break;
        case PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E:
        case PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E:
        case PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E:
        case PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E:
        case PRV_MPD_OP_CODE_SET_LOOP_BACK_E:
        case PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E:
        case PRV_MPD_OP_CODE_INIT_E:
        case PRV_MPD_OP_CODE_SET_ERRATA_E:
        case PRV_MPD_OP_CODE_GET_LANE_BMP_E:
        case PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E:
        case PRV_MPD_NUM_OF_OPS_E:
            /* no debug*/
            break;
        case PRV_MPD_OP_CODE_SET_FAST_LINK_DOWN_ENABLE_E:
            if (debug_set) {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                    "rel_ifIndex %d, op:%s, phy type %s. enable: %s",
                    portEntry_PTR->rel_ifIndex,
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                    params_PTR->internal.phyFastLinkDown.enable);
            }
            break;
        case PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                    "rel_ifIndex %d, op:%s, phy type %s. EEE enable mode %s",
                    portEntry_PTR->rel_ifIndex,
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                    (params_PTR->internal.phyEeeEnableMode.enableMode == MPD_EEE_ENABLE_MODE_LINK_CHANGE_E)?"On Link Change event":"On Admin only");
            }
            break;
        case PRV_MPD_OP_CODE_GET_VCT_OFFSET_E:
            if (debug_get) {
                PRV_MPD_DEBUG_LOG_MAC(prvMpdDebugOperationFlagId)(  PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                    "rel_ifIndex %d, op:%s, phy type %s. do_set %d",
                    portEntry_PTR->rel_ifIndex,
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_OP_ID_E, op),
                    prvMpdDebugConvert(MPD_DEBUG_CONVERT_PHY_TYPE_E, portEntry_PTR->initData_PTR->phyType),
                    params_PTR->internal.phyVctOffset.do_set);
            }
            break;
        }
    }
    return MPD_OK_E;
}

/*$ END OF  prvMpdDebugPerformPhyOperation */

