/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file prvCpssGenUtils.c
*
* @brief CPSS generic wrappers for serdes optimizer and port configuration
* black box
*
* @version   12
********************************************************************************
*/

#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwRegisters.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

GT_STATUS gtStatus;

#ifdef CHX_FAMILY
extern GT_VOID hwsAldrin2SerdesAddrCalc
(
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);

extern GT_VOID hwsBobcat3SerdesAddrCalc
(
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);

extern GT_VOID hwsFalconSerdesAddrCalc
(
    GT_U8           devNum,
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          *addressPtr
);

extern GT_VOID hwsSip6_10SerdesAddrCalc
(
    GT_U8           devNum,
    GT_UOPT         serdesNum,
    GT_UREG_DATA    regAddr,
    GT_U32          regType,
    GT_U32          *addressPtr
);

#else
/* stubs for PX_FAMILY */
#define hwsAldrin2SerdesAddrCalc(a,b,cPtr) *(cPtr) = 0
#define hwsBobcat3SerdesAddrCalc(a,b,cPtr) *(cPtr) = 0
#define hwsSip6_10SerdesAddrCalc(dev, a,b,c,cPtr) *(cPtr) = 0
#define hwsFalconSerdesAddrCalc(dev,a,b,cPtr)  *(cPtr) = 0
#define hwsIsIronmanAsPhoenix()   0
#endif
/**
* @internal genRegisterSet function
* @endinternal
*
* @brief   Writes the unmasked bits of a register.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number to write to.
* @param[in] portGroup                - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] address                  - Register  to write to.
* @param[in] data                     - Data to be written to register.
* @param[in] mask                     - Mask for selecting the written bits.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
GT_STATUS genRegisterSet
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 address,
    IN GT_U32 data,
    IN GT_U32 mask
)
{
    GT_STATUS rc;
#ifndef ASIC_SIMULATION
    GT_U32    chipIdx = 0xFFFFFFFF;
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* We found out that Serdes and D2D are using the same xbar number #6,
        this cause the read/write from d2d to receive timeout when CM3 is trying to access Serdes register.
        We trying to protect the read/write to d2d with semaphore (the one that is used already to protect  read/write from Serdes in host and CM3).
         */
        if((address & 0x00FF0000) == 0x00680000 || /*<D2D0>*/
           (address & 0x00FF0000) == 0x00690000 ){  /*<D2D1>*/

            chipIdx = (address%0x20000000)/ 0x01000000;
            if( ((address/0x20000000)%2) != 0 )
                chipIdx = (3 - chipIdx);
            chipIdx += ((address/0x20000000) * 4);
            AVAGO_LOCK(devNum, chipIdx);
        }
    }
#endif

    rc = cpssDrvPpHwRegBitMaskWrite(devNum,portGroup,address, (mask != 0) ? mask : 0xffffffff,data);

#ifndef ASIC_SIMULATION
    if(chipIdx != 0xFFFFFFFF)
    {
        /* if D2D address we need to take serdes sem (they are both on the same XBAR) */
        if((address & 0x00FF0000) == 0x00680000 || /*<D2D0>*/
           (address & 0x00FF0000) == 0x00690000 ){  /*<D2D1>*/
            AVAGO_UNLOCK(devNum, chipIdx);
        }
    }
#endif
    return rc;
}

/**
* @internal genRegisterGet function
* @endinternal
*
* @brief   Reads the unmasked bits of a register.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - PP device number to read from.
* @param[in] portGroup                - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] address                  - Register  to read from.
* @param[in] mask                     - Mask for selecting the read bits.
*
* @param[out] data                     - Data read from register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note The bits in value to be read are the masked bit of 'mask'.
*
*/
GT_STATUS genRegisterGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroup,
    IN  GT_U32  address,
    OUT GT_U32  *data,
    IN  GT_U32  mask
)
{
    GT_STATUS rc;
#ifndef ASIC_SIMULATION
    GT_U32    chipIdx = 0xFFFFFFFF;
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* We found out that Serdes and D2D are using the same xbar number #6,
        this cause the read/write from d2d to receive timeout when CM3 is trying to access Serdes register.
        We trying to protect the read/write to d2d with semaphore (the one that is used already to protect  read/write from Serdes in host and CM3).
         */
        if((address & 0x00FF0000) == 0x00680000 || /*<D2D0>*/
           (address & 0x00FF0000) == 0x00690000 ){  /*<D2D1>*/

            chipIdx = (address%0x20000000)/ 0x01000000;
            if( ((address/0x20000000)%2) != 0 )
                chipIdx = (3 - chipIdx);
            chipIdx += ((address/0x20000000) * 4);
            AVAGO_LOCK(devNum, chipIdx);
        }
     }
#endif
    CPSS_NULL_PTR_CHECK_MAC(data);
    rc = cpssDrvPpHwRegBitMaskRead(devNum,portGroup,address, (mask != 0) ? mask : 0xffffffff,data);

