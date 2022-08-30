#include "mtdCopyright.h"
/* Copyright 2013 Marvell International Ltd, See included file for licensing information */

/********************************************************************
This file contains functions and global data for
higher-level functions using MDIO access to perform resets, perform
queries of the phy, etc. that are necessary to control and read
status of the Marvell 88X3240 ethernet PHY.
********************************************************************/
#include "mtdApiRegs.h"
#include "mtdApiTypes.h"
#include "mtdHwCntl.h"
#include "mtdDiagnostics.h"
#include "mtdAPI.h"
#include "mtdCunit.h"
#include "mtdHXunit.h"

/*******************************************************************
 Local Function Prototypes
 *******************************************************************/
STATIC MTD_STATUS mtdAutonegIsSpeedDuplexResolutionDone
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *anSpeedResolutionDone
);

STATIC MTD_STATUS mtdAutonegIsCompleted
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *anStatusReady
);

STATIC MTD_STATUS mtdReadMasterOrSlaveBit
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *msBitFlag
);

STATIC MTD_STATUS mtdGetMasterSlaveConfigFault
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *msConfigFaultFlag
);


/*******************************************************************/
/*******************************************************************
 API Version
 Resets
 Download Mode Selection/Query
 Phy Low Power Mode
 Tunit Low Power Mode
 Parallel Write Mode
 MDIO Download Mode
 *******************************************************************/

/****************************************************************************/
MTD_VOID mtdGetAPIVersion
(
    IN MTD_U8 *major,
    IN MTD_U8 *minor
)
{
    *major = MTD_API_MAJOR_VERSION;
    *minor = MTD_API_MINOR_VERSION;
}

MTD_STATUS mtdSpecialSoftwareReset
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port
)
{
    return (mtdHwSetPhyRegField(contextPtr,port,MTD_TUNIT_PHY_EXT_CTRL_1,15,1,1)); /* bit self clears when done */
}

MTD_STATUS mtdSoftwareReset
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    IN MTD_U16 timeoutMs
)
{
    MTD_U16 counter;
    MTD_BOOL phyReady;

    ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,15,1,1)); /* bit self clears when done */
    if (timeoutMs)
    {
        counter=0;
        ATTEMPT(mtdIsPhyReadyAfterReset(contextPtr,port,&phyReady));
        while (phyReady == MTD_FALSE && counter <= timeoutMs)
        {
            ATTEMPT(mtdWait(contextPtr,1));
            ATTEMPT(mtdIsPhyReadyAfterReset(contextPtr,port,&phyReady));
            counter++;
        }

        if (counter < timeoutMs)
        {
            return MTD_OK;
        }
        else
        {
            return MTD_FAIL; /* timed out without becoming ready */
        }
    }
    else
    {
        return MTD_OK;
    }
}



MTD_STATUS mtdIsPhyInMdioDownloadMode
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *downloadMode
)
{
    MTD_U16 val;

    *downloadMode = MTD_FALSE;

    ATTEMPT(mtdHwXmdioRead(contextPtr,port,MTD_BOOT_STATUS_REG,&val));

    if (((val & MTD_BOOT_PROGRESS_MASK)>>MTD_BOOT_PROGRESS_POS) == MTD_WAIT_DL)
    {
        *downloadMode =  MTD_TRUE;
    }
    else
    {
        *downloadMode = MTD_FALSE;
    }
    return MTD_OK;
}

MTD_STATUS mtdGetPhyResetReason
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *resetType
)
{
    MTD_U16 val;

    *resetType = 3; /*  set to invalid value in case of MDIO error */

    ATTEMPT(mtdHwXmdioRead(contextPtr, port,MTD_BOOT_STATUS_REG,&val));

    *resetType = ((val & MTD_RESET_CAUSE_MASK)>>MTD_RESET_CAUSE_POS);
    return MTD_OK;
}

MTD_STATUS mtdDidPhyAppCodeStart
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *appStarted
)
{
    MTD_U16 val;

    *appStarted = MTD_FALSE;

    ATTEMPT(mtdHwXmdioRead(contextPtr, port,MTD_BOOT_STATUS_REG,&val));

    if ((val & MTD_CODE_STARTED_MASK)>>MTD_CODE_STARTED_POS)
    {
        *appStarted = MTD_TRUE;
    }
    return MTD_OK;
}

MTD_STATUS mtdFlashUploadedChecksumGood
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *checksumGood
)
{
    MTD_U16 val;

    *checksumGood = MTD_FALSE;

    ATTEMPT(mtdHwXmdioRead(contextPtr, port,MTD_BOOT_STATUS_REG,&val));

    if (((val & MTD_BOOT_PROGRESS_MASK)>>MTD_BOOT_PROGRESS_POS) == MTD_JUMPING_TO_APP)
    {
        *checksumGood = MTD_TRUE;
    }
    else
    {
        *checksumGood = MTD_FALSE;
    }
    return MTD_OK;
}

MTD_STATUS mtdCheckForFatalBootError
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *fatalBootError
)
{
    MTD_U16 val;

    *fatalBootError = MTD_TRUE;

    ATTEMPT(mtdHwXmdioRead(contextPtr, port,MTD_BOOT_STATUS_REG,&val));

    if (((val & MTD_FATAL_BOOT_ERROR_MASK)>>MTD_FATAL_BOOT_ERROR_POS) == 0)
    {
        *fatalBootError = MTD_FALSE;
    }

    return MTD_OK;
}

MTD_STATUS mtdIsTunitResponsive
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *tunitReady
)
{
    MTD_BOOL lowpowermode;
    MTD_U8 major, minor, inc, test; /* for firmware version */
    MTD_U16 bistStatus, tunitMode, tunitState;

    *tunitReady = MTD_TRUE;

    /* Checks if T-unit is still in reset NOTE: This checks for several conditions fatal BIST during
       reset, PHY in download mode and no code ever loaded, non-fatal BIST during reset on unreleased
       code, fatal boot code error */
    ATTEMPT(mtdIsPhyReadyAfterReset(contextPtr,port,tunitReady));

    if (*tunitReady == MTD_FALSE)
    {
        return MTD_OK;
    }

    /* Check if T-unit is powered down/in low power mode, which can happen if host powered it down
       or if it happened as a result of fiber-only mode or auto-media detect mode */
    ATTEMPT(mtdIsTunitInLowPowerMode(contextPtr,port,&lowpowermode));

    if (lowpowermode == MTD_TRUE)
    {
        *tunitReady = MTD_FALSE;
        return MTD_OK;
    }

    /* Check if unreleased code has had a BIST or fatal error of some kind, which will cause code
       to hang instead of restarting. */
    ATTEMPT(mtdGetFirmwareVersion(contextPtr,port,&major,&minor,&inc,&test));
    ATTEMPT(mtdGetTunitBISTStatus(contextPtr,port,&bistStatus));

    if ((test != 0) &&
        (bistStatus & (MTD_BIST_SW_FAIL | MTD_BIST_ECC_CORRECT)))
    {
        *tunitReady = MTD_FALSE;
        return MTD_OK;
    }

    /* check if T-unit is in a test mode */
    ATTEMPT(mtdGetTunitFirmwareMode(contextPtr,port,&tunitMode,&tunitState));

    if (tunitMode == MTD_TEST_MODE)
    {
        *tunitReady = MTD_FALSE;
        return MTD_OK;
    }

    return MTD_OK;
}


