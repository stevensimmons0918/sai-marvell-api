/*******************************************************************************
Copyright (C) 2014-2016, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file is a placeholder of ComPhy defined file.
********************************************************************/

#ifndef MV_HWS_COMPHY_SERDES_H
#define MV_HWS_COMPHY_SERDES_H

/* undef non relevant SerDes IPs */
#undef C12GP4                           /* 16FFC_COMPHY_12G_PIPE4, 12FFC_COMPHY_12G_PIPE4 */
#undef C28GP4X2                         /* 16FFC_COMPHY_28G_PIPE4_X2, 12FFC_COMPHY_28G_PIPE4_X2  */
#undef C56GX4                           /* 12FFC_COMPHY_56G_X4 */
#undef C56GP5X4                         /* 7FF_COMPHY_56G_PIPE5_2PLL_X4 */
#undef C112GADCX4                       /* 7FF_COMPHY_112G_ADC_X4 */
#undef N5C56GP5X4                       /* 5FFP_COMPHY_56G_PIPE5_X4_4PLL */
#undef N5C112GX4                        /* 5FFP_COMPHY_112G_ADC_X4_8PLL */
#undef N5C112GX4                        /* 5FFP_COMPHY_112G_ADC_X4_8PLL */
#undef N5XC112GX4                       /* 5FFP_COMPHY_112G_ADC_X4_8PLL - X9380 */

#ifdef AC5_DEV_SUPPORT
#undef C28GP4X1                         /* 12FFC_COMPHY_28G_PIPE4_RPLL_1P2V */
#undef C28GP4X4                         /* 12FFC_COMPHY_28G_PIPE4_RPLL_X4_1P2V */
#undef C112GX4                          /* 12FFC_COMPHY_112G_X4 */
#undef N5XC56GP5X4                      /* 5FFP_COMPHY_56G_PIPE5_X4_4PLL - X9380 */
#endif

/* IP revisions */
#define IP_MAJOR_C12GP41P2V   2
#define IP_MINOR_C12GP41P2V   0

#define IP_MAJOR_C28GP4X2     1
#define IP_MINOR_C28GP4X2     0

#define IP_MAJOR_C28GP4X1     1
#define IP_MINOR_C28GP4X1     0

#define IP_MAJOR_C28GP4X4     1
#define IP_MINOR_C28GP4X4     0

#define IP_MAJOR_C112GX4      1
#define IP_MINOR_C112GX4      2

#define IP_MAJOR_N5XC56GP5X4  1
#define IP_MINOR_N5XC56GP5X4  2

#define mcesdGetAPIVersion                      mvHws_mcesdGetAPIVersion
#define mcesdGetMCUActive                       mvHws_mcesdGetMCUActive
#define mcesdGetIPRev                           mvHws_mcesdGetIPRev
#define mcesdLoadDriver                         mvHws_mcesdLoadDriver
#define mcesdUnloadDriver                       mvHws_mcesdUnloadDriver
#define ConvertSignedMagnitudeToI32             mvHws_ConvertSignedMagnitudeToI32
#define ConvertTwosComplementToI32              mvHws_ConvertTwosComplementToI32
#define LoadFwDataFileToBuffer                  mvHws_LoadFwDataFileToBuffer
#define PatternStringToU8Array                  mvHws_PatternStringToU8Array
#define GenerateStringFromU8Array               mvHws_GenerateStringFromU8Array
#define ConvertU32ToGrayCode                    mvHws_ConvertU32ToGrayCode
#define ConvertGrayCodeToU32                    mvHws_ConvertGrayCodeToU32
#define calculateChecksum                       mvHws_calculateChecksum

