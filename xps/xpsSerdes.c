// xpsSerdes.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsMac.h"
#include "xpsSerdes.h"
#include "xpsPort.h"
#include "xpsLock.h"
#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsSerdesAaplGetReturnCode(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetEyeWithDefaultConfig(xpDevice_t devId, uint8_t serdesId,
                                           xpSerdesEyeConfig_t *serdesEyeConfigPtr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesAaplSetIpType(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesHardwareInfoFormat(xpsDevice_t devId, uint8_t serdesId,
                                      char *hwInfo)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDiagSerdesRwTest(xpsDevice_t devId, uint8_t serdesId,
                                    int32_t cycles)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDiagSerdesDump(xpsDevice_t devId, uint8_t serdesId,
                                  int32_t binEnable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDiag1(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

const char *xpsSerdesGetAaplLog(xpsDevice_t devId, uint8_t serdesId)
{
    return NULL;
}

XP_STATUS xpsSerdesDfePause(xpsDevice_t devId, uint8_t serdesId,
                            uint32_t *rrEnabled)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDfeWait(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDfeWaitTimeout(xpsDevice_t devId, uint8_t serdesId,
                                  int32_t timeoutInMilliseconds)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDfeResume(xpsDevice_t devId, uint8_t serdesId,
                             uint32_t rrEnable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesPmdTrain(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesPmdDebug(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesPmdDebugPrint(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
//not there
XP_STATUS xpsSerdesCommandExec(xpsDevice_t devId, uint8_t serdesId,
                               unsigned char regAddr, unsigned char command, uint32_t serdesData,
                               int32_t recvDataBack, uint32_t *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesWr(xpsDevice_t devId, uint8_t serdesId,
                      unsigned char regAddr, uint32_t serdesData, uint32_t *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesRmw(xpsDevice_t devId, uint8_t serdesId,
                       unsigned char regAddr, uint32_t serdesData, uint32_t mask, uint32_t *initialVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesRd(xpsDevice_t devId, uint8_t serdesId,
                      unsigned char regAddr, uint32_t *rdData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesReset(xpsDevice_t devId, uint8_t serdesId, int32_t hard)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDiag(xpsDevice_t devId, uint8_t serdesId,
                        uint32_t *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesStateDump(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeErrorsGet(xpsDevice_t devId, uint8_t serdesId,
                                uint32_t xPoint, uint32_t yPoint, int64_t *errCnt)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeBitsGet(xpsDevice_t devId, uint8_t serdesId,
                              uint32_t xPoint, uint32_t yPoint, int64_t *bitCnt)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeGradGet(xpsDevice_t devId, uint8_t serdesId,
                              uint32_t xPoint, uint32_t yPoint, Float *gradVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeQvalGet(xpsDevice_t devId, uint8_t serdesId,
                              uint32_t xPoint, uint32_t yPoint, Float *qVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyePlotLogPrint(xpsDevice_t devId, uint8_t serdesId,
                                   xpSerdesLogType_t level, const char *func, int32_t line)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeHbtcLogPrint(xpsDevice_t devId, uint8_t serdesId,
                                   xpSerdesLogType_t level, const char *func, int32_t line,
                                   xpSerdesEyeHbtc_t *hbtcp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeVbtcLogPrint(xpsDevice_t devId, uint8_t serdesId,
                                   xpSerdesLogType_t level, const char *func, int32_t line,
                                   xpSerdesEyeVbtc_t *vbtcp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeDataReadFile(xpsDevice_t devId, uint8_t serdesId,
                                   const char *filename)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesAacsServer(xpsDevice_t devId, uint8_t serdesId, int tcpPort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetFirmwareRev(xpsDevice_t devId, uint8_t serdesId,
                                  uint32_t *fwRev)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetFirmwareBuildId(xpsDevice_t devId, uint8_t serdesId,
                                      uint32_t *fwBldId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetLsbRev(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetTxRxReady(xpsDevice_t devId, uint8_t serdesId, int *tx,
                                int *rx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetTxRxEnable(xpsDevice_t devId, uint8_t serdesId,
                                 uint8_t tx, uint8_t rx, uint8_t txOutput)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetTxOutputEnable(xpsDevice_t devId, uint8_t serdesId,
                                     int *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetTxOutputEnable(xpsDevice_t devId, uint8_t serdesId,
                                     uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetTxRxWidth(xpsDevice_t devId, uint8_t serdesId,
                                int32_t *txWidth, int32_t *rxWidth)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetTxRxWidth(xpsDevice_t devId, uint8_t serdesId,
                                int32_t txWidth, int32_t rxWidth)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetTxInvert(xpsDevice_t devId, uint8_t serdesId,
                               uint8_t invert)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetRxInvert(xpsDevice_t devId, uint8_t serdesId,
                               uint8_t invert)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesTxInjectError(xpsDevice_t devId, uint8_t serdesId,
                                 uint32_t numBits)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesRxInjectError(xpsDevice_t devId, uint8_t serdesId,
                                 uint32_t numBits)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetRxInputLoopback(xpsDevice_t devId, uint8_t serdesId,
                                      uint8_t selectInternal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetTxUserData(xpsDevice_t devId, uint8_t serdesId,
                                 long data[4])
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetTxUserData(xpsDevice_t devId, uint8_t serdesId,
                                 long data[4])
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetTxDataSel(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesTxDataSel_t input)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetRxCmpData(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesRxCmpData_t cmpData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetTxEq(xpsDevice_t devId, uint8_t serdesId,
                           xpSerdesTxEq_t *txEq)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetTxEq(xpsDevice_t devId, uint8_t serdesId,
                           xpSerdesTxEq_t *txEq)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetRxTerm(xpsDevice_t devId, uint8_t serdesId,
                             xpSerdesRxTerm_t term)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetRxCmpMode(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesRxCmpMode_t mode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetClkSrc(xpsDevice_t devId, uint8_t serdesId,
                             xpSerdesClk_t src)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetTxPllClkSrc(xpsDevice_t devId, uint8_t serdesId,
                                  xpSerdesTxPllClk_t src)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesMemRd(xpsDevice_t devId, uint8_t serdesId,
                         xpSerdesMemType_t type, uint32_t memAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesMemWr(xpsDevice_t devId, uint8_t serdesId,
                         xpSerdesMemType_t type, uint32_t memAddr, uint32_t data)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesMemRmw(xpsDevice_t devId, uint8_t serdesId,
                          xpSerdesMemType_t type, uint32_t memAddr, uint32_t data, uint32_t mask)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetSignalOk(xpsDevice_t devId, uint8_t serdesId,
                               uint8_t reset, int *signalOk)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesInitializeSignalOk(xpsDevice_t devId, uint8_t serdesId,
                                      int32_t threshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetErrors(xpsDevice_t devId, uint8_t serdesId,
                             xpSerdesMemType_t type, uint8_t resetCountAfterGet, uint32_t *errCnt)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetTxInvert(xpsDevice_t devId, uint8_t serdesId,
                               uint8_t *getTxInvert)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetRxInvert(xpsDevice_t devId, uint8_t serdesId,
                               uint8_t *getRxInvert)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetRxInputLoopback(xpsDevice_t devId, uint8_t serdesId,
                                      uint8_t *getRxInputLoopback)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetTxDataSel(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesTxDataSel_t *txDataSel)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetRxCmpData(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesRxCmpData_t *rxCmpData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetRxTerm(xpsDevice_t devId, uint8_t serdesId,
                             xpSerdesRxTerm_t *getRxTerm)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetRxCmpMode(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesRxCmpMode_t *rxCmpModeT)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetClkSrc(xpsDevice_t devId, uint8_t serdesId,
                             xpSerdesClk_t *ClkT)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetTxPllClkSrc(xpsDevice_t devId, uint8_t serdesId,
                                  xpSerdesTxPllClk_t *txPllClkT)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeDataWriteFile(xpsDevice_t devId, uint8_t serdesId,
                                    const char *filename)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeDataWrite(xpsDevice_t devId, uint8_t serdesId, FILE *file)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeVbtcWrite(xpsDevice_t devId, uint8_t serdesId,
                                const char *fileName)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeHbtcWrite(xpsDevice_t devId, uint8_t serdesId,
                                const char *fileName)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyePlotWrite(xpsDevice_t devId, uint8_t serdesId, FILE *file)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeVbtcFormat(xpsDevice_t devId, uint8_t serdesId,
                                 xpSerdesEyeVbtc_t *vbtcp, char *eyeData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeHbtcFormat(xpsDevice_t devId, uint8_t serdesId,
                                 xpSerdesEyeHbtc_t *hbtcp, char *eyeData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyePlotFormat(xpsDevice_t devId, uint8_t serdesId,
                                 char *eyeData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeVbtcExtrapolate(xpsDevice_t devId, uint8_t serdesId,
                                      uint32_t dataCol, xpSerdesEyeVbtc_t *results)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeHbtcExtrapolate(xpsDevice_t devId, uint8_t serdesId,
                                      uint32_t dataRow, xpSerdesEyeHbtc_t *results)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCtrlrStatus(xpsDevice_t devId, uint8_t serdesId,
                               xpSerdesStatus_t *st, bool no_cache)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCtrlrRunning(xpsDevice_t devId, uint8_t serdesId,
                                uint32_t *runStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCtrlrReset(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCtrlrUploadSwapImage(xpsDevice_t devId, uint8_t serdesId,
                                        int32_t words, const int32_t *rom)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesFirmwareUpload(xpsDevice_t devId, uint8_t serdesId,
                                  uint8_t ramBist, int32_t words, const int32_t *rom)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCtrlrInt(xpsDevice_t devId, uint8_t serdesId, int32_t intNum,
                            int32_t param, uint32_t *intStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCtrlrIntArray(xpsDevice_t devId, uint8_t serdesId,
                                 int32_t numElements, xpSerdesInt_t *interrupts)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCtrlrCrc(xpsDevice_t devId, uint8_t serdesId,
                            uint32_t *crcResult)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCtrlrRamBist(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDfeRunning(xpsDevice_t devId, uint8_t serdesId,
                              uint8_t* dfeRunning)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetSignalOkEnable(xpsDevice_t devId, uint8_t serdesId,
                                     uint8_t *signalOkEnable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetSignalOkThreshold(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeGet(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsSerdesSetAsyncCancelFlag(xpsDevice_t devId, uint8_t serdesId,
                                      int32_t newValue)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetFirmwareBuild(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesAddrInitBroadcast(xpsDevice_t devId, uint8_t serdesId,
                                     xpSerdesAddr_t *addrStruct, uint32_t *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDeviceInfo(xpsDevice_t devId, uint8_t serdesId,
                              xpSerdesAddr_t *addrStruct, xpSerdesIpType_t type)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsSerdesSerdesWrFlush(xpsDevice_t devId, uint8_t serdesId,
                                 unsigned char reg, uint32_t data, uint32_t *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetTxLineEncoding(xpsDevice_t devId, uint8_t serdesId,
                                     uint8_t *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetRxLineEncoding(xpsDevice_t devId, uint8_t serdesId,
                                     uint8_t *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetTxRxDataWidth(xpsDevice_t devId, uint8_t serdesId,
                                    int32_t txWidth, int32_t rxWidth, xpSerdesEncodingMode_t txEncoding,
                                    xpSerdesEncodingMode_t rxEncoding)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSlipBits(xpsDevice_t devId, uint8_t serdesId, uint32_t bits,
                            uint8_t *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSlipRxPhase(xpsDevice_t devId, uint8_t serdesId,
                               uint32_t bits, uint8_t applyAtInit, uint8_t *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSlipTxPhase(xpsDevice_t devId, uint8_t serdesId,
                               uint32_t bits, uint8_t applyAtInit, uint8_t *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesTuneInit(xpsDevice_t devId, uint8_t serdesId,
                            xpSerdesDfeTune_t *control)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeGetSimpleMetric(xpsDevice_t devId, uint8_t serdesId,
                                      uint32_t *eyeMetric)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeSimpleMetricThresholdGet(xpsDevice_t devId,
                                               uint8_t serdesId, uint32_t *eyeMetricThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeSimpleMetricThresholdSet(xpsDevice_t devId,
                                               uint8_t serdesId, uint32_t eyeMetricThreshold)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetRxLiveData(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCtrlrDiag(xpsDevice_t devId, uint8_t serdesId,
                             int32_t cycles, uint32_t *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesFirmwareUpload_2(xpsDevice_t devId, uint8_t serdesId,
                                    int32_t serdesRomSize, const int32_t *serdesRom, int32_t serdesCtrlrRomSize,
                                    const int32_t *serdesCtrlrRom, int32_t sdiRomSize, const int32_t *sdiRom)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCtrlrWaitForUpload(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesTwiWaitForComplete(xpsDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesFirmwareUploadFile_2(xpsDevice_t devId, uint8_t serdesId,
                                        const char *serdesRomFile, const char *serdesCtrlrRomFile,
                                        const char *sdiRomFile)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetSerdesCtleVal(xpDevice_t devId, uint8_t serdesId,
                                    xpSerdesCtle_t* ctleVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCtrlrIntCheck(xpsDevice_t devId, uint8_t serdesId,
                                 const char *caller, int32_t line, int32_t intNum, int32_t param,
                                 uint8_t *serdesStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsSerdesSetStruct(xpsDevice_t devId, uint8_t serdesId,
                             xpSerdes_t *serdesData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsSerdesGetStruct(xpsDevice_t devId, uint8_t serdesId,
                             xpSerdes_t *serdesData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesTimeoutSet(xpsDevice_t devId, uint8_t serdesId,
                              uint32_t timeoutVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetEyeMeasurement(xpsDevice_t devId, uint8_t serdesId,
                                     uint32_t *eyeHeight, uint32_t *eyeWidth)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetEyeMeasurementWithCustomConfig(xpDevice_t devId,
                                                     uint8_t serdesId, xpSerdesEyeConfig_t *serdesEyeConfigPtr, uint32_t *eyeHeight,
                                                     uint32_t *eyeWidth)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSensorGetTemperature(xpsDevice_t devId, int32_t *temp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCtrlrGetSerdesClockDivider(xpDevice_t devId,
                                              uint8_t serdesId, int *divider)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesConnectionStatus(xpDevice_t devId, uint8_t serdesId,
                                    int *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesCloseConnection(xpDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetCtrlrRunningFlag(xpDevice_t devId, uint8_t serdesId,
                                       int isSerdesCtrlRunning)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetCtrlrRunningFlag(xpDevice_t devId, uint8_t serdesId,
                                       int *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetInterruptRev(xpDevice_t devId, int intNum,
                                   uint8_t serdesId, int *rev)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetSdrev(xpDevice_t devId, uint8_t serdesId, int *rev)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesInitQuick(xpDevice_t devId, uint32_t divider,
                             uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesResume(xpDevice_t devId, int initialState, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesHalt(xpDevice_t devId, uint8_t serdesId, int *runState)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesPmdStatus(xpDevice_t devId, uint8_t serdesId, int *status)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeGradientPlotWrite(xpDevice_t devId, const char *fileName,
                                        uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetTxDatapathEncoding(xpDevice_t devId, int grayCode,
                                         int preCode, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetTxDatapathEncoding(xpDevice_t devId, int *grayCode,
                                         int *preCode, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSetRxDatapathEncoding(xpDevice_t devId, int grayCode,
                                         int preCode, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetRxDatapathEncoding(xpDevice_t devId, int *grayCode,
                                         int *preCode, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetRxData(xpDevice_t devId, uint8_t serdesId, long data[4])
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetMediaStatus(xpDevice_t devId, uint8_t serdesId,
                                  uint8_t *isConnected)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetFrequencyLock(xpDevice_t devId, uint8_t serdesId,
                                    int *locked)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetSignalOkLive(xpDevice_t devId, uint8_t serdesId,
                                   int *isLive)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesGetErrorFlag(xpDevice_t devId, int reset, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEnableCoreToControl(xpDevice_t devId, int txRxEnable,
                                       int lowPowerEnable, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesAddrInit(xpDevice_t devId, xpSerdesAddr_t *addrStruct,
                            uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSensorStartTemperature(xpDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSensorWaitTemperature(xpDevice_t devId, uint8_t serdesId,
                                         int *temp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSensorGetVoltage(xpDevice_t devId, uint8_t serdesId,
                                    int *mVolt)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSensorStartVoltage(xpDevice_t devId, uint8_t serdesId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesSensorWaitVoltage(xpDevice_t devId, uint8_t serdesId,
                                     int *mVolt)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesBroadcastIntWMask(xpDevice_t devId, uint8_t serdesId,
                                     uint32_t addrMask, int intNum, int param, int count, int *args)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesBroadcastInt(xpDevice_t devId, uint8_t serdesId, int intNum,
                                int param, int count, int *args)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesElectricalIdleThresholdSet(xpDevice_t devId,
                                              uint8_t serdesId, uint8_t eiThreshold, uint32_t *intStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesPmroMetricGet(xpDevice_t devId, uint8_t serdesId,
                                 uint32_t *pmroMetric)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDfeStatusGet(xpDevice_t devId, uint8_t serdesId,
                                int32_t *dfeStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDfeStateGet(xpDevice_t devId, uint8_t serdesId,
                               xpSerdesDfeState_t *dfeState)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDfeStatePrint(xpDevice_t devId, uint8_t serdesId,
                                 xpSerdesDfeState_t *dfeState, uint8_t singleLine)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesEyeDataReadPhase(xpDevice_t devId, uint8_t serdesId,
                                    const char *filename, int32_t* dataRdPhaseVal)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSerdesDfeLos(xpDevice_t devId, uint8_t serdesId,
                          uint8_t* isDfeLosDetected)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsRegisterLoggingFunc(xpDevice_t devId, uint8_t serdesId,
                                 userLogFunc LogFunc,
                                 userLogOpenFunc LogOpenFunc, userLogCloseFunc LogCloseFunc)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
#ifdef __cplusplus
}
#endif

