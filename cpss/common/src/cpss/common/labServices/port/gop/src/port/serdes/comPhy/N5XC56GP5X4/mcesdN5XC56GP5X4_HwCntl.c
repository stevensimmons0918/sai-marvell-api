/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions and global data for interfacing with the
host's hardware-specific IO in order to control and query the Marvell
5FFP_COMPHY_56G_PIPE5_X4_4PLL

These functions as written were tested with a USB-interface to
access Marvell's EVB. These functions must be ported to
the host's specific platform.
********************************************************************/
#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "mcesdN5XC56GP5X4_Defs.h"
#include "mcesdN5XC56GP5X4_HwCntl.h"
#include "mcesdN5XC56GP5X4_RegRW.h"

#ifdef N5XC56GP5X4

MCESD_STATUS API_N5XC56GP5X4_HwWriteReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg,
    IN MCESD_U32 value
)
{
    if (devPtr->fmcesdWriteReg == NULL)
    {
        MCESD_DBG_ERROR("fmcesdWriteReg function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (devPtr->fmcesdWriteReg(devPtr, reg, value) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdWriteReg 0x%X failed to write to dev=C56GX4, reg=0x%X\n", value, reg);
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_HwReadReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg,
    OUT MCESD_U32 *data
)
{
    if (devPtr->fmcesdReadReg == NULL)
    {
        MCESD_DBG_ERROR("fmcesdReadReg function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (devPtr->fmcesdReadReg(devPtr, reg, data) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdReadReg failed from dev=C56GX4, reg=0x%X\n", reg);
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_HwSetPinCfg
(
    IN MCESD_DEV_PTR devPtr,
    IN E_N5XC56GP5X4_PIN pin,
    IN MCESD_U16 pinValue
)
{
    if (devPtr->fmcesdSetPinCfg == NULL)
    {
        MCESD_DBG_ERROR("fmcesdSetPinCfg function pointer is NULL\n");
        return MCESD_FAIL;
    }

#ifndef N5XC56GP5X4_ISOLATION
    if (devPtr->fmcesdSetPinCfg(devPtr, pin, pinValue) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdSetPinCfg 0x%X failed to configure dev=C56GX4, pin=0x%X\n", pinValue, pin);
        return MCESD_FAIL;
    }
#else
    switch (pin)
    {
    case N5XC56GP5X4_PIN_RX_INIT0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_INIT, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_INIT1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_INIT, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_INIT2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_INIT, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_INIT3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_INIT, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_IVREF:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_IVREF, 255 /* ignored */, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_TX0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_TX, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_TX1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_TX, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_TX2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_TX, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_TX3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_TX, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_RX0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_RX, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_RX1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_RX, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_RX2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_RX, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_RX3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_RX, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_IDLE0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_IDLE, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_IDLE1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_IDLE, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_IDLE2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_IDLE, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_IDLE3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_IDLE, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_PLL0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_PLL, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_PLL1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_PLL, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_PLL2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_PLL, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_PU_PLL3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PU_PLL, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_PHY_MODE:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PHY_MODE, 255 /* ignored */, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFFREF_TX0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_TX, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFFREF_TX1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_TX, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFFREF_TX2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_TX, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFFREF_TX3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_TX, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFFREF_RX0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_RX, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFFREF_RX1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_RX, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFFREF_RX2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_RX, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFFREF_RX3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_RX, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFCLK_TX0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TS_REFCLK_SEL, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFCLK_TX1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TS_REFCLK_SEL, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFCLK_TX2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TS_REFCLK_SEL, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFCLK_TX3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TS_REFCLK_SEL, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFCLK_RX0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RS_REFCLK_SEL, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFCLK_RX1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RS_REFCLK_SEL, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFCLK_RX2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RS_REFCLK_SEL, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_REFCLK_RX3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RS_REFCLK_SEL, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_TX0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_TX, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_TX1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_TX, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_TX2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_TX, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_TX3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_TX, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_RX0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_RX, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_RX1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_RX, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_RX2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_RX, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_RX3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_RX, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_MCU_CLK:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_MCU_FREQ, 255 /* ignored */, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_TRAINEN0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_TRAIN_ENA, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_TRAINEN1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_TRAIN_ENA, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_TRAINEN2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_TRAIN_ENA, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_TRAINEN3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_TRAIN_ENA, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_TRAINEN0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_TRAIN_ENA, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_TRAINEN1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_TRAIN_ENA, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_TRAINEN2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_TRAIN_ENA, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_TRAINEN3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_TRAIN_ENA, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_AVDD_SEL:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_AVDD_SEL, 255 /* ignored */, pinValue);
        break;
    case N5XC56GP5X4_PIN_PIPE_SEL:
        /* PHY ISOLATION must already be in SERDES Interface */
        break;
    case N5XC56GP5X4_PIN_SPD_CFG:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_SPD_CFG, 255 /* ignored */, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_GRAY_CODE0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_GRAY_EN, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_GRAY_CODE1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_GRAY_EN, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_GRAY_CODE2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_GRAY_EN, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_GRAY_CODE3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TX_GRAY_EN, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_GRAY_CODE0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_GRAY_EN, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_GRAY_CODE1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_GRAY_EN, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_GRAY_CODE2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_GRAY_EN, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_GRAY_CODE3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RX_GRAY_EN, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_PRE_CODE0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TXDATA_PRECODE_EN, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_PRE_CODE1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TXDATA_PRECODE_EN, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_PRE_CODE2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TXDATA_PRECODE_EN, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_TX_PRE_CODE3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_TXDATA_PRECODE_EN, 3, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_PRE_CODE0:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RXDATA_PRECODE_EN, 0, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_PRE_CODE1:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RXDATA_PRECODE_EN, 1, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_PRE_CODE2:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RXDATA_PRECODE_EN, 2, pinValue);
        break;
    case N5XC56GP5X4_PIN_RX_PRE_CODE3:
        N5XC56GP5X4_WRITE_FIELD(devPtr, F_N5XC56GP5X4_RXDATA_PRECODE_EN, 3, pinValue);
        break;
    default:
        /* Pins with no isolation field mapping */
        if (devPtr->fmcesdSetPinCfg(devPtr, pin, pinValue) == MCESD_FAIL)
        {
            MCESD_DBG_INFO("fmcesdSetPinCfg 0x%X failed to configure dev=C112GX4, pin=0x%X\n", pinValue, pin);
            return MCESD_FAIL;
        }
    }
#endif

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_HwGetPinCfg
(
    IN MCESD_DEV_PTR devPtr,
    IN E_N5XC56GP5X4_PIN pin,
    OUT MCESD_U16 *pinValue
)
{
#ifndef N5XC56GP5X4_ISOLATION
    if (devPtr->fmcesdGetPinCfg == NULL)
    {
        MCESD_DBG_ERROR("fmcesdGetPinCfg function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (devPtr->fmcesdGetPinCfg(devPtr, pin, pinValue) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdGetPinCfg failed from dev=C56GX4, pin=0x%X\n", pin);
        return MCESD_FAIL;
    }
#else
    MCESD_U32 data;

    if (devPtr->fmcesdGetPinCfg == NULL)
    {
        MCESD_DBG_ERROR("fmcesdGetPinCfg function pointer is NULL\n");
        return MCESD_FAIL;
    }

    switch (pin)
    {
    case N5XC56GP5X4_PIN_RX_INITDON0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_INIT_DONE, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_INITDON1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_INIT_DONE, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_INITDON2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_INIT_DONE, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_INITDON3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_INIT_DONE, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_IVREF:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_IVREF, 255 /* ignored */, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_TX0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_TX, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_TX1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_TX, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_TX2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_TX, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_TX3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_TX, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_RX0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_RX, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_RX1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_RX, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_RX2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_RX, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_RX3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_RX, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_IDLE0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_IDLE, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_IDLE1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_IDLE, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_IDLE2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_IDLE, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_IDLE3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_IDLE, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_PLL0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_PLL, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_PLL1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_PLL, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_PLL2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_PLL, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PU_PLL3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PU_PLL, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PHY_MODE:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PHY_MODE, 255 /* ignored */, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFFREF_TX0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_TX, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFFREF_TX1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_TX, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFFREF_TX2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_TX, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFFREF_TX3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_TX, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFFREF_RX0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_RX, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFFREF_RX1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_RX, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFFREF_RX2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_RX, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFFREF_RX3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_REF_FREF_RX, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFCLK_TX0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TS_REFCLK_SEL, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFCLK_TX1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TS_REFCLK_SEL, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFCLK_TX2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TS_REFCLK_SEL, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFCLK_TX3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TS_REFCLK_SEL, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFCLK_RX0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RS_REFCLK_SEL, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFCLK_RX1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RS_REFCLK_SEL, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFCLK_RX2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RS_REFCLK_SEL, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_REFCLK_RX3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RS_REFCLK_SEL, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_TX0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_TX, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_TX1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_TX, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_TX2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_TX, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_TX3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_TX, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_RX0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_RX, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_RX1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_RX, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_RX2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_RX, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_PHY_GEN_RX3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_PHY_GEN_RX, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_MCU_CLK:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_MCU_FREQ, 255 /* ignored */, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_TRAINCO0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_TRAIN_COM, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_TRAINCO1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_TRAIN_COM, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_TRAINCO2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_TRAIN_COM, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_TRAINCO3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_TRAIN_COM, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_TRAINCO0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_TRAIN_COM, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_TRAINCO1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_TRAIN_COM, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_TRAINCO2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_TRAIN_COM, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_TRAINCO3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_TRAIN_COM, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_TRAINFA0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_TRAIN_FAI, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_TRAINFA1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_TRAIN_FAI, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_TRAINFA2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_TRAIN_FAI, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_TRAINFA3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_TRAIN_FAI, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_TRAINFA0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_TRAIN_FAI, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_TRAINFA1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_TRAIN_FAI, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_TRAINFA2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_TRAIN_FAI, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_TRAINFA3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_TRAIN_FAI, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_SQ_DET_LPF0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_SQ_OUT, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_SQ_DET_LPF1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_SQ_OUT, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_SQ_DET_LPF2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_SQ_OUT, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_SQ_DET_LPF3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_SQ_OUT, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_GRAY_CODE0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_GRAY_EN, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_GRAY_CODE1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_GRAY_EN, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_GRAY_CODE2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_GRAY_EN, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_GRAY_CODE3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TX_GRAY_EN, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_GRAY_CODE0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_GRAY_EN, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_GRAY_CODE1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_GRAY_EN, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_GRAY_CODE2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_GRAY_EN, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_GRAY_CODE3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RX_GRAY_EN, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_PRE_CODE0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TXDATA_PRECODE_EN, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_PRE_CODE1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TXDATA_PRECODE_EN, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_PRE_CODE2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TXDATA_PRECODE_EN, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_TX_PRE_CODE3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_TXDATA_PRECODE_EN, 3, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_PRE_CODE0:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RXDATA_PRECODE_EN, 0, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_PRE_CODE1:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RXDATA_PRECODE_EN, 1, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_PRE_CODE2:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RXDATA_PRECODE_EN, 2, data);
        *pinValue = data;
        break;
    case N5XC56GP5X4_PIN_RX_PRE_CODE3:
        N5XC56GP5X4_READ_FIELD(devPtr, F_N5XC56GP5X4_RXDATA_PRECODE_EN, 3, data);
        *pinValue = data;
        break;
    default:
        /* Pins with no isolation field mapping */
        if (devPtr->fmcesdGetPinCfg(devPtr, pin, pinValue) == MCESD_FAIL)
        {
            MCESD_DBG_INFO("fmcesdGetPinCfg failed from dev=C112GX4, pin=0x%X\n", pin);
            return MCESD_FAIL;
        }
    }
#endif

    return MCESD_OK;
}

MCESD_STATUS API_N5XC56GP5X4_Wait
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 ms
)
{
    if (devPtr->fmcesdWaitFunc == NULL)
    {
        MCESD_DBG_ERROR("fmcesdWaitFunc function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (devPtr->fmcesdWaitFunc(devPtr, ms) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdWaitFunc failed from dev=C56GX4\n");
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

#endif /* N5XC56GP5X4 */