#ifdef C12GP41P2V
#define API_C12GP41P2V_PollField                mvHws_API_C12GP41P2V_PollField
#define API_C12GP41P2V_PollPin                  mvHws_API_C12GP41P2V_PollPin
#define API_C12GP41P2V_ReadField                mvHws_API_C12GP41P2V_ReadField
#define API_C12GP41P2V_ReadReg                  mvHws_API_C12GP41P2V_ReadReg
#define API_C12GP41P2V_WriteField               mvHws_API_C12GP41P2V_WriteField
#define API_C12GP41P2V_WriteReg                 mvHws_API_C12GP41P2V_WriteReg
#define API_C12GP41P2V_HwGetPinCfg              mvHws_API_C12GP41P2V_HwGetPinCfg
#define API_C12GP41P2V_HwReadReg                mvHws_API_C12GP41P2V_HwReadReg
#define API_C12GP41P2V_HwSetPinCfg              mvHws_API_C12GP41P2V_HwSetPinCfg
#define API_C12GP41P2V_HwWriteReg               mvHws_API_C12GP41P2V_HwWriteReg
#define API_C12GP41P2V_Wait                     mvHws_API_C12GP41P2V_Wait
#define API_C12GP41P2V_PowerOffLane             mvHws_API_C12GP41P2V_PowerOffLane
#define API_C12GP41P2V_PowerOnSeq               mvHws_API_C12GP41P2V_PowerOnSeq
#define API_C12GP41P2V_CheckTraining            mvHws_API_C12GP41P2V_CheckTraining
#define API_C12GP41P2V_EOMFinalize              mvHws_API_C12GP41P2V_EOMFinalize
#define API_C12GP41P2V_EOMGetWidthHeight        mvHws_API_C12GP41P2V_EOMGetWidthHeight
#define API_C12GP41P2V_EOMInit                  mvHws_API_C12GP41P2V_EOMInit
#define API_C12GP41P2V_EOMMeasPoint             mvHws_API_C12GP41P2V_EOMMeasPoint
#define API_C12GP41P2V_ExecuteTraining          mvHws_API_C12GP41P2V_ExecuteTraining
#define API_C12GP41P2V_GetAlign90               mvHws_API_C12GP41P2V_GetAlign90
#define API_C12GP41P2V_GetCDRLock               mvHws_API_C12GP41P2V_GetCDRLock
#define API_C12GP41P2V_GetCDRParam              mvHws_API_C12GP41P2V_GetCDRParam
#define API_C12GP41P2V_GetComparatorStats       mvHws_API_C12GP41P2V_GetComparatorStats
#define API_C12GP41P2V_GetCTLEParam             mvHws_API_C12GP41P2V_GetCTLEParam
#define API_C12GP41P2V_GetDataBusWidth          mvHws_API_C12GP41P2V_GetDataBusWidth
#define API_C12GP41P2V_GetDataPath              mvHws_API_C12GP41P2V_GetDataPath
#define API_C12GP41P2V_GetDfeEnable             mvHws_API_C12GP41P2V_GetDfeEnable
#define API_C12GP41P2V_GetDfeTap                mvHws_API_C12GP41P2V_GetDfeTap
#define API_C12GP41P2V_GetFreezeDfeUpdates      mvHws_API_C12GP41P2V_GetFreezeDfeUpdates
#define API_C12GP41P2V_GetPhyMode               mvHws_API_C12GP41P2V_GetPhyMode
#define API_C12GP41P2V_GetPLLLock               mvHws_API_C12GP41P2V_GetPLLLock
#define API_C12GP41P2V_GetPowerIvRef            mvHws_API_C12GP41P2V_GetPowerIvRef
#define API_C12GP41P2V_GetPowerPLL              mvHws_API_C12GP41P2V_GetPowerPLL
#define API_C12GP41P2V_GetPowerRx               mvHws_API_C12GP41P2V_GetPowerRx
#define API_C12GP41P2V_GetPowerTx               mvHws_API_C12GP41P2V_GetPowerTx
#define API_C12GP41P2V_GetRefFreq               mvHws_API_C12GP41P2V_GetRefFreq
#define API_C12GP41P2V_GetSlewRateEnable        mvHws_API_C12GP41P2V_GetSlewRateEnable
#define API_C12GP41P2V_GetSlewRateParam         mvHws_API_C12GP41P2V_GetSlewRateParam
#define API_C12GP41P2V_GetSquelchDetect         mvHws_API_C12GP41P2V_GetSquelchDetect
#define API_C12GP41P2V_GetSquelchThreshold      mvHws_API_C12GP41P2V_GetSquelchThreshold
#define API_C12GP41P2V_GetTrainingTimeout       mvHws_API_C12GP41P2V_GetTrainingTimeout
#define API_C12GP41P2V_GetTxEqParam             mvHws_API_C12GP41P2V_GetTxEqParam
#define API_C12GP41P2V_GetTxOutputEnable        mvHws_API_C12GP41P2V_GetTxOutputEnable
#define API_C12GP41P2V_GetTxRxBitRate           mvHws_API_C12GP41P2V_GetTxRxBitRate
#define API_C12GP41P2V_GetTxRxPattern           mvHws_API_C12GP41P2V_GetTxRxPattern
#define API_C12GP41P2V_GetTxRxPolarity          mvHws_API_C12GP41P2V_GetTxRxPolarity
#define API_C12GP41P2V_GetTxRxReady             mvHws_API_C12GP41P2V_GetTxRxReady
#define API_C12GP41P2V_ResetComparatorStats     mvHws_API_C12GP41P2V_ResetComparatorStats
#define API_C12GP41P2V_RxInit                   mvHws_API_C12GP41P2V_RxInit
#define API_C12GP41P2V_SetAlign90               mvHws_API_C12GP41P2V_SetAlign90
#define API_C12GP41P2V_SetCDRParam              mvHws_API_C12GP41P2V_SetCDRParam
#define API_C12GP41P2V_SetCTLEParam             mvHws_API_C12GP41P2V_SetCTLEParam
#define API_C12GP41P2V_SetDataBusWidth          mvHws_API_C12GP41P2V_SetDataBusWidth
#define API_C12GP41P2V_SetDataPath              mvHws_API_C12GP41P2V_SetDataPath
#define API_C12GP41P2V_SetDfeEnable             mvHws_API_C12GP41P2V_SetDfeEnable
#define API_C12GP41P2V_SetFreezeDfeUpdates      mvHws_API_C12GP41P2V_SetFreezeDfeUpdates
#define API_C12GP41P2V_SetPhyMode               mvHws_API_C12GP41P2V_SetPhyMode
#define API_C12GP41P2V_SetPowerIvRef            mvHws_API_C12GP41P2V_SetPowerIvRef
#define API_C12GP41P2V_SetPowerPLL              mvHws_API_C12GP41P2V_SetPowerPLL
#define API_C12GP41P2V_SetPowerRx               mvHws_API_C12GP41P2V_SetPowerRx
#define API_C12GP41P2V_SetPowerTx               mvHws_API_C12GP41P2V_SetPowerTx
#define API_C12GP41P2V_SetRefFreq               mvHws_API_C12GP41P2V_SetRefFreq
#define API_C12GP41P2V_SetSlewRateEnable        mvHws_API_C12GP41P2V_SetSlewRateEnable
#define API_C12GP41P2V_SetSlewRateParam         mvHws_API_C12GP41P2V_SetSlewRateParam
#define API_C12GP41P2V_SetSquelchThreshold      mvHws_API_C12GP41P2V_SetSquelchThreshold
#define API_C12GP41P2V_SetTrainingTimeout       mvHws_API_C12GP41P2V_SetTrainingTimeout
#define API_C12GP41P2V_SetTxEqParam             mvHws_API_C12GP41P2V_SetTxEqParam
#define API_C12GP41P2V_SetTxOutputEnable        mvHws_API_C12GP41P2V_SetTxOutputEnable
#define API_C12GP41P2V_SetTxRxBitRate           mvHws_API_C12GP41P2V_SetTxRxBitRate
#define API_C12GP41P2V_SetTxRxPattern           mvHws_API_C12GP41P2V_SetTxRxPattern
#define API_C12GP41P2V_SetTxRxPolarity          mvHws_API_C12GP41P2V_SetTxRxPolarity
#define API_C12GP41P2V_StartPhyTest             mvHws_API_C12GP41P2V_StartPhyTest
#define API_C12GP41P2V_StartTraining            mvHws_API_C12GP41P2V_StartTraining
#define API_C12GP41P2V_StopPhyTest              mvHws_API_C12GP41P2V_StopPhyTest
#define API_C12GP41P2V_StopTraining             mvHws_API_C12GP41P2V_StopTraining
#define API_C12GP41P2V_TxInjectError            mvHws_API_C12GP41P2V_TxInjectError
#endif /*C12GP41P2V*/