MTD_STATUS mtdIsPhyReadyAfterReset
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *phyReady
)
{
    MTD_U16 val;

    *phyReady = MTD_FALSE;

    ATTEMPT(mtdHwGetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,15,1,&val));

    if(val)
    {
        *phyReady = MTD_FALSE;   /* if still in reset return '0' (could be coming up, or disabled by download mode) */
    }
    else
    {
        *phyReady = MTD_TRUE;   /* if Phy is in normal operation */
    }
    return MTD_OK;
}

MTD_STATUS mtdPutPhyInLowPowerMode
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port
)
{
    MTD_BOOL xfiloopback, sgmiiloopback, sgmiian_enable,dummy;
    MTD_U16 sgmiispeed, horxunit;

    for (horxunit=MTD_X_UNIT; horxunit<=MTD_H_UNIT; horxunit++)
    {
        ATTEMPT(mtdGet1000BXSGMIIControl(contextPtr,port,horxunit,&sgmiiloopback,&sgmiispeed,&sgmiian_enable,&dummy));
        ATTEMPT(mtdGet10GBRControl(contextPtr,port,horxunit,&xfiloopback,&dummy));

        ATTEMPT(mtdSet1000BXSGMIIControl(contextPtr,port,horxunit,sgmiiloopback,sgmiispeed,sgmiian_enable,MTD_TRUE,MTD_FALSE,MTD_FALSE));
        ATTEMPT(mtdSet10GBRControl(contextPtr,port,horxunit,xfiloopback,MTD_TRUE,MTD_FALSE));
    }

    ATTEMPT(mtdPutTunitInLowPowerMode(contextPtr,port));

    return MTD_OK;
}

MTD_STATUS mtdRemovePhyLowPowerMode
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port
)
{
    MTD_BOOL xfiloopback, sgmiiloopback, sgmiian_enable,dummy;
    MTD_U16 sgmiispeed, horxunit;

    for (horxunit=MTD_X_UNIT; horxunit<=MTD_H_UNIT; horxunit++)
    {
        ATTEMPT(mtdGet1000BXSGMIIControl(contextPtr,port,horxunit,&sgmiiloopback,&sgmiispeed,&sgmiian_enable,&dummy));
        ATTEMPT(mtdGet10GBRControl(contextPtr,port,horxunit,&xfiloopback,&dummy));

        ATTEMPT(mtdSet1000BXSGMIIControl(contextPtr,port,horxunit,sgmiiloopback,sgmiispeed,sgmiian_enable,MTD_FALSE,MTD_FALSE,MTD_FALSE));
        ATTEMPT(mtdSet10GBRControl(contextPtr,port,horxunit,xfiloopback,MTD_FALSE,MTD_FALSE));
    }

    ATTEMPT(mtdRemoveTunitLowPowerMode(contextPtr,port));

    return MTD_OK;
}

MTD_STATUS mtdPutTunitInLowPowerMode
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port
)
{
    return (mtdHwSetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,11,1,1));
}

MTD_STATUS mtdRemoveTunitLowPowerMode
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port
)
{
    if (mtdHwSetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,11,1,0) == MTD_FAIL ||
        mtdHwSetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PCS_CTRL1,11,1,0) == MTD_FAIL)
    {
        return MTD_FAIL;
    }
    else
    {
        return MTD_OK;
    }
}

MTD_STATUS mtdIsTunitInLowPowerMode
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *inLowPowerMode
)
{
    MTD_U16 tunitMode, tunitState;

    ATTEMPT(mtdGetTunitFirmwareMode(contextPtr,port,&tunitMode,&tunitState));

    if (tunitMode == MTD_LOW_POWER_MODE)
    {
        *inLowPowerMode = MTD_TRUE;
    }
    else
    {
        *inLowPowerMode = MTD_FALSE;
    }

    return MTD_OK;
}

MTD_STATUS mtdPutPhyInParallelWriteMode
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port
)
{
    return (mtdHwSetPhyRegField(contextPtr,port,3,0xD101,14,1,1));
}

MTD_STATUS mtdRemovePhyFromParallelWriteMode
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port
)
{
    return (mtdHwSetPhyRegField(contextPtr,port,3,0xD101,14,1,0));
}

MTD_STATUS mtdPutPhyInMdioDownloadMode
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port
)
{
    if (mtdHwSetPhyRegField(contextPtr,port,31,0xF008,5,1,1) == MTD_OK)
    {
       /*  // mtdSpecialSoftwareReset() will not work here. Must use
        // the external hardware T-unit reset */
        return (mtdTunitHwReset(contextPtr,port));
    }
    else
    {
        return MTD_FAIL;
    }
}

MTD_STATUS mtdRemovePhyMdioDownloadMode
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port
)
{
    if (mtdHwSetPhyRegField(contextPtr,port,31,0xF008,5,1,0) == MTD_OK)
    {
       /*  // mtdSpecialSoftwareReset() will not work here. Must use
        // the external hardware T-unit reset */
        return (mtdTunitHwReset(contextPtr,port));
    }
    else
    {
        return MTD_FAIL;
    }
}

/****************************************************************************/
/*******************************************************************
 Firmware Version
 *******************************************************************/
/****************************************************************************/
MTD_STATUS mtdGetFirmwareVersion
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_U8 *major,
    OUT MTD_U8 *minor,
    OUT MTD_U8 *inc,
    OUT MTD_U8 *test)
{
    MTD_U16 reg_49169, reg_49170;

    ATTEMPT(mtdHwXmdioRead(contextPtr, port,1,49169,&reg_49169));

    *major = (reg_49169 & 0xFF00) >> 8;
    *minor = (reg_49169 & 0x00FF);

    ATTEMPT(mtdHwXmdioRead(contextPtr, port,1,49170,&reg_49170));

    *inc = (reg_49170 &0xFF00) >> 8;
    *test = (reg_49170 &0x00FF);

    if (!(*major || *minor || *inc || *test)) /* // firmware is not running if all 0's */
    {
        return MTD_FAIL;
    }
    return MTD_OK;
}

/****************************************************************************/

/****************************************************************************/
/*******************************************************************
   802.3 Clause 28 and Clause 45
   Autoneg Related Control & Status
 *******************************************************************/
/*******************************************************************
  Enabling speeds for autonegotiation
  Reading speeds enabled for autonegotation
  Set/get pause advertisement for autonegotiation
  Other Autoneg-related Control and Status (restart,disable/enable,
  force master/slave/auto, checking for autoneg resolution, etc.)
 *******************************************************************/