#ifndef ASIC_SIMULATION
    if(chipIdx != 0xFFFFFFFF)
    {
        /* if D2D address we need to take serdes sem (they are both on the same XBAR) */
        if((address & 0x00FF0000) == 0x00680000 || /*<D2D0>*/
           (address & 0x00FF0000) == 0x00690000 ){  /*<D2D1>*/
            AVAGO_UNLOCK(devNum, chipIdx);
        }
    }
#endif
    return rc;
}

/**
* @internal gtBreakOnFail function
* @endinternal
*
* @brief   Treat failure
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       None.
*/
GT_VOID gtBreakOnFail
(
        GT_VOID
)
{
}

/**
* @internal prvCpssGenPortGroupSerdesCalculateRegAddress
*           function
* @endinternal
*
* @brief   calculate SERDES external/internal registers address.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group number
* @param[in] regType                  - internal/external
* @param[in] serdesNum                - seredes number to access
* @param[in] regAddr                  - serdes register address (offset) to access
* @param[out] realRegAddrPtr          - full register address
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvCpssGenPortGroupSerdesCalculateRegAddress
(
    IN  GT_U8  devNum,
    IN  GT_U8  portGroup,
    IN  GT_U8  regType,
    IN  GT_U32 serdesNum,
    IN  GT_U32 regAddr,
    OUT GT_U32 *realRegAddrPtr
)
{
    GT_U32  baseAddr;
    GT_UNUSED_PARAM(portGroup);

    if(regType > INTERNAL_REG)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        hwsAldrin2SerdesAddrCalc(serdesNum, regAddr, realRegAddrPtr);
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        hwsBobcat3SerdesAddrCalc(serdesNum, regAddr, realRegAddrPtr);
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)/* in sip 6 this function calling CB implemented by higher cpss logic */
    {
        hwsFalconSerdesAddrCalc(devNum, serdesNum, regAddr, realRegAddrPtr);
    }
    else
    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ||
       (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) ||
       hwsIsIronmanAsPhoenix())
    {
        hwsSip6_10SerdesAddrCalc(devNum, serdesNum, regAddr, regType, realRegAddrPtr);
    }
    else
    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
       (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E))
    {
        if(serdesNum > 11)
        {
            *realRegAddrPtr = 0x13000000 + regAddr + 0x1000*serdesNum + 0x40000;
        }
        else
        {
            *realRegAddrPtr = 0x13000000 + regAddr + 0x1000*serdesNum;
        }
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
    {
        baseAddr = (0 == regType) ? 0x13000000 : 0x13001000;
        *realRegAddrPtr = baseAddr + regAddr + (0x2000 * serdesNum);
    }
    else
    {
        baseAddr = (0 == regType) ? 0x13000000 : 0x13000800;

        *realRegAddrPtr = baseAddr + regAddr + 0x1000*serdesNum;
    }
    return GT_OK;
}

/**
* @internal prvCpssGenPortGroupSerdesWriteRegBitMask function
* @endinternal
*
* @brief   Implement write access to SERDES external/internal registers.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group number
* @param[in] regType                  - internal/external
* @param[in] serdesNum                - seredes number to access
* @param[in] regAddr                  - serdes register address (offset) to access
* @param[in] data                     -  to write
* @param[in] mask                     -  write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvCpssGenPortGroupSerdesWriteRegBitMask
(
    IN  GT_U8  devNum,
    IN  GT_U8  portGroup,
    IN  GT_U8  regType,
    IN  GT_U32 serdesNum,
    IN  GT_U32 regAddr,
    IN  GT_U32 data,
    IN  GT_U32 mask
)
{
    GT_U32    realRegAddr;
    GT_STATUS rc;

    rc = prvCpssGenPortGroupSerdesCalculateRegAddress(devNum, portGroup, regType, serdesNum, regAddr, &realRegAddr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return genRegisterSet(devNum,portGroup,realRegAddr,data,mask);
}
/* bobcat3 pipe1 address indication */
#define BOBCAT3_PIPE_1_INDICATION_CNS 0x80000000

/**
* @internal prvCpssGenPortGroupSerdesReadRegBitMask function
* @endinternal
*
* @brief   Implement read access from SERDES external/internal registers.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group number
* @param[in] regType                  - internal/external
* @param[in] serdesNum                - seredes number to access
* @param[in] regAddr                  - serdes register address (offset) to access
* @param[in] mask                     - read mask
*
* @param[out] dataPtr                  - read data
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssGenPortGroupSerdesReadRegBitMask
(
    IN  GT_U8  devNum,
    IN  GT_U8  portGroup,
    IN  GT_U8  regType,
    IN  GT_U32 serdesNum,
    IN  GT_U32 regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32 mask
)
{
    GT_U32  realRegAddr;
    GT_STATUS rc;

    rc = prvCpssGenPortGroupSerdesCalculateRegAddress(devNum, portGroup, regType, serdesNum, regAddr, &realRegAddr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return genRegisterGet(devNum,portGroup,realRegAddr,dataPtr,mask);
}