#ifdef C112GX4
#define API_C112GX4_PollField                   mvHws_API_C112GX4_PollField
#define API_C112GX4_PollPin                     mvHws_API_C112GX4_PollPin
#define API_C112GX4_ReadField                   mvHws_API_C112GX4_ReadField
#define API_C112GX4_ReadReg                     mvHws_API_C112GX4_ReadReg
#define API_C112GX4_WriteField                  mvHws_API_C112GX4_WriteField
#define API_C112GX4_WriteReg                    mvHws_API_C112GX4_WriteReg
#define API_C112GX4_HwGetPinCfg                 mvHws_API_C112GX4_HwGetPinCfg
#define API_C112GX4_HwReadReg                   mvHws_API_C112GX4_HwReadReg
#define API_C112GX4_HwSetPinCfg                 mvHws_API_C112GX4_HwSetPinCfg
#define API_C112GX4_HwWriteReg                  mvHws_API_C112GX4_HwWriteReg
#define API_C112GX4_Wait                        mvHws_API_C112GX4_Wait
#define API_C112GX4_DownloadFirmware            mvHws_API_C112GX4_DownloadFirmware
#define API_C112GX4_ProgCmnXData                mvHws_API_C112GX4_ProgCmnXData
#define API_C112GX4_ProgLaneXData               mvHws_API_C112GX4_ProgLaneXData
#define API_C112GX4_UpdateRamCode               mvHws_API_C112GX4_UpdateRamCode
#define API_C112GX4_PowerOnSeq                  mvHws_API_C112GX4_PowerOnSeq
#define API_C112GX4_EOM1UIStepCount             mvHws_API_C112GX4_EOM1UIStepCount
#define API_C112GX4_EOMFinalize                 mvHws_API_C112GX4_EOMFinalize
#define API_C112GX4_EOMGetWidthHeight           mvHws_API_C112GX4_EOMGetWidthHeight
#define API_C112GX4_EOMInit                     mvHws_API_C112GX4_EOMInit
#define API_C112GX4_EOMMeasPoint                mvHws_API_C112GX4_EOMMeasPoint
#define API_C112GX4_ExecuteCDS                  mvHws_API_C112GX4_ExecuteCDS
#define API_C112GX4_ExecuteTraining             mvHws_API_C112GX4_ExecuteTraining
#define API_C112GX4_GetAlign90                  mvHws_API_C112GX4_GetAlign90
#define API_C112GX4_GetCDRParam                 mvHws_API_C112GX4_GetCDRParam
#define API_C112GX4_GetComparatorStats          mvHws_API_C112GX4_GetComparatorStats
#define API_C112GX4_GetCTLEParam                mvHws_API_C112GX4_GetCTLEParam
#define API_C112GX4_GetDataAcquisitionRate      mvHws_API_C112GX4_GetDataAcquisitionRate
#define API_C112GX4_GetDataBusWidth             mvHws_API_C112GX4_GetDataBusWidth
#define API_C112GX4_GetDataPath                 mvHws_API_C112GX4_GetDataPath
#define API_C112GX4_GetDfeEnable                mvHws_API_C112GX4_GetDfeEnable
#define API_C112GX4_GetDfeTap                   mvHws_API_C112GX4_GetDfeTap
#define API_C112GX4_GetFirmwareRev              mvHws_API_C112GX4_GetFirmwareRev
#define API_C112GX4_GetFreezeDfeUpdates         mvHws_API_C112GX4_GetFreezeDfeUpdates
#define API_C112GX4_GetLaneEnable               mvHws_API_C112GX4_GetLaneEnable
#define API_C112GX4_GetMcuBroadcast             mvHws_API_C112GX4_GetMcuBroadcast
#define API_C112GX4_GetMcuClockFreq             mvHws_API_C112GX4_GetMcuClockFreq
#define API_C112GX4_GetMcuEnable                mvHws_API_C112GX4_GetMcuEnable
#define API_C112GX4_GetPhyMode                  mvHws_API_C112GX4_GetPhyMode
#define API_C112GX4_GetPLLLock                  mvHws_API_C112GX4_GetPLLLock
#define API_C112GX4_GetPowerIvRef               mvHws_API_C112GX4_GetPowerIvRef
#define API_C112GX4_GetPowerPLL                 mvHws_API_C112GX4_GetPowerPLL
#define API_C112GX4_GetPowerRx                  mvHws_API_C112GX4_GetPowerRx
#define API_C112GX4_GetPowerTx                  mvHws_API_C112GX4_GetPowerTx
#define API_C112GX4_GetRefFreq                  mvHws_API_C112GX4_GetRefFreq
#define API_C112GX4_GetSquelchDetect            mvHws_API_C112GX4_GetSquelchDetect
#define API_C112GX4_GetSquelchThreshold         mvHws_API_C112GX4_GetSquelchThreshold
#define API_C112GX4_GetTemperature              mvHws_API_C112GX4_GetTemperature
#define API_C112GX4_GetTrainedEyeHeight         mvHws_API_C112GX4_GetTrainedEyeHeight
#define API_C112GX4_GetTrainingTimeout          mvHws_API_C112GX4_GetTrainingTimeout
#define API_C112GX4_GetTxEqParam                mvHws_API_C112GX4_GetTxEqParam
#define API_C112GX4_GetTxOutputEnable           mvHws_API_C112GX4_GetTxOutputEnable
#define API_C112GX4_GetTxRxBitRate              mvHws_API_C112GX4_GetTxRxBitRate
#define API_C112GX4_GetTxRxPattern              mvHws_API_C112GX4_GetTxRxPattern
#define API_C112GX4_GetTxRxPolarity             mvHws_API_C112GX4_GetTxRxPolarity
#define API_C112GX4_GetTxRxReady                mvHws_API_C112GX4_GetTxRxReady
#define API_C112GX4_ResetComparatorStats        mvHws_API_C112GX4_ResetComparatorStats
#define API_C112GX4_RxInit                      mvHws_API_C112GX4_RxInit
#define API_C112GX4_SetAlign90                  mvHws_API_C112GX4_SetAlign90
#define API_C112GX4_SetCDRParam                 mvHws_API_C112GX4_SetCDRParam
#define API_C112GX4_SetCTLEParam                mvHws_API_C112GX4_SetCTLEParam
#define API_C112GX4_SetDataBusWidth             mvHws_API_C112GX4_SetDataBusWidth
#define API_C112GX4_SetDataPath                 mvHws_API_C112GX4_SetDataPath
#define API_C112GX4_SetDfeEnable                mvHws_API_C112GX4_SetDfeEnable
#define API_C112GX4_SetFreezeDfeUpdates         mvHws_API_C112GX4_SetFreezeDfeUpdates
#define API_C112GX4_SetLaneEnable               mvHws_API_C112GX4_SetLaneEnable
#define API_C112GX4_SetMcuBroadcast             mvHws_API_C112GX4_SetMcuBroadcast
#define API_C112GX4_SetMcuClockFreq             mvHws_API_C112GX4_SetMcuClockFreq
#define API_C112GX4_SetMcuEnable                mvHws_API_C112GX4_SetMcuEnable
#define API_C112GX4_SetPhyMode                  mvHws_API_C112GX4_SetPhyMode
#define API_C112GX4_SetPowerIvRef               mvHws_API_C112GX4_SetPowerIvRef
#define API_C112GX4_SetPowerPLL                 mvHws_API_C112GX4_SetPowerPLL
#define API_C112GX4_SetPowerRx                  mvHws_API_C112GX4_SetPowerRx
#define API_C112GX4_SetPowerTx                  mvHws_API_C112GX4_SetPowerTx
#define API_C112GX4_SetRefFreq                  mvHws_API_C112GX4_SetRefFreq
#define API_C112GX4_SetSquelchThreshold         mvHws_API_C112GX4_SetSquelchThreshold
#define API_C112GX4_SetTrainingTimeout          mvHws_API_C112GX4_SetTrainingTimeout
#define API_C112GX4_SetTxEqParam                mvHws_API_C112GX4_SetTxEqParam
#define API_C112GX4_SetTxOutputEnable           mvHws_API_C112GX4_SetTxOutputEnable
#define API_C112GX4_SetTxRxBitRate              mvHws_API_C112GX4_SetTxRxBitRate
#define API_C112GX4_SetTxRxPattern              mvHws_API_C112GX4_SetTxRxPattern
#define API_C112GX4_SetTxRxPolarity             mvHws_API_C112GX4_SetTxRxPolarity
#define API_C112GX4_StartPhyTest                mvHws_API_C112GX4_StartPhyTest
#define API_C112GX4_StopPhyTest                 mvHws_API_C112GX4_StopPhyTest
#define API_C112GX4_TxInjectError               mvHws_API_C112GX4_TxInjectError
#define API_C112GX4_SetTxEqPolarity             mvHws_API_C112GX4_SetTxEqPolarity
#define API_C112GX4_GetTxEqPolarity             mvHws_API_C112GX4_GetTxEqPolarity
#endif /*C112GX4*/