#define MTD_7_0010_SPEED_BIT_LENGTH 4
#define MTD_7_0010_SPEED_BIT_POS    5
#define MTD_7_8000_SPEED_BIT_LENGTH 2
#define MTD_7_8000_SPEED_BIT_POS    8
#define MTD_7_0020_SPEED_BIT_LENGTH 1
#define MTD_7_0020_SPEED_BIT_POS    12
#define MTD_7_0020_SPEED_BIT_LENGTH2 2   /* for 88X33X0/88X2180 family revision */
#define MTD_7_0020_SPEED_BIT_POS2    7

#define MTD_IS_X32X0_BASE(mTdrevId) ((MTD_BOOL)((mTdrevId & (0x3F<<4)) == MTD_X32X0_BASE))
/* Bit defines for speed bits */
#define MTD_FORCED_SPEEDS_BIT_MASK  (MTD_SPEED_10M_HD_AN_DIS | MTD_SPEED_10M_FD_AN_DIS | \
                                     MTD_SPEED_100M_HD_AN_DIS | MTD_SPEED_100M_FD_AN_DIS)
#define MTD_LOWER_BITS_MASK         0x000F /* bits in base page */
#define MTD_GIG_SPEED_POS           4
#define MTD_XGIG_SPEED_POS          6
#define MTD_2P5G_SPEED_POS          11
#define MTD_5G_SPEED_POS            12
#define MTD_GET_1000BT_BITS(_speedBits) ((_speedBits & 0x0030)>>MTD_GIG_SPEED_POS) /* 1000BT bits */
#define MTD_GET_10GBT_BIT(_speedBits) ((_speedBits & 0x0040)>>MTD_XGIG_SPEED_POS) /* 10GBT bit setting */
#define MTD_GET_2P5GBT_BIT(_speedBits) ((_speedBits & 0x0800)>>MTD_2P5G_SPEED_POS) /* 2.5GBT bit setting */
#define MTD_GET_5GBT_BIT(_speedBits) ((_speedBits & 0x1000)>>MTD_5G_SPEED_POS) /* 5GBT bit setting */
#define MTD_3340_REV_A0             0x09A2
#define MTD_3340_REV_A1             0x09A3
#define MTD_2180_REV_A0             0x09B0
/****************************************************************************/
/* Returns the  3x40 PHY type */
static MTD_U16 mtdGet3x40DeviceType(CTX_PTR_TYPE contextPtr, MTD_U16 port)
{
    MTD_U16 devType;
    MTD_U16 reg1_3;
    MTD_U16 manuf_model_num;
    MTD_U16 oui1,oui2;

    /* Check if this is a Marvell PHY */
    mtdHwXmdioRead(contextPtr,port,1, 2, &oui1);

    mtdHwXmdioRead(contextPtr,port,1, 3, &reg1_3);

    mtdHwGetRegFieldFromWord(reg1_3,10,6,&oui2);

    mtdHwGetRegFieldFromWord(reg1_3,4,6,&manuf_model_num);

    /* Marvell OUI is 0x05043, X3240 model num is 0x18 */
    if (oui1 == 0x141 && oui2 == 3 && manuf_model_num == 0x18)
    {
        devType = DEVICE_ID_88x3240;
    }
    else if (oui1 == 0x141 && oui2 == 3 && manuf_model_num == 0x1a)
    {
        devType = DEVICE_ID_88x3340;
    }
    else if (oui1 == 0x2B && oui2 == 2 && manuf_model_num == 0x1a)
    {
        devType = DEVICE_ID_88x3340;
    }
    else if (oui1 == 0x2B && oui2 ==2 && manuf_model_num == 0x1b)
    {
        /* 2180 PHY is a sub-family of 3340*/
        devType = DEVICE_ID_88x3340;
    }
    else
    {
        devType = 0x00;
        /*printf("MTL no device found..\n");*/
    }

    return devType;
}



MTD_STATUS mtdEnableSpeeds
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    IN MTD_U16 speed_bits,
    IN MTD_BOOL anRestart
)
{
    MTD_BOOL speedForced;
    MTD_U16 dummy,devType;
    MTD_U16 tempRegValue;
    MTD_U16 phyRev;

    if (speed_bits & MTD_FORCED_SPEEDS_BIT_MASK)
    {
        return MTD_FAIL; /* tried to force the speed, this function is for autonegotiation control */
    }


    /* Enable AN and set speed back to power-on default in case previously forced
       Only do it if forced, to avoid an extra/unnecessary soft reset */
    ATTEMPT(mtdGetForcedSpeed(contextPtr,port,&speedForced,&dummy));
    if (speedForced)
    {
        ATTEMPT(mtdUndoForcedSpeed(contextPtr,port,MTD_FALSE));
    }
    devType = mtdGet3x40DeviceType(contextPtr, port);
    if (speed_bits == MTD_ADV_NONE)
    {
       /*  // Set all speeds to be disabled
        // Take care of bits in 7.0010 (advertisement register, 10BT and 100BT bits) */
        ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0x0010,\
                MTD_7_0010_SPEED_BIT_POS, MTD_7_0010_SPEED_BIT_LENGTH, \
                0));

        /* // Take care of speed bits in 7.8000 (1000BASE-T speed bits) */
        ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0x8000,\
                MTD_7_8000_SPEED_BIT_POS, MTD_7_8000_SPEED_BIT_LENGTH, \
                0));
        if ( devType == DEVICE_ID_88x3240 )
        {
            /*  Now take care of bits in 7.0020 (10GBASE-T speed bit) */
            ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0x0020,MTD_7_0020_SPEED_BIT_POS, MTD_7_0020_SPEED_BIT_LENGTH,0));
        }
        else if ( devType == DEVICE_ID_88x3340)
        {
            /* Now take care of bits in 7.0020 (10GBASE-T, 2.5G, 5G speed bits) */
            ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0x0020,MTD_7_0020_SPEED_BIT_POS2, MTD_7_0020_SPEED_BIT_LENGTH2, 0));
        }
        else
            return MTD_FAIL;
    }
    else
    {
        /* // Take care of bits in 7.0010 (advertisement register, 10BT and 100BT bits) */
        ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0x0010,\
                MTD_7_0010_SPEED_BIT_POS, MTD_7_0010_SPEED_BIT_LENGTH, \
                (speed_bits & MTD_LOWER_BITS_MASK)));

       /*  // Take care of speed bits in 7.8000 (1000BASE-T speed bits) */
        ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0x8000,\
                MTD_7_8000_SPEED_BIT_POS, MTD_7_8000_SPEED_BIT_LENGTH, \
                MTD_GET_1000BT_BITS(speed_bits)));
      if (devType == DEVICE_ID_88x3240)
      {
           /*  // Now take care of bits in 7.0020 (10GBASE-T speed bit) */
          ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0x0020,\
                                      MTD_7_0020_SPEED_BIT_POS, MTD_7_0020_SPEED_BIT_LENGTH, \
          MTD_GET_10GBT_BIT(speed_bits)));

          if (MTD_GET_10GBT_BIT(speed_bits))
            {
                /* Set XNP on*/
                ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0,13,1,1));
            }
       }
       else if ( devType == DEVICE_ID_88x3340)
       {
           ATTEMPT(mtdHwXmdioRead(contextPtr,port,1,0x0003,&phyRev));
           if ( (MTD_3340_REV_A0 == phyRev) || (MTD_3340_REV_A1 ==
                phyRev) || (phyRev == MTD_2180_REV_A0))
           {
                /* Now take care of bits in 7.0020 (10GBASE-T first) */
                ATTEMPT(mtdHwXmdioRead(contextPtr,port,7,0x0020,&tempRegValue));

                ATTEMPT(mtdHwSetRegFieldToWord(tempRegValue,MTD_GET_10GBT_BIT(speed_bits),\
                    MTD_7_0020_SPEED_BIT_POS, MTD_7_0020_SPEED_BIT_LENGTH, \
                    &tempRegValue));

                ATTEMPT(mtdHwSetRegFieldToWord(tempRegValue,MTD_GET_2P5GBT_BIT(speed_bits),\
                    7, 1, \
                    &tempRegValue));

                ATTEMPT(mtdHwSetRegFieldToWord(tempRegValue,MTD_GET_5GBT_BIT(speed_bits),\
                    8, 1, \
                    &tempRegValue));

               /* Now write result back to 7.0020 */
               ATTEMPT(mtdHwXmdioWrite(contextPtr,port,7,0x0020,tempRegValue));
           }
           else
           {
                /* Now take care of bits in 7.0020 (10GBASE-T, 2.5G and 5G speed bits)*/
                ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0x0020,\
                    MTD_7_0020_SPEED_BIT_POS, MTD_7_0020_SPEED_BIT_LENGTH, \
                    MTD_GET_10GBT_BIT(speed_bits)));


                ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0x0020,\
                    11, 1, \
                    MTD_GET_2P5GBT_BIT(speed_bits)));

                ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0x0020,\
                    10, 1, \
                    MTD_GET_5GBT_BIT(speed_bits)));
           }
           if (MTD_GET_10GBT_BIT(speed_bits) || MTD_GET_2P5GBT_BIT(speed_bits) || MTD_GET_5GBT_BIT(speed_bits))
           {
               /* Set XNP on */
               ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0,13,1,1));
           }
       }
       else
           return MTD_FAIL;
    }

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(contextPtr, port) ||
                             mtdAutonegRestart(contextPtr, port)));
    }

    return MTD_OK;
}


