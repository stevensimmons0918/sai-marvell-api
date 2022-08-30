/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

//
// file cpssHalPhy.c
//

#include "cpssHalPhy.h"
#include "prvCpssBindFunc.h"
#include "xpsCommon.h"
#include "cpssHalUtil.h"
#include "gtGenTypes.h"
#include "cpssHalDevice.h"
#include "cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h"
XP_STATUS cpssHalReadPhyReg(xpsDevice_t devId, uint32_t portNum,
                            uint8_t pageNum, uint8_t phyReg, uint16_t* reg)
{
    XP_STATUS ret =  XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    GT_U16 regVal;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevId, cpssPortNum, 22, pageNum);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }

    cpssRet = cpssDxChPhyPortSmiRegisterRead(cpssDevId, cpssPortNum, (GT_U8)phyReg,
                                             &regVal);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }
    *reg = (uint16_t)regVal;
    cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevId, cpssPortNum, 22, 0);
    if (cpssRet != GT_OK)
    {
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }

    return XP_NO_ERR;
}

XP_STATUS cpssHalWritePhyReg(xpsDevice_t devId, uint32_t portNum,
                             uint8_t pageNum, uint8_t phyReg, uint16_t reg)
{

    XP_STATUS ret =  XP_NO_ERR;
    GT_STATUS cpssRet;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    cpssDevId = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevId, cpssPortNum, 22, pageNum);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Write to port smi register failed");
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }

    cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevId, cpssPortNum, (GT_U8)phyReg,
                                              (GT_U16)reg);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Write to port smi register failed");
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }
    cpssRet = cpssDxChPhyPortSmiRegisterWrite(cpssDevId, cpssPortNum, 22, 0);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Write to port smi register failed");
        ret = xpsConvertCpssStatusToXPStatus(cpssRet);
        return ret;
    }

    return XP_NO_ERR;
}