#ifdef C12GP41P2V
#define IF_C12GP41P2V(_serdesType, ...) {if((SERDES_LAST == _serdesType)||(COM_PHY_C12GP41P2V == _serdesType)) {__VA_ARGS__}}
#else
#define IF_C12GP41P2V(_serdesType, ...)
#endif

#ifdef C28GP4X1
#define IF_C28GP4X1(_serdesType, ...) {if((SERDES_LAST == _serdesType)||(COM_PHY_C28GP4X1 == _serdesType)) {__VA_ARGS__}}
#else
#define IF_C28GP4X1(_serdesType, ...)
#endif

#ifdef C28GP4X4
#define IF_C28GP4X4(_serdesType, ...) {if((SERDES_LAST == _serdesType)||(COM_PHY_C28GP4X4 == _serdesType)) {__VA_ARGS__}}
#else
#define IF_C28GP4X4(_serdesType, ...)
#endif

#ifdef C112GX4
#define IF_C112GX4(_serdesType, ...) {if((SERDES_LAST == _serdesType)||(COM_PHY_C112GX4 == _serdesType)) {__VA_ARGS__}}
#else
#define IF_C112GX4(_serdesType, ...)
#endif

#ifdef N5XC56GP5X4
#define IF_N5XC56GP5X4(_serdesType, ...) {if((SERDES_LAST == _serdesType)||(COM_PHY_N5XC56GP5X4 == _serdesType)) {__VA_ARGS__}}
#else
#define IF_N5XC56GP5X4(_serdesType, ...)
#endif


#endif /* defined MV_HWS_COMPHY_SERDES_H */