MTD_STATUS mtdGetSpeedsEnabled
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *speed_bits
)
{
    MTD_U16 lower_speeds, gig_speeds, xgig_speed, hgig_speeds,devType;

    if (mtdHwGetPhyRegField(contextPtr, port, 7, 0x0010, MTD_7_0010_SPEED_BIT_POS,
            MTD_7_0010_SPEED_BIT_LENGTH, &lower_speeds) == MTD_FAIL)
    {
        *speed_bits = MTD_ADV_NONE;
        return MTD_FAIL;
    }

    if (mtdHwGetPhyRegField(contextPtr, port, 7, 0x8000, MTD_7_8000_SPEED_BIT_POS,
            MTD_7_8000_SPEED_BIT_LENGTH, &gig_speeds) == MTD_FAIL)
    {
        *speed_bits = MTD_ADV_NONE;
        return MTD_FAIL;
    }

    *speed_bits = (lower_speeds | (gig_speeds <<= MTD_GIG_SPEED_POS));

    devType = mtdGet3x40DeviceType(contextPtr, port);
    if (devType == DEVICE_ID_88x3240)
    {
        if (mtdHwGetPhyRegField(contextPtr, port, 7, 0x0020, MTD_7_0020_SPEED_BIT_POS,
            MTD_7_0020_SPEED_BIT_LENGTH, &xgig_speed) == MTD_FAIL)
        {
        *speed_bits = MTD_ADV_NONE;
        return MTD_FAIL;
        }

        *speed_bits |= (xgig_speed <<= MTD_XGIG_SPEED_POS);
    }
    else if (devType == DEVICE_ID_88x3340)
    {
        /* read all 10G speed bit from 7.0020 */
        if (mtdHwGetPhyRegField(contextPtr, port, 7, 0x0020, MTD_7_0020_SPEED_BIT_POS,
            MTD_7_0020_SPEED_BIT_LENGTH, &xgig_speed) == MTD_FAIL)
        {
            *speed_bits = MTD_ADV_NONE;
            return MTD_FAIL;
        }
        /* read all 2 speed bits (2.5G/5G) from 7.0020 */
        if (mtdHwGetPhyRegField(contextPtr,port, 7, 0x0020, MTD_7_0020_SPEED_BIT_POS2,
                MTD_7_0020_SPEED_BIT_LENGTH2, &hgig_speeds) == MTD_FAIL)
        {
            *speed_bits = MTD_ADV_NONE;
            return MTD_FAIL;
        }
        /* need to unpack speed bits 7.0020.12>>12 on 88X33X0 Z2 */
        *speed_bits |= ((hgig_speeds & (1<<0)) ? (1<<MTD_XGIG_SPEED_POS) : 0);

        /* need to unpack speed bits 7.0020.8:7>>7 on 88X33X0 Z2 */
        *speed_bits |= ((hgig_speeds & (1<<1)) ? (1<<MTD_2P5G_SPEED_POS) : 0);
        *speed_bits |= ((hgig_speeds & (1<<0)) ? (1<<MTD_5G_SPEED_POS) : 0);

    }
    else
        return MTD_FAIL;

    return MTD_OK;
}

MTD_STATUS mtdForceSpeed
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    IN MTD_U16 forcedSpeedBit
)
{
    MTD_U16 duplexBit;

    /* force the speed by setting the correct bits in 1.0.13/6 and the duplex bit */
    /* in 7.A000.4, order is important. speed bits must be set before AN is disabled */

    switch (forcedSpeedBit)
    {
        case MTD_SPEED_10M_HD_AN_DIS:
        case MTD_SPEED_10M_FD_AN_DIS:
            ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,13,1,0));
            ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,6,1,0));
            if (forcedSpeedBit == MTD_SPEED_10M_FD_AN_DIS)
            {
                duplexBit = 1;
            }
            else
            {
                duplexBit = 0;
            }
            ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0x8000,4,1,duplexBit));
            break;

        case MTD_SPEED_100M_HD_AN_DIS:
        case MTD_SPEED_100M_FD_AN_DIS:
            ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,13,1,1));
            ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,6,1,0));
            if (forcedSpeedBit == MTD_SPEED_100M_FD_AN_DIS)
            {
                duplexBit = 1;
            }
            else
            {
                duplexBit = 0;
            }
            ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,7,0x8000,4,1,duplexBit));
            break;

        default:
            return MTD_FAIL;
            break;
    }

    /* disable an by setting 7.0.12 = 0 */
    /* IMPORTANT, this has to be done after the speed bits are changed, AN is forced enabled in the phy if the speed bits are set to */
    /* 1G or 10G in 1.0 */
    ATTEMPT(mtdAutonegDisable(contextPtr,port));

    /* when speed bits are changed, T unit sw reset is required wait until phy is ready */
    ATTEMPT(mtdSoftwareReset(contextPtr,port,100));

    return MTD_OK;
}

