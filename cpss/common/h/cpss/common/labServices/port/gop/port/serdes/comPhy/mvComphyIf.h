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
* mvComphymIf.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 15 $
*
*******************************************************************************/

#ifndef __mvComphyIf_H
#define __mvComphyIf_H

/* General H Files */
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>

#include <../src/cpss/common/labServices/port/gop/src/port/serdes/comPhy/mcesdTop.h>
#include <../src/cpss/common/labServices/port/gop/src/port/serdes/comPhy/mcesdApiTypes.h>
#include <../src/cpss/common/labServices/port/gop/src/port/serdes/comPhy/mcesdUtils.h>
#include <../src/cpss/common/labServices/port/gop/src/port/serdes/comPhy/mcesdInitialization.h>

#ifdef __cplusplus
extern "C" {
#endif


#define MV_HWS_SERDES_TXRX_OVERRIDE_TUNE_PARAMS_ARR_SIZE       1
#define HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(val, defVal, overrideVal)\
        val = ((overrideVal == NA_8BIT) ?  defVal : overrideVal)

#define HWS_COMPHY_CHECK_AND_SET_DB_16BIT_SIGNED_VAL(val, defVal, overrideVal)\
        val = ((overrideVal == NA_16BIT_SIGNED) ?  defVal : overrideVal)

#define HWS_COMPHY_GET_SERDES_DATA_FROM_SERDES_INFO(_serdesInfoPtr) ((MV_HWS_COMPHY_SERDES_DATA*)((_serdesInfoPtr->serdesData.comphySerdesData)))

#define HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray,funcPtr,serdesType,funcImplementation) \
        funcPtrArray[serdesType]->funcPtr = funcImplementation;

#define HWS_COMPHY_DEBUG_PRINTS 0

#if HWS_COMPHY_DEBUG_PRINTS
#define  comphyPrintf(...) osPrintf(__VA_ARGS__)
#else
#define  comphyPrintf(...)
#endif

#define  osPrintfIf(printBool,...) if (printBool) cpssOsPrintf(__VA_ARGS__)

#define MCESD_CHECK_STATUS(origFunc) CHECK_STATUS((origFunc==MCESD_OK)? GT_OK : GT_FAIL)
#define MV_HWS_COMPHY_FIRMWARE_NR                       0xff
#define MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE      0
#define MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE     1
#define MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE          25
#define MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE         156

#define PLL_INIT_POLLING_RETRIES 50
#ifdef AC5_DEV_SUPPORT
#define PLL_INIT_POLLING_DELAY   1
#else
#define PLL_INIT_POLLING_DELAY   10
#endif
#define PLL_INIT_POLLING_DONE    0xC

#define DFE_RESET_DELAY 1

#define MV_HWS_COMPHY_C28G_SELECTED_FIELDS_ARR_SIZE 55

#if defined MV_HWS_FREE_RTOS
#define GT_UNUSED_PARAM(x) (void)x
#endif

#ifdef  MV_HWS_REDUCED_BUILD
#define HOST_CONST
#else
#define HOST_CONST const
#endif

typedef enum
{
    MV_HWS_COMPHY_MCESD_ENUM_SERDES_SPEED,
    MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_FREQ,
    MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_SOURCE,
    MV_HWS_COMPHY_MCESD_ENUM_DATA_BUS_WIDTH,
    MV_HWS_COMPHY_MCESD_ENUM_PATTERN,
    MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK,

    MV_HWS_COMPHY_MCESD_ENUM_TYPE_LAST
} MV_HWS_COMPHY_MCESD_ENUM_TYPE;

/**
* @struct MV_HWS_COMPHY_REG_FIELD_STC
* @endinternal
*
* @brief   Register field
*
*/
typedef struct
{
    GT_U32 regOffset;
    GT_U8  fieldStart;
    GT_U8  fieldLen;
} MV_HWS_COMPHY_REG_FIELD_STC;

/**
* @struct MV_HWS_COMPHY_SERDES_DATA
* @endinternal
*
* @brief   Comphy SerDes Data
*
*/
typedef struct _MV_HWS_COMPHY_SERDES_DATA
{
    const MV_HWS_COMPHY_REG_FIELD_STC         *pinToRegMap;
    MCESD_DEV_PTR                             sDev; /* driver object */
    const MV_HWS_SERDES_TXRX_TUNE_PARAMS      *tuneParams;
    MV_HWS_SERDES_TXRX_TUNE_PARAMS            *tuneOverrideParams;
}MV_HWS_COMPHY_SERDES_DATA;

/**
* @internal mvHwsComphyConvertHwsToMcesdType function
* @endinternal
*
* @brief   Convert type from HWS to MCESD
*
* @param[in] devNum                   - system device number
* @param[in] serdesType               - serdesType (Comphy only)
* @param[in] enumType                 - enum type to convert
* @param[in] hwsEnum                  - source hws to convert
* @param[out] mcesdEnum               - dest mcesd to convert
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyConvertHwsToMcesdType
(
    IN  GT_U8                            devNum,
    IN  MV_HWS_SERDES_TYPE               serdesType,
    IN  MV_HWS_COMPHY_MCESD_ENUM_TYPE    enumType,
    IN  GT_U32                           hwsEnum,
    OUT GT_U32                           *mcesdEnum
);

GT_STATUS mvHwsComphyC112GX4SerdesFirmwareDownload
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     *serdesArr,
    GT_U8       numOfSerdeses,
    GT_U8       firmwareSelector
);

/**
* @internal mvHwsComphyConvertMcesdToHwsType function
* @endinternal
*
* @brief   Convert type from MCESD to HWS
*
* @param[in] devNum                   - system device number
* @param[in] serdesType               - serdesType (Comphy only)
* @param[in] enumType                 - enum type to convert
* @param[in] mcesdEnum                - source mcesd to convert
* @param[out] hwsEnum                 - dest hws to convert
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyConvertMcesdToHwsType
(
    IN  GT_U8                            devNum,
    IN  MV_HWS_SERDES_TYPE               serdesType,
    IN  MV_HWS_COMPHY_MCESD_ENUM_TYPE    enumType,
    IN  GT_U32                           mcesdEnum,
    OUT GT_U32                           *hwsEnum
);

/**
* @internal mvHwsComphySerdesRegisterWrite function
* @endinternal
*
* @brief   Write to SerDes's internal register
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  regAddr   - register address 
*                           for 12G :
*                           regAddr = (cider_base_address - 0x1000) / 4
* @param[in]  mask      - mask
* @param[in]  data      - value to set
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphySerdesRegisterWrite
(
    IN  GT_U8   devNum,
    IN  GT_UOPT portGroup,
    IN  GT_UOPT serdesNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  mask,
    IN  GT_U32  data
);

/**
* @internal mvHwsComphySerdesRegisterRead function
* @endinternal
*
* @brief   Read from SerDes's internal register
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  regAddr   - register address 
*                           for 12G :
*                           regAddr = (cider_base_address - 0x1000) / 4
* @param[in]  mask      - mask
* @param[out] data      - read value - can be NULL
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphySerdesRegisterRead
(
    IN  GT_U8   devNum,
    IN  GT_UOPT portGroup,
    IN  GT_UOPT serdesNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  mask,
    OUT GT_U32  *data
);

/**
* @internal mvHwsComphySerdesRegisterWriteField function
* @endinternal
*
* @brief   Write to SerDes's internal register
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  regAddr   - register address 
*                           for 12G :
*                           regAddr = (cider_base_address - 0x1000) / 4
* @param[in]  fieldOffset - field's start bit number
* @param[in]  fieldLength - field's length
* @param[in]  data      - value to set
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphySerdesRegisterWriteField
(
    IN  GT_U8   devNum,
    IN  GT_UOPT portGroup,
    IN  GT_UOPT serdesNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    IN  GT_U32  data
);

/**
* @internal mvHwsComphySerdesRegisterReadField function
* @endinternal
*
* @brief   Reads from SerDes's internal register
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  regAddr   - register address 
*                           for 12G :
*                           regAddr = (cider_address - 0x1000) / 4
* @param[in]  fieldOffset - field's start bit number
* @param[in]  fieldLength - field's length
* @param[out] data      - read value - can be NULL
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphySerdesRegisterReadField
(
    IN  GT_U8   devNum,
    IN  GT_UOPT portGroup,
    IN  GT_UOPT serdesNum,
    IN  GT_U32  regAddr,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    OUT GT_U32  *data
);

/**
* @internal mvHwsComphySerdesPinSet function
* @endinternal
*
* @brief   Set SerDes pin
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  pinNum    - pin number
* @param[in]  pinVal    - value to set
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphySerdesPinSet
(
    IN  GT_U8   devNum,
    IN  GT_UOPT portGroup,
    IN  GT_UOPT serdesNum,
    IN  GT_U16  pinNum,
    IN  GT_U16  pinVal
);

/**
* @internal mvHwsComphySerdesPinSet function
* @endinternal
*
* @brief   Set SerDes pin
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  pinNum    - pin number
* @param[out]  pinVal   - pin value
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphySerdesPinGet
(
    IN  GT_U8   devNum,
    IN  GT_UOPT portGroup,
    IN  GT_UOPT serdesNum,
    IN  GT_U16  pinNum,
    OUT GT_U16  *pinVal
);

GT_STATUS mvHwsComphyC12GP41P2VIfInit (IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray);
GT_STATUS mvHwsComphyC28GP4X1IfInit   (IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray);
GT_STATUS mvHwsComphyC28GP4X2IfInit   (IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray);
GT_STATUS mvHwsComphyC28GP4X4IfInit   (IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray);
GT_STATUS mvHwsComphyC112GX4IfInit    (IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray);
GT_STATUS mvHwsComphyN5XC56GP5X4IfInit(IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray);

/**
* @internal mvHwsComphyIfInit function
* @endinternal
*
* @brief   Init Comphy Serdes IF functions.
*
* @param[in] devNum                   - system device number
* @param[in] serdesType               - serdesType (Comphy only)
*                           use SERDES_LAST to init all Comphy types
* @param[in] funcPtrArray             - array for function registration
* @param[in] firmwareVersion   - FW version to download
* @param[in] callbackInitOnly - if GT_TRUE only initialize function pointers.

*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyIfInit
(
    IN GT_U8                   devNum,
    IN MV_HWS_SERDES_TYPE      serdesType,
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray,
    IN GT_U8                   firmwareVersion,
    IN GT_BOOL                 callbackInitOnly
);


/**
* @internal mvHwsComphySerdesIndexInfoSDevGet function
* @endinternal
*
* @brief   get serdesInfo, serdesIndex and sDev.
*
* @param[in]  devNum                  - system device number
* @param[in]  serdesNum               - serdesNum
* @param[out] serdesInfo              - (pointer to) serdesInfo
* @param[out] serdesIndex             - (pointer to) serdesIndex
* @param[out] serdesInfo              - (pointer to) sDev
*
* @retval 0                           - on success
* @retval 1                           - on error
*/
GT_STATUS mvHwsComphySerdesIndexInfoSDevGet
(
    IN  GT_U8                      devNum,
    IN  GT_UOPT                    serdesNum,
    OUT MV_HWS_PER_SERDES_INFO_PTR *serdesInfoPtr,
    OUT GT_U32                     *serdesIndexPtr,
    OUT MCESD_DEV_PTR              *sDevPtr

);

/**
* @internal mvHwsComphyC12GP41P2VSerdesCheckAccess function
* @endinternal
*
* @brief   Check SerDes is accessible
*
* @param[in]  devNum                  - system device number
* @param[in]  portGroup               - port group (core) number
* @param[in]  serdesNum               - serdesNum
*
* @retval 0                           - on success
* @retval 1                           - on error
*/
GT_STATUS mvHwsComphyC12GP41P2VSerdesCheckAccess
(
    IN  GT_U8      devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum
);

/**
* @internal mvHwsComphySerdesManualTxDBSet function
* @endinternal
*
* @brief   set override param DB
*
* @param[in]  devNum                  - system device number
* @param[in]  portGroup               - port group (core) number
* @param[in]  phyPortNum              - port number
* @param[in]  serdesSpeed             - serdes speed
* @param[in]  serdesNum               - serdesNum
* @param[in]  *txComphyConfigPtr      - tx override prameters ptr
*
* @retval 0                           - on success
* @retval 1                           - on error
*/
GT_STATUS mvHwsComphySerdesManualTxDBSet
(
    IN  GT_U8                                   devNum,
    IN  GT_UOPT                                 portGroup,
    IN  GT_U32                                  phyPortNum,
    IN  MV_HWS_SERDES_SPEED                     serdesSpeed,
    IN  GT_UOPT                                 serdesNum,
    IN  MV_HWS_SERDES_TX_CONFIG_DATA_UNT        *txComphyConfigPtr
);

/**
* @internal mvHwsComphySerdesManualRxDBSet function
* @endinternal
*
* @brief   set override param DB
*
* @param[in]  devNum                  - system device number
* @param[in]  portGroup               - port group (core) number
* @param[in]  phyPortNum              - port number
* @param[in]  serdesSpeed             - serdes speed
* @param[in]  serdesNum               - serdesNum
* @param[in]  *rxComphyConfigPtr      - rx override prameters ptr
*
* @retval 0                           - on success
* @retval 1                           - on error
*/
GT_STATUS mvHwsComphySerdesManualRxDBSet
(
    IN  GT_U8                                   devNum,
    IN  GT_UOPT                                 portGroup,
    IN  GT_U32                                  phyPortNum,
    IN  MV_HWS_SERDES_SPEED                     serdesSpeed,
    IN  GT_UOPT                                 serdesNum,
    IN  MV_HWS_SERDES_RX_CONFIG_DATA_UNT        *rxComphyConfigPtr
);

/**
* @internal mvHwsComphySerdesManualDBClear function
* @endinternal
*
* @brief   clear override param DB
*
* @param[in]  devNum                  - system device number
* @param[in]  phyPortNum              - port number
* @param[in]  serdesNum               - serdesNum
*
* @retval 0                           - on success
* @retval 1                           - on error
*/
GT_STATUS mvHwsComphySerdesManualDBClear
(
    IN GT_U8                                devNum,
    IN GT_U32                               phyPortNum,
    IN GT_UOPT                              serdesNum
);

/**
* @internal mvHwsComphySerdesAddToDatabase function
* @endinternal
*
* @brief   Add SerDes to the main SerDeses database. Allocate
*          the database if needed (in the first call) Note:
*          before the firs call, the hwsDeviceSpecInfo must be
*          initialized with the correct numOfSerdeses and
*          serdesesDb=NULL
*
* @param[in] devNum                   - system device number
* @param[in] serdesIndex              - index in the table where
*                                       to add the new SerDes
* @param[in] serdesInfoPrototype      - a prototype of the
*                                       SerDes record to add
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphySerdesAddToDatabase
(
    IN GT_U8                                devNum,
    IN GT_U32                               serdesIndex,
    IN const MV_HWS_PER_SERDES_INFO_STC     *serdesInfoPrototype
);

/**
* @internal mvHwsComphySerdesDestroyDatabase function
* @endinternal
*
* @brief   Free allocated memory
*
* @param[in] devNum                   - system device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphySerdesDestroyDatabase
(
    IN GT_U8  devNum
);

#if defined  SHARED_MEMORY && !defined MV_HWS_REDUCED_BUILD

const MV_HWS_SERDES_TXRX_TUNE_PARAMS * mvHwsComphyTxRxTuneParamsGet
(
    GT_U8   devNum,
    GT_UOPT serdesNum
);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __mvComphyIf_H */


