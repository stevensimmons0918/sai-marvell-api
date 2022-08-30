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
* @file prvCpssPxHwRegAddrVer1_ppDb.c
*
* @brief This file includes definitions of DB of units base addresses.
*
* @version   1
********************************************************************************
*/
#include <cpss/px/cpssHwInit/private/prvCpssPxHwRegAddrVer1_ppDb.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/* --------------------------------------------------*
 *  Unit base address definition                     *
 *---------------------------------------------------*/
#define NON_VALID_ADDR_CNS (GT_U32)(~0)
typedef struct
{
    PRV_CPSS_DXCH_UNIT_ENT unitId;
    GT_U32                 unitBaseAdrr;  /* last line shall be  { PRV_CPSS_DXCH_UNIT_LAST_E , NON_VALID_ADDR_CNS } */
}PRV_CPSS_PX_UNITID_UNITBASEADDR_STC;


/*--------------------------------------------------------------*
 * Unit base addresses Data Base for PIPE                       *
 *--------------------------------------------------------------*/
static PRV_CPSS_PX_UNITID_UNITBASEADDR_STC prvPxUnitsIdUnitBaseAddrArr[] =
{
     { PRV_CPSS_DXCH_UNIT_MG_E,             0x00000000       }
    ,{ PRV_CPSS_DXCH_UNIT_RXDMA_E,          0x01000000       }
    ,{ PRV_CPSS_DXCH_UNIT_TXDMA_E,          0x02000000       }
    ,{ PRV_CPSS_DXCH_UNIT_TX_FIFO_E,        0x04000000       }
    /*UNIT_SBC      0x06000000*/
    ,{ PRV_CPSS_DXCH_UNIT_CNC_0_E,          0x07000000       }
    ,{ PRV_CPSS_DXCH_UNIT_LED_0_E,          0x09000000       }
    ,{ PRV_CPSS_DXCH_UNIT_TAI_E,            0x0a000000       }
    ,{ PRV_CPSS_DXCH_UNIT_SMI_0_E,          0x0b000000       }
    ,{ PRV_CPSS_PX_UNIT_PCP_E,              0x0e000000       }
    ,{ PRV_CPSS_PX_UNIT_PHA_E,              0x0f000000       }

    ,{ PRV_CPSS_DXCH_UNIT_GOP_E,            0x10000000       }
    ,{ PRV_CPSS_DXCH_UNIT_MIB_E,            0x12000000       }
    ,{ PRV_CPSS_DXCH_UNIT_SERDES_E,         0x13000000       }

    ,{ PRV_CPSS_DXCH_UNIT_MPPM_E,           0x15000000       }
    ,{ PRV_CPSS_DXCH_UNIT_BM_E,             0x17000000       }
    ,{ PRV_CPSS_DXCH_UNIT_BMA_E,            0x18000000       }

    ,{ PRV_CPSS_DXCH_UNIT_TXQ_PFC_E,        0x1a000000       }
    ,{ PRV_CPSS_PX_UNIT_MCFC_E,             0x1b000000       }

    ,{ PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,      0x1e000000       }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_LL_E,         0x1f000000       }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_QCN_E,        0x20000000       }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,         0x21000000       }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E,        0x22000000       }
    ,{ PRV_CPSS_DXCH_UNIT_TXQ_BMX_E,        0x23000000       }

    /***************************/
    ,{ PRV_CPSS_DXCH_UNIT_LAST_E,               NON_VALID_ADDR_CNS  }
};

GT_U32 prvPxUnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1];  /* unit base address array */

/**
* @internal prvCpssPxUnitBaseTableInit function
* @endinternal
*
* @brief   init based address lists for all ASICs
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       GT_OK on success
*/
static GT_STATUS prvCpssPxUnitBaseTableInit
(
    GT_VOID
)
{
    GT_U32 unitIdx;
    PRV_CPSS_DXCH_UNIT_ENT unitType;
    PRV_CPSS_PX_UNITID_UNITBASEADDR_STC   *idDbPtr;
    GT_U32  *baseAddrDbPtr;

    idDbPtr = &prvPxUnitsIdUnitBaseAddrArr[0];
    baseAddrDbPtr =  &prvPxUnitsBaseAddr[0];

    if(baseAddrDbPtr[PRV_CPSS_DXCH_UNIT_LAST_E] == END_OF_TABLE_INDICATOR_CNS)
    {
        /*already initialized */
        return GT_OK;
    }

    /*---------------------------------------------------------------------------------*
     *init default value                                                               *
     *  all but last (0:PRV_CPSS_DXCH_UNIT_LAST_E-1) --    NON_VALID_BASE_ADDR_CNS     *
     *  last(idx = PRV_CPSS_DXCH_UNIT_LAST_E)        --    END_OF_TABLE_INDICATOR_CNS  *
     *---------------------------------------------------------------------------------*/
    for (unitIdx = 0 ; unitIdx < PRV_CPSS_DXCH_UNIT_LAST_E + 1 ; unitIdx++)
    {
        baseAddrDbPtr[unitIdx] = NON_VALID_BASE_ADDR_CNS;
    }
    baseAddrDbPtr[PRV_CPSS_DXCH_UNIT_LAST_E] = END_OF_TABLE_INDICATOR_CNS;

    /*--------------------------------------------------*
     *  init specific values : from init list           *
     *--------------------------------------------------*/
    for (unitIdx = 0 ; idDbPtr[unitIdx].unitId != PRV_CPSS_DXCH_UNIT_LAST_E ; unitIdx++)
    {
        unitType                = idDbPtr[unitIdx].unitId;
        baseAddrDbPtr[unitType] = idDbPtr[unitIdx].unitBaseAdrr;
    }

    return GT_OK;
}



/**
* @internal prvCpssPxUnitBaseTableGet function
* @endinternal
*
* @brief   get based address lists for all ASICs
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @param[out] prvPxUnitsBaseAddrPtrPtr - ptr to point to table
*                                       GT_OK on success
*/
GT_STATUS prvCpssPxUnitBaseTableGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32 **prvPxUnitsBaseAddrPtrPtr
)
{
    GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(prvPxUnitsBaseAddrPtrPtr);

    rc = prvCpssPxUnitBaseTableInit();
    if (rc != GT_OK)
    {
        return rc;
    }

    *prvPxUnitsBaseAddrPtrPtr = prvPxUnitsBaseAddr;

    return GT_OK;
}