MTD_STATUS mtdUndoForcedSpeed
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    IN MTD_BOOL anRestart
)
{
    ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,13,1,1));
    ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,6,1,1));
    /* when speed bits are changed, T unit sw reset is required, wait until phy is ready */
    ATTEMPT(mtdSoftwareReset(contextPtr,port,100));

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(contextPtr, port) ||
                             mtdAutonegRestart(contextPtr, port)));
    }

    return MTD_OK;
}


MTD_STATUS mtdGetForcedSpeed
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *speedIsForced,
    OUT MTD_U16 *forcedSpeed
)
{
    MTD_U16 val,bit0,bit1, forcedSpeedBits, duplexBit;
    MTD_BOOL anDisabled;

    *speedIsForced = MTD_FALSE;
    *forcedSpeed = MTD_ADV_NONE;

    /* check if 7.0.12 is 0 or 1 (disabled or enabled) */
    ATTEMPT(mtdHwGetPhyRegField(contextPtr,port,7,0,12,1,&val));

    (val) ? (anDisabled = MTD_FALSE) : (anDisabled = MTD_TRUE);

    if (anDisabled)
    {
        /* autoneg is disabled, see if it's forced to one of the speeds that work without AN */
        ATTEMPT(mtdHwGetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,6, 1,&bit0));
        ATTEMPT(mtdHwGetPhyRegField(contextPtr,port,MTD_TUNIT_IEEE_PMA_CTRL1,13, 1,&bit1));

        /* now read the duplex bit setting */
        ATTEMPT(mtdHwGetPhyRegField(contextPtr,port,7,0x8000,4, 1,&duplexBit));

        forcedSpeedBits = 0;
        forcedSpeedBits = bit0 | (bit1<<1);

        if (forcedSpeedBits == 0)
        {
            /* it's set to 10BT */
            if (duplexBit)
            {
                *speedIsForced = MTD_TRUE;
                *forcedSpeed = MTD_SPEED_10M_FD_AN_DIS;
            }
            else
            {
                *speedIsForced = MTD_TRUE;
                *forcedSpeed = MTD_SPEED_10M_HD_AN_DIS;
            }
        }
        else if (forcedSpeedBits == 2)
        {
            /* it's set to 100BT */
            if (duplexBit)
            {
                *speedIsForced = MTD_TRUE;
                *forcedSpeed = MTD_SPEED_100M_FD_AN_DIS;
            }
            else
            {
                *speedIsForced = MTD_TRUE;
                *forcedSpeed = MTD_SPEED_100M_HD_AN_DIS;
            }
        }
        /* else it's set to 1000BT or 10GBT which require AN to work */
    }

    return MTD_OK;
}


MTD_STATUS mtdSetPauseAdvertisement
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    IN MTD_U8 pauseType,
    IN MTD_BOOL anRestart
)
{
    /* sets/clears bits 11, 10 (A6,A5 in the tech bit field of 7.16) */
    ATTEMPT(mtdHwSetPhyRegField(contextPtr, port, 7, 0x0010, \
                        10, 2, (MTD_U16)pauseType));

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(contextPtr, port) ||
                             mtdAutonegRestart(contextPtr, port)));
    }

    return MTD_OK;
}

MTD_STATUS mtdGetLPAdvertisedPause
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_U8 *pauseBits
)
{
    MTD_U16 val;
    MTD_BOOL anStatusReady;

    /* Make sure AN is complete */
    ATTEMPT(mtdAutonegIsCompleted(contextPtr,port,&anStatusReady));

    if (anStatusReady == MTD_FALSE)
    {
        *pauseBits = MTD_CLEAR_PAUSE;
        return MTD_FAIL;
    }

    /* get bits 11, 10 (A6,A5 in the tech bit field of 7.19) */
    if (mtdHwGetPhyRegField(contextPtr, port, 7, 19,
         10, 2, &val) == MTD_FAIL)
    {
        *pauseBits = MTD_CLEAR_PAUSE;
        return MTD_FAIL;
    }

    *pauseBits = (MTD_U8)val;

    return MTD_OK;
}

MTD_STATUS mtdGetTxRxPauseResolution
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *pauseResolved,
    OUT MTD_BOOL *tx_pause_enabled,
    OUT MTD_BOOL *rx_pause_enabled
)
{
    MTD_U16 val;

    ATTEMPT(mtdAutonegIsSpeedDuplexResolutionDone(contextPtr,port,pauseResolved));

    if (*pauseResolved == MTD_FALSE)
    {
        /* AN hasn't completed far enough to determine this */
        return MTD_OK;
    }

    /* get bits 3.8008.9:8 */
    if (mtdHwGetPhyRegField(contextPtr, port, 3, 0x8008,
         8, 2, &val) == MTD_FAIL)
    {
        *tx_pause_enabled = *rx_pause_enabled = MTD_FALSE;
        return MTD_FAIL;
    }

    if (val & 0x1)
    {
        /* // Bit 8 was set */
        *rx_pause_enabled = MTD_TRUE;
    }
    else
    {
        *rx_pause_enabled = MTD_FALSE;
    }

    if (val & 0x2)
    {
       /*  // Bit 9 was set */
        *tx_pause_enabled = MTD_TRUE;
    }
    else
    {
        *tx_pause_enabled = MTD_FALSE;
    }

    return MTD_OK;
}


MTD_STATUS mtdAutonegRestart
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port
)
{
    /* set 7.0.9, restart AN */
    return (mtdHwSetPhyRegField(contextPtr,port,7,0,
             9, 1, 1));
}

MTD_STATUS mtdAutonegDisable
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port
)
{
    /* set 7.0.12=0, disable AN */
    return (mtdHwSetPhyRegField(contextPtr,port,7,0,
             12, 1, 0));
}

MTD_STATUS mtdAutonegEnable
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port
)
{
    /* set 7.0.12=1, enable AN */
    return (mtdHwSetPhyRegField(contextPtr,port,7,0,
             12, 1, 1));
}

MTD_STATUS mtdAutonegForceMaster
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    IN MTD_BOOL anRestart
)
{
    /* set bits 14 and 15 in register 7.32 (master, and manual m/s) both 1 */
    ATTEMPT(mtdHwSetPhyRegField(contextPtr,port, 7, 32, \
                            14, 2, 3));

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(contextPtr, port) ||
                             mtdAutonegRestart(contextPtr, port)));
    }

    return MTD_OK;

}

MTD_STATUS mtdAutonegForceSlave
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    IN MTD_BOOL anRestart
)
{
    /* set bits 14=0 and 15=1 (slave and manual m/s) in register 7.32 */
    ATTEMPT(mtdHwSetPhyRegField(contextPtr,port, 7, 32, \
                            14, 2, 2));

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(contextPtr, port) ||
                             mtdAutonegRestart(contextPtr, port)));
    }

    return MTD_OK;

}

MTD_STATUS mtdAutonegSetAutoMasterSlave
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    IN MTD_BOOL anRestart
)
{
    /* set bits 14=0 and 15=0 (slave and manual m/s) in register 7.32 */
    ATTEMPT(mtdHwSetPhyRegField(contextPtr,port, 7, 32, \
                            14, 2, 0));

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(contextPtr, port) ||
                             mtdAutonegRestart(contextPtr, port)));
    }

    return MTD_OK;
}

MTD_STATUS mtdAutonegSetMasterPreference
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    IN MTD_BOOL anRestart
)
{

    /* set bit 15=0 and bit 13=1 in 7.32 (manual m/s and multiport bit */
    /* making preference to be master */
    if ((mtdHwSetPhyRegField(contextPtr,port, 7, 32,
                            15, 1, 0) == MTD_FAIL) ||
        (mtdHwSetPhyRegField(contextPtr,port, 7, 32,
                            13, 1, 1) == MTD_FAIL))
    {
        return MTD_FAIL;
    }

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(contextPtr, port) ||
                             mtdAutonegRestart(contextPtr, port)));
    }

    return MTD_OK;

}

MTD_STATUS mtdAutonegSetSlavePreference
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    IN MTD_BOOL anRestart
)
{
    /* set bit 15=0 and bit 13=0 in 7.32 (manual m/s and multiport bit to single port */
    /* making preference to be slave */
    if ((mtdHwSetPhyRegField(contextPtr,port, 7, 32,
                            15, 1, 0) == MTD_FAIL) ||
        (mtdHwSetPhyRegField(contextPtr,port, 7, 32,
                            13, 1, 0) == MTD_FAIL))
    {
        return MTD_FAIL;
    }

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(contextPtr, port) ||
                             mtdAutonegRestart(contextPtr, port)));
    }

    return MTD_OK;

}


MTD_STATUS mtdAutonegIsResolutionMasterOrSlave
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_U8 *anMsResolution
)
{
    MTD_BOOL anSpeedDuplexDone, msBit, cfgFault, anStatusReady;

    *anMsResolution = MTD_AN_RESOLUTION_UNKNOWN;

    ATTEMPT(mtdAutonegIsSpeedDuplexResolutionDone(contextPtr, port, &anSpeedDuplexDone));

    if (anSpeedDuplexDone == MTD_TRUE)
    {
        /* now check if AN has completed */
        /* this will clear the config fault, so don't do this unless the speed/duplex are resolved */
        /* or do this after checking for a config fault if there's a possibility that happened */
        ATTEMPT(mtdAutonegIsCompleted(contextPtr,port,&anStatusReady));

        if (anStatusReady == MTD_TRUE)
        {
            ATTEMPT(mtdReadMasterOrSlaveBit(contextPtr,port, &msBit));

            (msBit) ? (*anMsResolution = MTD_AN_RESOLUTION_MASTER) : (*anMsResolution = MTD_AN_RESOLUTION_SLAVE);
        }
    }
    else
    {
        /* might not be done/completed because there's a config fault, check for that */
        ATTEMPT(mtdGetMasterSlaveConfigFault(contextPtr, port,&cfgFault));

        if (cfgFault == MTD_TRUE)
        {
            *anMsResolution = MTD_AN_RESOLUTION_FAULT;
        }
        else
        {
            *anMsResolution = MTD_AN_RESOLUTION_UNKNOWN;
        }
    }

    return MTD_OK;
}


MTD_STATUS mtdGetAutonegSpeedDuplexResolution
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *speedResolution
)
{
    MTD_U16 val, speed, speed2, duplex,devType;
    MTD_BOOL resDone;

    *speedResolution = MTD_ADV_NONE;

    /* check if AN is enabled */
    ATTEMPT(mtdHwGetPhyRegField(contextPtr, port, \
            7, 0, 12, 1, &val));

    if (val)
    {
        /* an is enabled, check if speed is resolved */
        ATTEMPT(mtdAutonegIsSpeedDuplexResolutionDone(contextPtr, port, &resDone));

        if (resDone)
        {
            ATTEMPT(mtdHwGetPhyRegField(contextPtr, port, \
                    3, 0x8008, 14, 2, &speed));

            ATTEMPT(mtdHwGetPhyRegField(contextPtr, port, \
                    3, 0x8008, 13, 1, &duplex));

            switch (speed)
            {
                case MTD_CU_SPEED_10_MBPS:
                    if (duplex)
                    {
                        *speedResolution = MTD_SPEED_10M_FD;
                    }
                    else
                    {
                        *speedResolution = MTD_SPEED_10M_HD;
                    }
                    break;
                case MTD_CU_SPEED_100_MBPS:
                    if (duplex)
                    {
                        *speedResolution = MTD_SPEED_100M_FD;
                    }
                    else
                    {
                        *speedResolution = MTD_SPEED_100M_HD;
                    }
                    break;
                case MTD_CU_SPEED_1000_MBPS:
                    if (duplex)
                    {
                        *speedResolution = MTD_SPEED_1GIG_FD;
                    }
                    else
                    {
                        *speedResolution = MTD_SPEED_1GIG_HD;
                    }
                    break;
            case MTD_CU_SPEED_10_GBPS: /* also MTD_CU_SPEED_NBT */
                    devType = mtdGet3x40DeviceType(contextPtr, port);
                    if (devType == DEVICE_ID_88x3240)
                    {
                        *speedResolution = MTD_SPEED_10GIG_FD; /* 10G has only full duplex, ignore duplex bit */
                    }
                    else if (devType == DEVICE_ID_88x3340)
                    {
                        ATTEMPT(mtdHwGetPhyRegField(contextPtr,port, 3, 0x8008, 2, 2, &speed2));

                        switch (speed2)
                        {
                            case MTD_CU_SPEED_NBT_10G:
                                *speedResolution = MTD_SPEED_10GIG_FD;
                                break;

                            case MTD_CU_SPEED_NBT_5G:
                                *speedResolution = MTD_SPEED_5GIG_FD;
                                break;

                            case MTD_CU_SPEED_NBT_2P5G:
                                *speedResolution = MTD_SPEED_2P5GIG_FD;
                                break;

                            default:
                                /* this is an error */
                                return MTD_FAIL;
                                break;
                        }
                    }
                    else
                        return MTD_FAIL;
                    break;
                default:
                    /* this is an error */
                    return MTD_FAIL;
                    break;
            }

        }

    }

    return MTD_OK;
}

/****************************************************************************/
MTD_STATUS mtdIsBaseTUp(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *speed,
    OUT MTD_BOOL *linkUp
)
{
    MTD_BOOL speedIsForced;
    MTD_U16 forcedSpeed, cuSpeed, cuLinkStatus;

    *linkUp = MTD_FALSE;
    *speed = MTD_ADV_NONE;

    /* first check if speed is forced to one of the speeds not requiring AN to train */
    ATTEMPT(mtdGetForcedSpeed(contextPtr,port,&speedIsForced, &forcedSpeed));

    if (speedIsForced)
    {
        /* check if the link is up at the speed it's forced to */
        ATTEMPT(mtdHwGetPhyRegField(contextPtr,port,3,0x8008,14,2,&cuSpeed));
        ATTEMPT(mtdHwGetPhyRegField(contextPtr,port,3,0x8008,10,1,&cuLinkStatus));

        switch (forcedSpeed)
        {
            case MTD_SPEED_10M_HD_AN_DIS:
            case MTD_SPEED_10M_FD_AN_DIS: /* might want to add checking the duplex to make sure there */
                                          /* is no duplex mismatch */
                if (cuSpeed == MTD_CU_SPEED_10_MBPS)
                {
                    *speed = forcedSpeed;
                }
                else
                {
                    *speed = MTD_SPEED_MISMATCH;
                }
                if (cuLinkStatus)
                {
                    *linkUp = MTD_TRUE;
                }
                break;

            case MTD_SPEED_100M_HD_AN_DIS:
            case MTD_SPEED_100M_FD_AN_DIS:/* might want to add checking the duplex to make sure there */
                                          /* is no duplex mismatch */
                if (cuSpeed == MTD_CU_SPEED_100_MBPS)
                {
                    *speed = forcedSpeed;
                }
                else
                {
                    *speed = MTD_SPEED_MISMATCH;
                }
                if (cuLinkStatus)
                {
                    *linkUp = MTD_TRUE;
                }
                break;

            default:
                return MTD_FAIL;
                break;
        }
    }
    else
    {
        /* must be going through AN */
        ATTEMPT(mtdGetAutonegSpeedDuplexResolution(contextPtr,port,speed));

        if (*speed != MTD_ADV_NONE)
        {
            /* check if the link is up at the speed it's AN to */
            ATTEMPT(mtdHwGetPhyRegField(contextPtr,port,3,0x8008,10,1,&cuLinkStatus));

            switch(*speed)
            {
                case MTD_SPEED_10M_HD:
                case MTD_SPEED_10M_FD:
                case MTD_SPEED_100M_HD:
                case MTD_SPEED_100M_FD:
                case MTD_SPEED_1GIG_HD:
                case MTD_SPEED_1GIG_FD:
                case MTD_SPEED_10GIG_FD:
                case MTD_SPEED_2P5GIG_FD:
                case MTD_SPEED_5GIG_FD:
                    if (cuLinkStatus)
                    {
                        *linkUp = MTD_TRUE;
                    }
                    break;
                default:
                    return MTD_FAIL;
                    break;
            }

        }
        /* else link is down, and AN is in progress, */
    }

    if (*speed == MTD_SPEED_MISMATCH)
    {
        return MTD_FAIL;
    }
    else
    {
        return MTD_OK;
    }

}

MTD_STATUS mtdMDIXControl
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    IN MTD_U16 MDIXOptions,
    IN MTD_BOOL swReset
)
{
    if (MDIXOptions != MTD_FORCE_MDI &&
        MDIXOptions != MTD_FORCE_MDIX &&
        MDIXOptions != MTD_AUTO_MDI_MDIX)
    {
        return MTD_FAIL;
    }

    ATTEMPT(mtdHwSetPhyRegField(contextPtr,port,3,\
                            0x8000,5,2,MDIXOptions));

    if(swReset)
    {
        ATTEMPT(mtdSoftwareReset(contextPtr,port,100)); /* this needs to be polled for 'done' before
                                                       issuing any other MDIO commands
                                                       it self clears when the reset is finished */
    }

    return MTD_OK;
}

MTD_STATUS mtdGetMDIXControl
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *MDIXOptions
)
{
    ATTEMPT(mtdHwGetPhyRegField(contextPtr,port,3,0x8000,5,2,MDIXOptions));

    return MTD_OK;
}

MTD_STATUS mtdGetMDIXResolution
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_U8 *MDIX
)
{
    MTD_BOOL anDone;
    MTD_U16 bitVal;

    ATTEMPT(mtdAutonegIsSpeedDuplexResolutionDone(contextPtr,port,&anDone));

    if (anDone == MTD_FALSE)
    {
        *MDIX = MTD_MDI_MDIX_NOT_RESOLVED;
        return MTD_OK;
    }

    ATTEMPT(mtdHwGetPhyRegField(contextPtr,port,3,0x8008,6,1,&bitVal));

    (bitVal) ? (*MDIX = MTD_MDIX_RESOLVED) : (*MDIX = MTD_MDI_RESOLVED);

    return MTD_OK;
}

/*********************************************************************/
MTD_STATUS mtdControlEmiCanceller
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    IN MTD_BOOL enableCanceller,
    IN MTD_BOOL anRestart
)
{
    if (enableCanceller)
    {
        ATTEMPT(mtdHwSetPhyRegField(contextPtr,port, \
                1,0xC000,8,1,0)); /* enables EMI canceller */
    }
    else
    {
        ATTEMPT(mtdHwSetPhyRegField(contextPtr,port, \
                1,0xC000,8,1,1)) /* disables EMI canceller */
    }

    if(anRestart)
    {
        return ((MTD_STATUS)(mtdAutonegEnable(contextPtr, port) ||
                             mtdAutonegRestart(contextPtr, port)));
    }

    return MTD_OK;

}

/*********************************************************************
 Local Functions: These functions check lower level status bits.
 It's preferred to call the higher level functions which use these,
 since these higher level functions make sure the data is available/
 ready before checking them. Also, some of these lower level functions
 clear latched status bits when the register is read.

 If you call these lower-level functions directly, you should be clear
 that calling them might mean that you need to check if the data
 is available, and keep track of latched bits in the same register that
 get cleared when you read a certain bit value. Take a look at the
 higher level function that uses them to get an idea of what bits have to
 be checked to see if the data is ready/available.
 *********************************************************************/

/******************************************************************************
 STATIC MTD_STATUS mtdAutonegIsSpeedDuplexResolutionDone
 (
     IN CTX_PTR_TYPE contextPtr,
     IN MTD_U16 port,
     OUT MTD_BOOL *anSpeedResolutionDone
 );

 Inputs:
    contextPtr - pointer to host's private data
    port - MDIO port address, 0-31

 Outputs:
    anSpeedResolutionDone - one of the following
         MTD_TRUE if speed/duplex is resolved
         MTD_FALSE if speed/duplex is not resolved

 Returns:
    MTD_OK or MTD_FAIL, if query was successful or not

 Description:
    Queries register 3.8008.11 Speed/Duplex resolved to see if autonegotiation
    is resolved or in progress. See note below. This function is only to be
    called if autonegotation is enabled and speed is not forced.

    anSpeedResolutionDone being MTD_TRUE, only indicates if AN has determined
    the speed and duplex bits in 3.8008, which will indicate what registers
    to read later for AN resolution after AN has completed.

 Side effects:
    None

 Notes/Warnings:
    If autonegotiation is disabled or speed is forced, this function returns
    MTD_TRUE.

******************************************************************************/
STATIC MTD_STATUS mtdAutonegIsSpeedDuplexResolutionDone
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *anSpeedResolutionDone
)
{
    MTD_U16 val;

    /* read speed/duplex resolution done bit in 3.8008 bit 11 */
    if (mtdHwGetPhyRegField(contextPtr, port,
            3, 0x8008, 11, 1, &val) == MTD_FAIL)
    {
        *anSpeedResolutionDone = MTD_FALSE;
        return MTD_FAIL;
    }

    (val)?(*anSpeedResolutionDone = MTD_TRUE):(*anSpeedResolutionDone = MTD_FALSE);

    return MTD_OK;
}

/******************************************************************************
 STATIC MTD_STATUS mtdAutonegIsCompleted
 (
     IN CTX_PTR_TYPE contextPtr,
     IN MTD_U16 port,
     OUT MTD_BOOL *anStatusReady
 );

 Inputs:
    contextPtr - pointer to host's private data
    port - MDIO port address, 0-31

 Outputs:
    anStatusReady - one of the following
         MTD_TRUE if AN status registers are available to be read (7.1, 7.33, 7.32769, etc.)
         MTD_FALSE if AN is not completed and AN status registers may contain old data

 Returns:
    MTD_OK or MTD_FAIL, if query was successful or not

 Description:
    Checks 7.1.5 for 1. If 1, returns MTD_TRUE. If not, returns MTD_FALSE. Many
    autonegotiation status registers are not valid unless AN has completed
    meaning 7.1.5 = 1.

 Side effects:
    None

 Notes/Warnings:
    Call this function before reading 7.33 or 7.32769 to check for master/slave
    resolution or other negotiated parameters which are negotiated during
    autonegotiation like fast retrain, fast retrain type, etc.

******************************************************************************/
STATIC MTD_STATUS mtdAutonegIsCompleted
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *anStatusReady
)
{
    MTD_U16 val;

    /* read an completed, 7.1.5 bit */
    if (mtdHwGetPhyRegField(contextPtr, port,
            7, 1, 5, 1, &val) == MTD_FAIL)
    {
        *anStatusReady = MTD_FALSE;
        return MTD_FAIL;
    }

    (val)?(*anStatusReady = MTD_TRUE):(*anStatusReady = MTD_FALSE);

    return MTD_OK;
}


/******************************************************************************
 MTD_STATUS mtdReadMasterOrSlaveBit
 (
     IN CTX_PTR_TYPE contextPtr,
     IN MTD_U16 port,
     OUT MTD_BOOL *msBitFlag
 );

 Inputs:
    contextPtr - pointer to host's private data
    port - MDIO port address, 0-31

 Outputs:
    msBitFlag - MTD_TRUE if local PHY is a Master
                MTD_FALSE if local PHY is a Slave

 Returns:
    MTD_OK or MTD_FAIL, if query was successful or not

 Description:
    Reads the master/slave configuration resolution bit in 7.33 (for 10GBT)
    or 7.32769 (for 1000BT) and returns 1 if the phy is a master or 0 if the
    phy is a slave.

    These bits are only valid if autonegotiation has completed (bit 7.1.5
    is 1).

 Side effects:
    This function reads 7.33 or 7.32769 (depending on speed resolution)
    and will clear the latched status of the
    master/slave configuration fault bit. Check for that before calling
    this function if that bit is of interest.

 Notes/Warnings:
    This function does not check if auto-negotiation is completed. Call
    mtdAutonegIsCompleted() before calling this function.

******************************************************************************/
STATIC MTD_STATUS mtdReadMasterOrSlaveBit
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *msBitFlag
)
{
    MTD_U16 val = MTD_FALSE, speed;

    *msBitFlag = MTD_FALSE;

    /* this functions assumes previously that an is "done" and no config fault occurred */

    /* check the speed resolution so we know which master/slave bit */
    /* to look at */

    ATTEMPT(mtdHwGetPhyRegField(contextPtr, port, \
            3, 0x8008, 14, 2, &speed));

    if (speed == 3)
    {
        /* speed is 10GBT */
        /* read 7.33.14, this will clear the LH bit in 7.33.15 */
        ATTEMPT(mtdHwGetPhyRegField(contextPtr, port,
                7, 33, 14, 1, &val));

    }
    else if (speed == 2)
    {
        /* speed is 1000BT */
        /* read 7.32769.14, this will clear the LH bit in 7.32769.15 */
        ATTEMPT(mtdHwGetPhyRegField(contextPtr, port,
                7, 32769, 14, 1, &val));

    }
    else
    {
        /* this is an error (it means the speed is 10BT or 100BT and reading this */
        /* bit makes no sense) */
        return MTD_FAIL;
    }

    (val) ? (*msBitFlag = MTD_TRUE) : (*msBitFlag = MTD_FALSE);

    return MTD_OK;
}


/******************************************************************************
 STATIC MTD_STATUS mtdGetMasterSlaveConfigFault
 (
     IN CTX_PTR_TYPE contextPtr,
     IN MTD_U16 port,
     OUT MTD_BOOL *msConfigFaultFlag
 );

 Inputs:
    contextPtr - pointer to host's private data
    port - MDIO port address, 0-31

 Outputs:
    msConfigFaultFlag - MTD_TRUE if master/slave config fault was detected,
                        MTD_FALSE otherwise

 Returns:
    MTD_OK or MTD_FAIL, if query was successful or not

 Description:
    Reads the master/slave config fault bit in the auto-negotiation register
    7.33/7.32769 and msConfigFaultFlag to MTD_TRUE if a config fault has occurred
    since the last time these registers were read. The returned value
    in msConfigFaultFlag is the logical OR of 7.33.15 and 7.32769.15.

 Side effects:
    This bit is a latched high bit and will get cleared when read.

 Notes/Warnings:
    This function is called in functions that find out if auto-negotiation has
    resolved master/slave, mtdAutonegIsResolutionMasterOrSlave(), to see if
    a mis-configuration of auto-negotiation has occurred.

    THE CONDITIONS FOR FAULT ARE :-
    both sides set to manual master
    both sides set to manual slave
    8 consecutive random seed attempts tied

    The situation where 8 consecutive random seeds matched normally corrects
    itself and should rarely happen.

******************************************************************************/
STATIC MTD_STATUS mtdGetMasterSlaveConfigFault
(
    IN CTX_PTR_TYPE contextPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *msConfigFaultFlag
)
{
    MTD_U16 val = MTD_FALSE, val2 = MTD_FALSE;

    *msConfigFaultFlag = MTD_FALSE;

    /* there's no "done" indication config fault */
    /* the bit in 7.33.15 is set if the resolution was 10GBT */
    /* the bit in 7.32769.15 is set if the resolution was 1000BT */

    /* read 7.33.15, this will clear this LH bit */
    ATTEMPT(mtdHwGetPhyRegField(contextPtr, port, \
            7, 33, 15, 1, &val));

    /* read 7.32769.15, this will clear this LH bit */
    ATTEMPT(mtdHwGetPhyRegField(contextPtr, port, \
            7, 32769, 15, 1, &val2));

    val |= val2; /* either bit being set means there was a config fault */

    (val) ? (*msConfigFaultFlag = MTD_TRUE) : (*msConfigFaultFlag = MTD_FALSE);

    return MTD_OK;
}


