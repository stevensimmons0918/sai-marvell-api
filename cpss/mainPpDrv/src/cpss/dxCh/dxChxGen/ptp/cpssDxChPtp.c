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
* @file cpssDxChPtp.c
*
* @brief CPSS DxCh Precision Time Protocol function implementations.
*
* @version   43
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/ptp/private/prvCpssDxChPtpLog.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/ptp/private/prvCpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* number of Policer Counter sets */
#define PRV_CPSS_DXCH_POLICER_MAX_COUNTER_INDEX_CNS     16

/* temporary unused parameter */
#define PRV_TMP_PARAM_NOT_USED_MAC(_p) (void)(_p)

/* check tai number */
#define PRV_CPSS_DXCH_TAI_CHECK_MAC(_dev, _taiNumber)                                           \
    if (_taiNumber >= PRV_CPSS_DXCH_PP_MAC(_dev)->hwInfo.ptpInfo.maxNumOfTais)                  \
    {                                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                          \
    }                                                                                           \

/* TAI Iterator LOOP Header */
#define PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(_dev, _taiIdPtr, _taiIterator)     \
    _taiIterator.state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_START_E;                   \
    while (1)                                                                            \
    {                                                                                    \
        {                                                                                \
            GT_STATUS __rc;                                                              \
            __rc = prvCpssDxChPtpNextTaiGet(_dev, _taiIdPtr, &_taiIterator);             \
            if (__rc != GT_OK)                                                           \
            {                                                                            \
                return __rc;                                                             \
            }                                                                            \
            if (_taiIterator.state == PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E)    \
            {                                                                            \
                break;                                                                   \
            }                                                                            \
        }

/* TAI Iterator LOOP footer */
#define PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(_dev, _taiIterator, _taiIdPtr)       \
    }

/* check that port has valid mapping or not CPU port */
#define PRV_CPSS_DXCH_PORT_SKIP_CHECK(dev,port)                                                  \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(dev) == GT_TRUE)                    \
    {                                                                                            \
        GT_BOOL   _isCpu, _isValid;                                                              \
        GT_STATUS _rc, _rc1;                                                                     \
        if ((port) >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))                                \
            break; /* no more */                                                                 \
        _rc = cpssDxChPortPhysicalPortMapIsCpuGet(dev, port, &_isCpu);                           \
        _rc1 = cpssDxChPortPhysicalPortMapIsValidGet(dev, port, &_isValid);                      \
        if((_rc != GT_OK) || (_isCpu == GT_TRUE) ||(_rc1 != GT_OK) || (_isValid != GT_TRUE ) )   \
            continue;                                                                            \
    }                                                                                            \
    else                                                                                         \
    {                                                                                            \
        if(PRV_CPSS_PP_MAC(dev)->phyPortInfoArray[port].portType == PRV_CPSS_PORT_NOT_EXISTS_E)  \
            continue;                                                                            \
    }
/* Converts packet PTP TS UDP Checksum Update Mode to hardware value */
#define PRV_CPSS_CONVERT_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_TO_HW_VAL_MAC(_val, _mode) \
    switch (_mode)                                                                  \
    {                                                                               \
        case CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E:                     \
            _val = 0;                                                               \
            break;                                                                  \
        case CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E:               \
            _val = 1;                                                               \
            break;                                                                  \
        case CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E:                      \
            _val = 3;                                                               \
            break;                                                                  \
        default:                                                                    \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                    \
    }

/* Converts PTP TS UDP Checksum Update Mode hardware value to SW value */
#define PRV_CPSS_CONVERT_HW_VAL_TO_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_MAC(_val, _mode) \
    switch (_val)                                                                   \
    {                                                                               \
        case 0:                                                                     \
            _mode = CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E;              \
            break;                                                                  \
        case 1:                                                                     \
            _mode = CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E;        \
            break;                                                                  \
        case 3:                                                                     \
            _mode = CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E;               \
            break;                                                                  \
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);                                               \
    }

/* Converts packet PTP Egress Exception Command to hardware value */
#define PRV_CPSS_CONVERT_PTP_EGRESS_EXCEPTION_CMD_TO_HW_VAL_MAC(_val, _cmd) \
    switch (_cmd)                                                           \
    {                                                                       \
        case CPSS_PACKET_CMD_FORWARD_E:                                     \
            _val = 0;                                                       \
            break;                                                          \
        case CPSS_PACKET_CMD_DROP_HARD_E:                                   \
            _val = 1;                                                       \
            break;                                                          \
        default:                                                            \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                            \
    }

/* check that port has valid mapping or not CPU port */
#define CPSS_ENABLER_PORT_SKIP_CHECK(dev,port)                                                   \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(dev) == GT_TRUE)                    \
    {                                                                                            \
        GT_BOOL   _isCpu, _isValid;                                                                        \
        GT_STATUS _rc, _rc1;                                                                           \
        if ((port) >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))                                \
            break; /* no more */                                                                 \
        _rc = cpssDxChPortPhysicalPortMapIsCpuGet(dev, port, &_isCpu);                           \
        _rc1 = cpssDxChPortPhysicalPortMapIsValidGet(dev, port, &_isValid);                      \
        if((_rc != GT_OK) || (_isCpu == GT_TRUE) ||(_rc1 != GT_OK) || (_isValid != GT_TRUE ) )   \
            continue;                                                                            \
    }                                                                                            \
    else                                                                                         \
    {                                                                                            \
        if(PRV_CPSS_PP_MAC(dev)->phyPortInfoArray[port].portType == PRV_CPSS_PORT_NOT_EXISTS_E)  \
            continue;                                                                            \
    }

/* Converts PTP Egress Exception Command  to hardware value */
#define PRV_CPSS_CONVERT_HW_VAL_TO_PTP_EGRESS_EXCEPTION_CMD_MAC(_val, _cmd) \
    switch (_val)                                                           \
    {                                                                       \
        case 0:                                                             \
            _cmd = CPSS_PACKET_CMD_FORWARD_E;                               \
            break;                                                          \
        case 1:                                                             \
            _cmd = CPSS_PACKET_CMD_DROP_HARD_E;                             \
            break;                                                          \
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);                                            \
    }

/* TAI Iterator Get Instance and check that it is Single */
#define PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(_dev, _taiIdPtr, _taiIterator)     \
    _taiIterator.state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_START_E;                   \
    {                                                                                    \
        GT_STATUS __rc;                                                                  \
        __rc = prvCpssDxChPtpNextTaiGet(_dev, _taiIdPtr, &_taiIterator);                 \
        if (__rc != GT_OK)                                                               \
        {                                                                                \
            return __rc;                                                                 \
        }                                                                                \
        if (PRV_CPSS_PP_MAC(_dev)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)           \
        {                                                                                \
            /* mark reading from single tai instance */                                  \
            _taiIterator.state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_LAST_E;      \
        }                                                                                \
        if (_taiIterator.state != PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_LAST_E)     \
        {                                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);               \
        }                                                                                \
    }

/*check pointer only for not single TAI devices*/
#define PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(_dev, _ptr)                                  \
    if ((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(_dev) != GT_FALSE)                             \
       && (PRV_CPSS_DXCH_PP_MAC(_dev)->hwInfo.gop_tai.supportSingleInstance == GT_FALSE))           \
    {                                                                                               \
        CPSS_NULL_PTR_CHECK_MAC(_ptr);                                                              \
    }

#define FALCON_TILE_TAI_START_INDEX(_tileId)     (FALCON_PER_TILE_TAI_SUB_UNITS_NUMBER_CNS * _tileId)

/* check TAIs bitmap selection */
#define PRV_CPSS_DXCH_PTP_TAI_SELECTION_BMP_CHECK_MAC(_dev,_bmp)                                \
    if (_bmp > (GT_U32)((1<<PRV_CPSS_DXCH_PP_MAC(_dev)->hwInfo.ptpInfo.maxNumOfTais)-1))        \
    {                                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);                       \
    }

/* GOP tile slave TAI clock frequency */
#define FALCON_PTP_RAVEN_CLK_CNS                     833333 /* 833.33333 MHz */

/* Master tile TXQ slave TAI clock frequency */
#define FALCON_PTP_TXQ_CLK_CNS                       800781 /* 800.78125 MHz */

/* Master tile CP slave TAI clock frequency */
#define FALCON_PTP_CP_CLK_CNS                        700120 /* 700.1201923 MHz */

/* MNG clock TAI frequency */
#define HAWK_PTP_MNG_CLK_CNS                        800000000 /* 800 MHz */

/* CP clock TAI frequency */
#define HAWK_PTP_CP_CLK_CNS                         812500000 /* 812.5 MHz */

/* PB clock TAI frequency */
#define HAWK_PTP_PB_CLK_CNS                         816667000 /* 816.667 MHz */

/* GOP MAC0 clock TAI frequency */
#define HAWK_PTP_GOP_MAC0_CLK_CNS                   843750000 /* 843.75 MHz */

/* GOP MAC1 clock TAI frequency */
#define HAWK_PTP_GOP_MAC1_CLK_CNS                   562500000 /* 562.5 MHz */

/* GOP app clock TAI frequency */
#define HAWK_PTP_GOP_APP_CLK_CNS                    843750000 /* 843.75 MHz */

#define PHOENIX_PTP_CP_CLK_CNS                      510416500 /* 510.4165 MHz */

#define PHOENIX_PTP_TXQ_CLK_CNS                     600000000 /* 600 MHz */

#define PHOENIX_PTP_APP_CLK_CNS                     650000000/* 650 MHz */

#define PHOENIX_PTP_MAC0_CLK_CNS                    800000000 /* 800 MHz */

#define PHOENIX_PTP_MAC1_CLK_CNS                    533333000 /*  533.333 MHz */

#define PHOENIX_PTP_MAC2_CLK_CNS                    650000000 /* 650 MHz */

/* CP clock TAI frequency */
#define HARRIER_PTP_CP_CLK_CNS                      800000000 /* 800 MHz */

/* PB clock TAI frequency */
#define HARRIER_PTP_PB_CLK_CNS                      816667000 /* 816.667 MHz */

/* APP clock TAI frequency */
#define HARRIER_PTP_APP_CLK_CNS                     843750000 /* 843.75 MHz */

/* Support for multi instance debugging; global variables macros*/

#define PRV_SHARED_PTP_DIR_PTP_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.ptpDir.ptpSrc._var,_value)

#define PRV_SHARED_PTP_DIR_PTP_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ptpDir.ptpSrc._var)

/**
* @internal prvCpssDxChPtpNextTaiGet function
* @endinternal
*
* @brief   Gets next TAI subunit.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Id.
*                                      For single TAI devices taiIdPtr ignored,
*                                      iterator state evaluation is START_E => VALID_LAST => NO_MORE.
* @param[in,out] iteratorPtr              - (pointer to) TAI iterator
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvCpssDxChPtpNextTaiGet
(
    IN    GT_U8                               devNum,
    IN    CPSS_DXCH_PTP_TAI_ID_STC            *taiIdPtr,
    INOUT PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC  *iteratorPtr
)
{
    GT_STATUS rc;                        /* return code                  */
    GT_U32    gopsAmount;                /* amount of GOPs               */
    GT_U32    globalGop;                 /* Global TAI GOP index         */
    GT_U32    bmp;                       /* work port group bitmap       */
    GT_U32    i;                         /* loop index                   */
    GT_BOOL   isPortGroupLast;           /* is Port Group Last           */
    GT_U32    portMacNum;                /* MAC number                   */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance != GT_FALSE)
    {
        switch (iteratorPtr->state)
        {
            case PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_START_E:
                iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_LAST_E;
                iteratorPtr->gopNumber = 0;
                iteratorPtr->taiNumber = 0;
                PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
                    devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, (iteratorPtr->portGroupId));
                return GT_OK;
            case PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_LAST_E:
                iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
                return GT_OK;
            default:
                iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        gopsAmount = 0; /* BobCat3 - GOP not relevant */
        globalGop  = 0; /* BobCat3 - GOP not relevant */
    }
    else
    {
        gopsAmount = 10; /* BobCat2 amount of GOPs + ILKN */
        globalGop  = 9; /* BobCat2 Global TAI ILKN index */
    }

    switch (iteratorPtr->state)
    {
        case PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_START_E:
            /* calculate iteratorPtr->taiNumber */
            switch (taiIdPtr->taiNumber)
            {
                case CPSS_DXCH_PTP_TAI_NUMBER_0_E:
                    iteratorPtr->taiNumber = 0;
                    break;
                case CPSS_DXCH_PTP_TAI_NUMBER_1_E:
                    iteratorPtr->taiNumber = 1;
                    break;
                case CPSS_DXCH_PTP_TAI_NUMBER_2_E:
                    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E)
                    {
                        iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                    iteratorPtr->taiNumber = 2;
                    break;
                case CPSS_DXCH_PTP_TAI_NUMBER_3_E:
                    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E)
                    {
                        iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                    iteratorPtr->taiNumber = 3;
                    break;
                case CPSS_DXCH_PTP_TAI_NUMBER_4_E:
                    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E)
                    {
                        iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                    iteratorPtr->taiNumber = 4;
                    break;
                case CPSS_DXCH_PTP_TAI_NUMBER_ALL_E:
                    iteratorPtr->taiNumber = 0;
                    break;
                default:
                    iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* TAI instance not relevant for BC3 */
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) != GT_TRUE)
            {
                /* calculate iteratorPtr->gopNumber */
                switch (taiIdPtr->taiInstance)
                {
                    case CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E:
                        iteratorPtr->gopNumber = 0;
                        PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
                            devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, (iteratorPtr->portGroupId));
                        iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_E;
                        break;
                    case CPSS_DXCH_PTP_TAI_INSTANCE_GLOBAL_E:
                        iteratorPtr->gopNumber = globalGop;
                        if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
                        {
                            /* the last port group */
                            bmp = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp;
                            if (bmp == 0)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* must not occure */
                            }
                            /* look for the biggest non-zero bit */
                            for (i = 31; ((bmp & (1 << i)) == 0); i--) {};
                            iteratorPtr->portGroupId = i;

                        }
                        else
                        {
                            iteratorPtr->portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
                        }
                        break;
                    case CPSS_DXCH_PTP_TAI_INSTANCE_PORT_E:
                        /* state will be overriden below after successfull port checking */
                        iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
                        PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,taiIdPtr->portNum,portMacNum);
                        rc = prvCpssDxChHwPpPortToGopConvert(
                            devNum, portMacNum,
                            &(iteratorPtr->portGroupId), &(iteratorPtr->gopNumber));
                        if (rc != GT_OK)
                        {
                            iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
                            return rc;
                        }
                        break;
                    default:
                        iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                iteratorPtr->gopNumber = (PRV_SHARED_PTP_DIR_PTP_SRC_GLOBAL_VAR_GET(debugInstanceEnabled))?PRV_SHARED_PTP_DIR_PTP_SRC_GLOBAL_VAR_GET(debugtaiIterator.gopNumber):0;
            }

            /* calculate iteratorPtr->state */
            if (((taiIdPtr->taiInstance == CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E)
                 && (gopsAmount > 1))
                || (taiIdPtr->taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E))
            {
                iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_E;
            }
            else
            {
                iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_LAST_E;
            }
            return GT_OK;

        case PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_E:
            if (taiIdPtr->taiInstance != CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E)
            {
                if ((taiIdPtr->taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)
                    && (iteratorPtr->taiNumber == 0))
                {
                    iteratorPtr->taiNumber = 1;
                    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E)
                        iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_LAST_E;
                    return GT_OK;
                }
                else if ((taiIdPtr->taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)
                    && (iteratorPtr->taiNumber == 1))
                {
                    iteratorPtr->taiNumber = 2;
                    return GT_OK;
                }
                else if ((taiIdPtr->taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)
                    && (iteratorPtr->taiNumber == 2))
                {
                    iteratorPtr->taiNumber = 3;
                    return GT_OK;
                }
                else if ((taiIdPtr->taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)
                    && (iteratorPtr->taiNumber == 3))
                {
                    iteratorPtr->taiNumber = 4;
                    iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_LAST_E;
                    return GT_OK;
                }
                else
                {
                    iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            else /*(taiIdPtr->taiInstance == CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E)*/
            {
                bmp = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp;
                if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
                {
                    /* check is the port group last */
                    isPortGroupLast =
                        ((bmp & (0xFFFFFFFF << (iteratorPtr->portGroupId + 1))) == 0)
                            ? GT_TRUE : GT_FALSE;
                }
                else
                {
                    isPortGroupLast = GT_TRUE;
                }

                /* next TAI number */
                if (taiIdPtr->taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)
                {
                    if (iteratorPtr->taiNumber == 0)
                    {
                        iteratorPtr->taiNumber = 1;
                        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E)
                        {
                            iteratorPtr->state =
                                (((iteratorPtr->gopNumber + 1) >= gopsAmount)
                                    && (isPortGroupLast == GT_TRUE))
                                        ? PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_LAST_E
                                        : PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_E;
                        }
                        return GT_OK;
                    }
                    else if (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                            iteratorPtr->taiNumber = 0;
                    }
                    else if (iteratorPtr->taiNumber == 1)
                    {
                        iteratorPtr->taiNumber = 2;
                        return GT_OK;
                    }
                    else if (iteratorPtr->taiNumber == 2)
                    {
                        iteratorPtr->taiNumber = 3;
                        return GT_OK;
                    }
                    else if (iteratorPtr->taiNumber == 3)
                    {
                        iteratorPtr->taiNumber = 4;
                        iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_LAST_E;
                        return GT_OK;
                    }
                    else
                    {
                        iteratorPtr->taiNumber = 0;
                    }
                }
                else
                {
                    if (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        iteratorPtr->taiNumber = (taiIdPtr->taiNumber != CPSS_DXCH_PTP_TAI_NUMBER_1_E) ? 0 : 1;
                    }
                    else
                    {
                        switch (taiIdPtr->taiNumber)
                        {
                        case CPSS_DXCH_PTP_TAI_NUMBER_0_E:
                            iteratorPtr->taiNumber = 0;
                            break;
                        case CPSS_DXCH_PTP_TAI_NUMBER_1_E:
                            iteratorPtr->taiNumber = 1;
                            break;
                        case CPSS_DXCH_PTP_TAI_NUMBER_2_E:
                            iteratorPtr->taiNumber = 2;
                            break;
                        case CPSS_DXCH_PTP_TAI_NUMBER_3_E:
                            iteratorPtr->taiNumber = 3;
                            break;
                        case CPSS_DXCH_PTP_TAI_NUMBER_4_E:
                            iteratorPtr->taiNumber = 4;
                            break;
                        default:
                            iteratorPtr->taiNumber = 0;
                            break;
                        }
                    }
                }

                /* next GOP number */
                iteratorPtr->gopNumber++;
                if ((iteratorPtr->gopNumber + 1) < gopsAmount)
                {
                    iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_E;
                    return GT_OK;
                }
                if ((iteratorPtr->gopNumber + 1) == gopsAmount)
                {
                    if (isPortGroupLast == GT_TRUE)
                    {
                        iteratorPtr->state =
                            (taiIdPtr->taiNumber != CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)
                                ? PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_LAST_E
                                : PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_E;
                        return GT_OK;
                    }
                    else
                    {
                        /* next port group */
                        /* look for the next non-zero bit */
                        for (i = (iteratorPtr->portGroupId + 1);
                              ((bmp & (1 << i)) == 0); i++) {};
                        iteratorPtr->portGroupId = i;
                        iteratorPtr->gopNumber = 0;
                        iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_E;
                        return GT_OK;
                    }
                }
                /* error state */
                iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_LAST_E:
            iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
            return GT_OK;
        case PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E:
        default:
            iteratorPtr->state = PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /*return GT_OK;*/
}

/**
* @internal prvCpssDxChPtpPLLMsi5nmConfig function
* @endinternal
*
* @brief   config the PTP PLL MSI 5 nm parameters to support different frequencies.
*
* @note   APPLICABLE DEVICES:     Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] clockFrequency           - REF_CLK_inputs options in Mhz
* @param[in] taiNumber                - TAI Number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
static GT_STATUS prvCpssDxChPtpPLLMsi5nmConfig
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT         clockFrequency
)
{
    GT_STATUS   rc;
    GT_U32 pllConfigRegAddr;
    GT_U32 pllParamsRegAddr;
    GT_U32 regAddr;
    GT_U32 K;
    GT_U32 M;
    GT_U32 N;
    GT_U32 updateRate;
    GT_U32 icp;
    GT_U32 bw;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                                  CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E |
                                                  CPSS_IRONMAN_E);
    switch (clockFrequency)
    {
    case CPSS_DXCH_PTP_10_FREQ_E:
    case CPSS_DXCH_PTP_20_FREQ_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    case CPSS_DXCH_PTP_25_FREQ_E:
            K = 3;
            M = 1;
            N = (120 << 16); /* 9 MSBs + 16 LSBs = '0' */
            updateRate = 250;
            icp = 3;
            bw = 2;
            break;
    case CPSS_DXCH_PTP_156_25_FREQ_E: /* 156.25 */
            K = 3;
            M = 5;
            N = (96 << 16); /* 9 MSBs + 16 LSBs = '0' */
            updateRate = 313;
            icp = 6;
            bw = 3;
            break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* PLL MSI PTP disable channel 1 */
    pllConfigRegAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.pllPTPConfig;
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 20, 1, 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* PLL MSI PTP power down */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 21, 1, 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* PLL MSI PTP no-reset */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 22, 1, 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* PLL MSI PTP Enable Using the configurations from the RF instead of the default */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 30, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* PLL MSI PTP reset */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 22, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    pllParamsRegAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.pllPTPParams;
    /* K */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllParamsRegAddr, 0, 9, K);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* M */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllParamsRegAddr, 9, 6, M);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* update rate */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllParamsRegAddr, 15, 11, updateRate);
    if (rc != GT_OK)
    {
        return rc;
    }

    pllParamsRegAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.pllPTPParams1;
    /* N */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllParamsRegAddr, 0, 25, N);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* icp */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllParamsRegAddr, 25, 4, icp);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* bw */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllParamsRegAddr, 29, 3, bw);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* PLL MSI PTP Use the configurations from the RF instead of the default */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 29, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* PLL MSI PTP enable channel 1 */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 20, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* PLL MSI PTP power up */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 21, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsTimerWkAfter(1); /* time to sleep in milliseconds */

    /* PLL MSI PTP no-reset */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 22, 1, 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsTimerWkAfter(1); /* time to sleep in milliseconds */

    rc = cpssDxChPtpPLLBypassEnableSet(devNum,GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*Toggle PTP clock from MISC0 PLL to PTP PLL*/
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl5;
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 17, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPtpPLLConfig function
* @endinternal
*
* @brief   config the PTP PLL parameters to support different frequencies.
*
* @note   APPLICABLE DEVICES:     Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5; AC3X; Bobcat2; Caelum; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] clockFrequency           - PTP PLL frequency
* @param[in] taiNumber                - TAI Number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
static GT_STATUS prvCpssDxChPtpPLLConfig
(
    IN    GT_U8                                devNum,
    IN CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT        clockFrequency,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber
)
{
    GT_STATUS   rc;
    GT_U32 pllConfigRegAddr;
    GT_U32 regAddr;
    GT_U32 hwValue;
    GT_U32 K;
    GT_U32 M;
    GT_U32 N;
    GT_U32 vcoBand;
    GT_U32 lpfCtrl;
    GT_U32 taiOffset;


    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_IRONMAN_E);

    /* configure Harrier PTP PLL MSI 5nm; PTP PLL is powered down by default */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
    {
        return prvCpssDxChPtpPLLMsi5nmConfig(devNum,clockFrequency);
    }

    /* Default Frequency of the PTP PLL clock */
    if (clockFrequency == CPSS_DXCH_PTP_25_FREQ_E)
    {
        return GT_OK;
    }

    switch (clockFrequency)
    {
    case CPSS_DXCH_PTP_10_FREQ_E:
            K = 1;
            M = 1;
            N = 99;
            vcoBand = 2;
            lpfCtrl = 4;
            break;
    case CPSS_DXCH_PTP_20_FREQ_E:
            K = 2;
            M = 1;
            N = 99;
            vcoBand = 2;
            lpfCtrl = PRV_CPSS_SIP_6_CHECK_MAC(devNum)? 5:4;
            break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* pll bypass */
        pllConfigRegAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnitsDeviceSpecificRegs.pllPTPConfig;
        hwValue = BOOL2BIT_MAC(GT_TRUE);
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 0, 1, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set pll params */
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnitsDeviceSpecificRegs.pllPTPParams;
        /* K */
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 0, 3, K);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* M */
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 3, 9, M);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* N */
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 12, 9, N);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* VCO band */
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 21, 4, vcoBand);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* lpfCtrl */
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 25, 4, lpfCtrl);
        if (rc != GT_OK)
        {
            return rc;
        }


        /*Set PTP PLL Reconfiguration Enable */
        hwValue = BOOL2BIT_MAC(GT_TRUE);
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 9, 1, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* clear pll bypass */
        hwValue = BOOL2BIT_MAC(GT_FALSE);
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 0, 1, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /*Toggle PTP clock from MISC0 PLL to PTP PLL*/
        if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnitsDeviceSpecificRegs.deviceCtrl19;
            taiOffset = (taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_0_E)?14:17;
        }
        else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnitsDeviceSpecificRegs.deviceCtrl13;
            taiOffset = (taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_0_E)?16:19;
        }
        else if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnitsDeviceSpecificRegs.deviceCtrl15;
            taiOffset = (taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_0_E)?4:7;
        }
        else
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;
            taiOffset = (taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_0_E)?9:12;
        }
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, taiOffset, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }

        hwValue = BOOL2BIT_MAC(GT_TRUE);
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, pllConfigRegAddr, 9, 1, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        return GT_OK;
    }
    else if (PRV_CPSS_SIP_5_16_CHECK_MAC(devNum))
    {
        if (clockFrequency == CPSS_DXCH_PTP_20_FREQ_E)
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                    DFXServerUnitsDeviceSpecificRegs.deviceCtrl7;

            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 31, 1, 1);
            if (rc !=GT_OK)
            {
                return rc;
            }

            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                    DFXServerUnitsDeviceSpecificRegs.deviceCtrl3;

            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 27, 1, 1);
            if (rc !=GT_OK)
            {
                return rc;
            }
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 0, 1, 1);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 12, 1, 1);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 24, 1, 1);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 24, 1, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 12, 1, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        if (clockFrequency == CPSS_DXCH_PTP_10_FREQ_E)
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                 DFXServerUnitsDeviceSpecificRegs.deviceCtrl3;

            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 0, 32, 0x04000001);
            if (rc != GT_OK)
            {
                return rc;
            }
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                 DFXServerUnitsDeviceSpecificRegs.deviceCtrl7;

            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 0, 32, 0xAE0837fE);
            if (rc != GT_OK)
            {
                return rc;
            }
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                 DFXServerUnitsDeviceSpecificRegs.pllPTPParams;

            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 0, 32, 0x2332B011);
            if (rc != GT_OK)
            {
                return rc;
            }
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                 DFXServerUnitsDeviceSpecificRegs.pllPTPConfig;

            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 0, 32, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}
/**
* @internal prvCpssDxChPtpTaiDebugInstanceSet function
* @endinternal
*
* @brief   Sets the TAI debug instance.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               - device number
* @param[in] tileId               - Processing die identifier
* @param[in] taiInstanceType      - Type of the instance
*                                   (Master, CP_Slave, TXQ_Slave, Chiplet_Slave )
* @param[in] instanceId           - TAI instance identifier for a particular tile.
*                                   Master              - Not used.
*                                   CP_Slave,TXQ_Slave  - 0..1 - processing pipe number.
*                                   Chiplet_Slave       - 0..3 - local GOP die number of specific tile.
*
* @retval GT_OK                   - on success
* @retval GT_BAD_PARAM            - on wrong parameter
*/
GT_STATUS prvCpssDxChPtpTaiDebugInstanceSet
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    tileId,
    IN    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_ENT   taiInstanceType,
    IN    GT_U32                                    instanceId
)
{
    GT_U32 indexOffset;
    GT_U32 maxValidInstanceId;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E);

    if(tileId && tileId >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(taiInstanceType)
    {
        /* Calculate GOP number and tai number index
         * Total [36] pairs (9TAI per tile)
         * Each Eagle tile Indexing[0-8]
         * ========================
         * Master 1pair  [0]
         * EPCL   2pair  [1-2]
         * TXQ    2pair  [3-4]
         * Raven  4pair  [5-8]
         */
        case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_MASTER_E:
            indexOffset = 0;
            maxValidInstanceId = 1;
            break;

        case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_CP_SLAVE_E:
            indexOffset = 1 + instanceId;
            maxValidInstanceId = 2;
            break;

        case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_TXQ_SLAVE_E:
            indexOffset = 3 + instanceId;
            maxValidInstanceId = 2;
            break;

        case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_CHIPLET_SLAVE_E:
            indexOffset = 5 + instanceId;
            maxValidInstanceId = 4;
            break;

        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Check the InstanceID range according to the type of instance */
    if(instanceId >= maxValidInstanceId)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Set the global TAI debug indicator */
    PRV_SHARED_PTP_DIR_PTP_SRC_GLOBAL_VAR_SET(debugInstanceEnabled,GT_TRUE);
    PRV_SHARED_PTP_DIR_PTP_SRC_GLOBAL_VAR_SET(debugtaiIterator.gopNumber,FALCON_TILE_TAI_START_INDEX(tileId) + indexOffset);

    return GT_OK;
}

/**
* @internal prvCpssDxChPtpTaiClockEdgeSet function
* @endinternal
*
* @brief  Set the Clock Reception/Generation Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] edge                  - Rising edge time of Reception/Generation clock, in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS prvCpssDxChPtpTaiClockEdgeSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC                     *taiIdPtr,
    IN GT_U32                                       edge,
    IN PRV_CPSS_DXCH_PTP_TAI_CLOCK_CONTROL_TYPE_ENT clockType
)
{
    GT_STATUS                          rc;                   /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;          /* TAI iterator     */
    GT_U32                             regAddrClockControl;  /* Clock Control Register address */
    GT_U32                             regAddrClockEdge;     /* Reception/Generation edge register address */
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT restoreCntrFunc;      /* counter function restore */
    CPSS_PORT_DIRECTION_ENT            direction = CPSS_PORT_DIRECTION_BOTH_E; /* DUMMY for SIP6*/
    GT_U32                             phaseOffset;


    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    phaseOffset = (clockType == PRV_CPSS_DXCH_PTP_TAI_CLOCK_CONTROL_TYPE_GEN_E)?4:5;

    /* Process:
     * 1. Store the Time Counter Function Control(Need to restore at the end of this API)
     * 2. Configure the Clock Generation/Reception Edge
     * 3. Set the Clock Generation/Reception Phase set
     * 4. Configure Time Counter Function Control to "NOP"
     * 5. Set the Time Counter Function Control Trigger
     * 6. Clear the Clock Generation/Reception Phase set
     * 7. Restore the Time Counter Function Control(from step 1)
     */

    /* 1. Store the Time Counter Function Control(Need to restore at the end of this API)*/
    rc = cpssDxChPtpTodCounterFunctionGet(
            devNum, direction, taiIdPtr, &restoreCntrFunc);
    if (rc != GT_OK)
    {
        return rc;
    }


    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        if(clockType == PRV_CPSS_DXCH_PTP_TAI_CLOCK_CONTROL_TYPE_GEN_E)
        {
            regAddrClockEdge = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                    devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockGenerationEdge;
        }
        else
        {
            regAddrClockEdge = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                    devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockReceptionEdge;
        }
        regAddrClockControl = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockControl;

        /* 2. Configure the Clock Reception/Generation Edge */
        rc = prvCpssDrvHwPpPortGroupSetRegField(
                devNum, taiIterator.portGroupId, regAddrClockEdge, 0, 32, edge);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* 3. Set the Clock Reception/Generation Phase set */
        rc = prvCpssDrvHwPpPortGroupSetRegField(
                devNum, taiIterator.portGroupId, regAddrClockControl, phaseOffset, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    /* 4. Configure Time Counter Function Control to "NOP" */
    rc = cpssDxChPtpTodCounterFunctionSet(
            devNum, direction, taiIdPtr, CPSS_DXCH_PTP_TOD_COUNTER_FUNC_NOP_E);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* 5. Set the Time Counter Function Control Trigger */
    rc = cpssDxChPtpTodCounterFunctionTriggerSet(devNum, direction, taiIdPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {

        regAddrClockControl = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockControl;

        /* 6. Clear the Clock Reception Phase set */
        rc = prvCpssDrvHwPpPortGroupSetRegField(
                devNum, taiIterator.portGroupId, regAddrClockControl, phaseOffset, 1, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)



    /* 7. Restore the Time Counter Function Control(from step 1)*/
    rc = cpssDxChPtpTodCounterFunctionSet(devNum, direction, taiIdPtr, restoreCntrFunc);
    if (rc != GT_OK)
    {
        return rc;
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddrClockControl = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockControl;
        rc = prvCpssDrvHwPpPortGroupSetRegField(
                devNum, taiIterator.portGroupId, regAddrClockControl, 2, 1, 1);
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal internal_cpssDxChPtpEtherTypeSet function
* @endinternal
*
* @brief   Configure ethertype0/ethertype1 of PTP over Ethernet packets.
*         A packet is identified as PTP over Ethernet if its EtherType matches
*         one of the configured values.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] etherTypeIndex           - PTP EtherType index.
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] etherType                - PTP EtherType0 or EtherType1 according to the index.
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_OUT_OF_RANGE          - on wrong etherType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpEtherTypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherTypeIndex,
    IN GT_U32   etherType
)
{
    GT_U32    regAddr;          /* register address                 */
    GT_U32    fieldOffset;      /* field offset inside register     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    if (etherTypeIndex > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (etherType > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* BobCat2 and above devices */
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPEtherTypes;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ptpEthertypes;
    }

    fieldOffset = (etherTypeIndex * 16);

    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 16,
                                     etherType);
}

/**
* @internal cpssDxChPtpEtherTypeSet function
* @endinternal
*
* @brief   Configure ethertype0/ethertype1 of PTP over Ethernet packets.
*         A packet is identified as PTP over Ethernet if its EtherType matches
*         one of the configured values.
*
* @note   APPLICABLE DEVICES:       Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] etherTypeIndex           - PTP EtherType index.
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] etherType                - PTP EtherType0 or EtherType1 according to the index.
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_OUT_OF_RANGE          - on wrong etherType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpEtherTypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherTypeIndex,
    IN GT_U32   etherType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpEtherTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, etherTypeIndex, etherType));

    rc = internal_cpssDxChPtpEtherTypeSet(devNum, etherTypeIndex, etherType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, etherTypeIndex, etherType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpEtherTypeGet function
* @endinternal
*
* @brief   Get ethertypes of PTP over Ethernet packets.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] etherTypeIndex           - PTP EtherType index.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] etherTypePtr             - (pointer to) PTP EtherType0 or EtherType1 according
*                                      to the index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpEtherTypeGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   etherTypeIndex,
    OUT GT_U32   *etherTypePtr
)
{
    GT_U32      regAddr;        /* register address                 */
    GT_U32      fieldOffset;    /* field offset inside register     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(etherTypePtr);

    if (etherTypeIndex > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* BobCat2 and above devices */
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPEtherTypes;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ptpEthertypes;
    }

    fieldOffset = (etherTypeIndex * 16);

    return prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 16,
                                     etherTypePtr);
}

/**
* @internal cpssDxChPtpEtherTypeGet function
* @endinternal
*
* @brief   Get ethertypes of PTP over Ethernet packets.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] etherTypeIndex           - PTP EtherType index.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] etherTypePtr             - (pointer to) PTP EtherType0 or EtherType1 according
*                                      to the index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpEtherTypeGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   etherTypeIndex,
    OUT GT_U32   *etherTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpEtherTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, etherTypeIndex, etherTypePtr));

    rc = internal_cpssDxChPtpEtherTypeGet(devNum, etherTypeIndex, etherTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, etherTypeIndex, etherTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpUdpDestPortsSet function
* @endinternal
*
* @brief   Configure UDP destination port0/port1 of PTP over UDP packets.
*         A packet is identified as PTP over UDP if it is a UDP packet, whose
*         destination port matches one of the configured ports.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] udpPortIndex             - UDP port index.
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] udpPortNum               - UDP port1/port0 number according to the index,
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or udpPortIndex
* @retval GT_OUT_OF_RANGE          - on wrong udpPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpUdpDestPortsSet
(
    IN GT_U8    devNum,
    IN GT_U32   udpPortIndex,
    IN GT_U32   udpPortNum
)
{
    GT_U32    regAddr;          /* register address                 */
    GT_U32    fieldOffset;      /* field offset inside register     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    if (udpPortIndex > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (udpPortNum > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* BobCat2 and above devices */
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPOverUDPDestinationPorts;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ptpUdpDstPorts;
    }

    fieldOffset = (udpPortIndex * 16);

    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 16,
                                     udpPortNum);
}

/**
* @internal cpssDxChPtpUdpDestPortsSet function
* @endinternal
*
* @brief   Configure UDP destination port0/port1 of PTP over UDP packets.
*         A packet is identified as PTP over UDP if it is a UDP packet, whose
*         destination port matches one of the configured ports.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] udpPortIndex             - UDP port index.
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] udpPortNum               - UDP port1/port0 number according to the index,
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or udpPortIndex
* @retval GT_OUT_OF_RANGE          - on wrong udpPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpUdpDestPortsSet
(
    IN GT_U8    devNum,
    IN GT_U32   udpPortIndex,
    IN GT_U32   udpPortNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpUdpDestPortsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, udpPortIndex, udpPortNum));

    rc = internal_cpssDxChPtpUdpDestPortsSet(devNum, udpPortIndex, udpPortNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, udpPortIndex, udpPortNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpUdpDestPortsGet function
* @endinternal
*
* @brief   Get UDP destination port0/port1 of PTP over UDP packets.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] udpPortIndex             - UDP port index
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] udpPortNumPtr            - (pointer to) UDP port0/port1 number, according
*                                      to the index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or udpPortIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpUdpDestPortsGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   udpPortIndex,
    OUT GT_U32   *udpPortNumPtr
)
{
    GT_U32      regAddr;        /* register address                 */
    GT_U32      fieldOffset;    /* field offset inside register     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(udpPortNumPtr);

    if (udpPortIndex > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* BobCat2 and above devices */
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPOverUDPDestinationPorts;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ptpUdpDstPorts;
    }

    fieldOffset = (udpPortIndex * 16);

    return prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 16,
                                     udpPortNumPtr);
}

/**
* @internal cpssDxChPtpUdpDestPortsGet function
* @endinternal
*
* @brief   Get UDP destination port0/port1 of PTP over UDP packets.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] udpPortIndex             - UDP port index
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] udpPortNumPtr            - (pointer to) UDP port0/port1 number, according
*                                      to the index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or udpPortIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpUdpDestPortsGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   udpPortIndex,
    OUT GT_U32   *udpPortNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpUdpDestPortsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, udpPortIndex, udpPortNumPtr));

    rc = internal_cpssDxChPtpUdpDestPortsGet(devNum, udpPortIndex, udpPortNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, udpPortIndex, udpPortNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpMessageTypeCmdSet function
* @endinternal
*
* @brief   Configure packet command per PTP message type.
*         The message type is extracted from the PTP header.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] command                  - assigned  to the packet.
*                                      (APPLICABLE VALUES: FORWARD, MIRROR, TRAP, HARD_DROP, SOFT_DROP)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpMessageTypeCmdSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   domainIndex,
    IN GT_U32                   messageType,
    IN CPSS_PACKET_CMD_ENT      command
)
{
    GT_U32    regAddr;          /* register address                 */
    GT_U32    fieldOffset;      /* field offset inside register     */
    GT_U32    hwValue;          /* value to write to hw             */
    CPSS_DXCH_TABLE_ENT tableType;/* table type */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    if (messageType > 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(hwValue, command);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {

        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

        if (domainIndex > 4)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            tableType = CPSS_DXCH_SIP5_20_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E;
            /* continues messageType 3 bits command for each of 5 domains */
            fieldOffset = (15 * messageType) + (3 * domainIndex) ;
        }
        else
        {
            tableType = CPSS_DXCH_SIP5_TABLE_TTI_PTP_COMMAND_E;
            /* for each messageType 3 bits command for each of 5 domains an one reserved bit */
            fieldOffset = (16 * messageType) + (3 * domainIndex) ;
        }

        return prvCpssDxChWriteTableEntryField(
            devNum, tableType,
            portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            fieldOffset, 3 /*fieldLength*/, hwValue);
    }

    fieldOffset = (messageType % 10) * 3;

    if (messageType / 10)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        ttiRegs.ptpPacketCmdCfg1Reg;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        ttiRegs.ptpPacketCmdCfg0Reg;
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 3, hwValue);
}

/**
* @internal cpssDxChPtpMessageTypeCmdSet function
* @endinternal
*
* @brief   Configure packet command per PTP message type.
*         The message type is extracted from the PTP header.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] command                  - assigned  to the packet.
*                                      (APPLICABLE VALUES: FORWARD, MIRROR, TRAP, HARD_DROP, SOFT_DROP)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpMessageTypeCmdSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   domainIndex,
    IN GT_U32                   messageType,
    IN CPSS_PACKET_CMD_ENT      command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpMessageTypeCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, domainIndex, messageType, command));

    rc = internal_cpssDxChPtpMessageTypeCmdSet(devNum, portNum, domainIndex, messageType, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, domainIndex, messageType, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpMessageTypeCmdGet function
* @endinternal
*
* @brief   Get packet command per PTP message type.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] commandPtr               - (pointer to) assigned command to the packet.
*                                      (APPLICABLE VALUES: FORWARD, MIRROR, TRAP, HARD_DROP, SOFT_DROP)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpMessageTypeCmdGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   domainIndex,
    IN  GT_U32                   messageType,
    OUT CPSS_PACKET_CMD_ENT      *commandPtr
)
{
    GT_U32    regAddr;          /* register address                 */
    GT_U32    fieldOffset;      /* field offset inside register     */
    GT_U32    hwValue;          /* value to read from hw            */
    GT_U32    rc;               /* return code                      */
    CPSS_DXCH_TABLE_ENT tableType;/* table type */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(commandPtr);

    if (messageType > 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {

        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

        if (domainIndex > 4)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            tableType = CPSS_DXCH_SIP5_20_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E;
            /* continues messageType 3 bits command for each of 5 domains */
            fieldOffset = (15 * messageType) + (3 * domainIndex) ;
        }
        else
        {
            tableType = CPSS_DXCH_SIP5_TABLE_TTI_PTP_COMMAND_E;
            /* for each messageType 3 bits command for each of 5 domains an one reserved bit */
            fieldOffset = (16 * messageType) + (3 * domainIndex) ;
        }

        rc = prvCpssDxChReadTableEntryField(
            devNum, tableType,
            portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            fieldOffset, 3 /*fieldLength*/, &hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*commandPtr, hwValue);

        return GT_OK;
    }

    fieldOffset = (messageType % 10) * 3;

    if (messageType / 10)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        ttiRegs.ptpPacketCmdCfg1Reg;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        ttiRegs.ptpPacketCmdCfg0Reg;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 3, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*commandPtr, hwValue);

    return GT_OK;
}

/**
* @internal cpssDxChPtpMessageTypeCmdGet function
* @endinternal
*
* @brief   Get packet command per PTP message type.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] commandPtr               - (pointer to) assigned command to the packet.
*                                      (APPLICABLE VALUES: FORWARD, MIRROR, TRAP, HARD_DROP, SOFT_DROP)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpMessageTypeCmdGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   domainIndex,
    IN  GT_U32                   messageType,
    OUT CPSS_PACKET_CMD_ENT      *commandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpMessageTypeCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, domainIndex, messageType, commandPtr));

    rc = internal_cpssDxChPtpMessageTypeCmdGet(devNum, portNum, domainIndex, messageType, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, domainIndex, messageType, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpCpuCodeBaseSet function
* @endinternal
*
* @brief   Set CPU code base assigned to trapped and mirrored PTP packets.
*         A PTP packet's CPU code is computed as base CPU code + messageType
*         (where message type is taken from PTP header).
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                - device number
* @param[in] cpuCode               - The base of CPU code assigned to PTP packets mirrored
*                                      or trapped to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpCpuCodeBaseSet
(
    IN GT_U8                        devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT     cpuCode
)
{
    GT_U32                                  regAddr;    /* register address */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode; /* DSA code */
    GT_STATUS                               rc;         /* return code */
    GT_U32                                  offset;     /* bit offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode, &dsaCpuCode);
    if( rc != GT_OK )
    {
        return rc;
    }

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPExceptionsAndCPUCodeConfig;
        offset  = 14;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ptpPacketCmdCfg1Reg;
        offset  = 18;
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, offset, 8, (GT_U32)dsaCpuCode);
}

/**
* @internal cpssDxChPtpCpuCodeBaseSet function
* @endinternal
*
* @brief   Set CPU code base assigned to trapped and mirrored PTP packets.
*         A PTP packet's CPU code is computed as base CPU code + messageType
*         (where message type is taken from PTP header).
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - The base of CPU code assigned to PTP packets mirrored
*                                      or trapped to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpCpuCodeBaseSet
(
    IN GT_U8                        devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT     cpuCode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpCpuCodeBaseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCode));

    rc = internal_cpssDxChPtpCpuCodeBaseSet(devNum, cpuCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpCpuCodeBaseGet function
* @endinternal
*
* @brief   Get CPU code base assigned to trapped and mirrored PTP packets.
*         A PTP packet's CPU code is computed as base CPU code + messageType
*         (where message type is taken from PTP header).
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodePtr              - (pointer to) The base of CPU code assigned to PTP
*                                      packets mirrored or trapped to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpCpuCodeBaseGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT     *cpuCodePtr
)
{

    GT_U32                                  regAddr;    /* register address */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode; /* DSA code */
    GT_U32                                  tempCode;   /* dsa code */
    GT_STATUS                               rc;         /* return code */
    GT_U32                                  offset;     /* bit offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(cpuCodePtr);

    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* BobCat2 and above devices */
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPExceptionsAndCPUCodeConfig;
        offset  = 14;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ttiRegs.ptpPacketCmdCfg1Reg;
        offset  = 18;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 8, &tempCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    dsaCpuCode = (PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)tempCode;

    return prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,cpuCodePtr);
}

/**
* @internal cpssDxChPtpCpuCodeBaseGet function
* @endinternal
*
* @brief   Get CPU code base assigned to trapped and mirrored PTP packets.
*         A PTP packet's CPU code is computed as base CPU code + messageType
*         (where message type is taken from PTP header).
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodePtr              - (pointer to) The base of CPU code assigned to PTP
*                                       packets mirrored or trapped to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpCpuCodeBaseGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT     *cpuCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpCpuCodeBaseGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCodePtr));

    rc = internal_cpssDxChPtpCpuCodeBaseGet(devNum, cpuCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTodCounterFunctionSet function
* @endinternal
*
* @brief   The function configures type of TOD counter action that will be
*         performed once triggered by cpssDxChPtpTodCounterFunctionTriggerSet()
*         API or by pulse from an external interface.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Tx, Rx or both directions
*                                      (APPLICABLE DEVICES   Lion2.)
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] function                 - One of the four possible TOD counter functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTodCounterFunctionSet
(
    IN GT_U8                                    devNum,
    IN CPSS_PORT_DIRECTION_ENT                  direction,
    IN CPSS_DXCH_PTP_TAI_ID_STC                 *taiIdPtr,
    IN CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT       function
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    GT_UNUSED_PARAM(direction);
    /* BobCat2 and above devices */

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    switch (function)
    {
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E:
            hwValue = 0;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E:
            hwValue = 1;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E:
            hwValue = 2;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_DECREMENT_E:
            hwValue = 3;
            if ((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_CAELUM_TOD_DECREMENT_OPERATION_E)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_CAPTURE_E:
            hwValue = 4;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E:
            hwValue = 5;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E:
            hwValue = 6;
            break;
        case CPSS_DXCH_PTP_TOD_COUNTER_FUNC_NOP_E:
            hwValue = 7;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;
        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, 2, 3, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;

}

/**
* @internal cpssDxChPtpTodCounterFunctionSet function
* @endinternal
*
* @brief   The function configures type of TOD counter action that will be
*         performed once triggered by cpssDxChPtpTodCounterFunctionTriggerSet()
*         API or by pulse from an external interface.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Tx, Rx or both directions
*                                      (APPLICABLE DEVICES Lion2.)
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] function                 - One of the four possible TOD counter functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTodCounterFunctionSet
(
    IN GT_U8                                    devNum,
    IN CPSS_PORT_DIRECTION_ENT                  direction,
    IN CPSS_DXCH_PTP_TAI_ID_STC                 *taiIdPtr,
    IN CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT       function
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTodCounterFunctionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction, taiIdPtr, function));

    rc = internal_cpssDxChPtpTodCounterFunctionSet(devNum, direction, taiIdPtr, function);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction, taiIdPtr, function));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTodCounterFunctionGet function
* @endinternal
*
* @brief   Get type of TOD counter action that will be performed once triggered by
*         cpssDxChPtpTodCounterFunctionTriggerSet() API or by pulse from an
*         external interface.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Tx, Rx or both directions
*                                      (APPLICABLE DEVICES   Lion2.)
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] functionPtr              - (pointer to) One of the four possible TOD counter
*                                      functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTodCounterFunctionGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_DIRECTION_ENT                 direction,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT      *functionPtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    GT_UNUSED_PARAM(direction);

    /* BobCat2 and above devices */
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(functionPtr);
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 2, 3, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (hwValue)
    {
        case 0:
            *functionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E;
            break;
        case 1:
            *functionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E;
            break;
        case 2:
            *functionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E;
            break;
        case 3:
            *functionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_DECREMENT_E;
            break;
        case 4:
            *functionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_CAPTURE_E;
            break;
        case 5:
            *functionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E;
            break;
        case 6:
            *functionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E;
            break;
        case 7:
            *functionPtr = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_NOP_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /*never occurs*/
    }
    return GT_OK;
}

/**
* @internal cpssDxChPtpTodCounterFunctionGet function
* @endinternal
*
* @brief   Get type of TOD counter action that will be performed once triggered by
*         cpssDxChPtpTodCounterFunctionTriggerSet() API or by pulse from an
*         external interface.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Tx, Rx or both directions
*                                      (APPLICABLE DEVICES   Lion2.)
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] functionPtr              - (pointer to) One of the four possible TOD counter
*                                      functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTodCounterFunctionGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_DIRECTION_ENT                 direction,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT      *functionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTodCounterFunctionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction, taiIdPtr, functionPtr));

    rc = internal_cpssDxChPtpTodCounterFunctionGet(devNum, direction, taiIdPtr, functionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction, taiIdPtr, functionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTodCounterFunctionTriggerSet function
* @endinternal
*
* @brief   Trigger TOD (Time of Day) counter function according
*         to the function set by
*         cpssDxChPtpTodCounterFunctionSet().
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Tx, Rx or both directions
*                                      (APPLICABLE DEVICES   Lion2.)
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTodCounterFunctionTriggerSet
(
    IN GT_U8                       devNum,
    IN CPSS_PORT_DIRECTION_ENT     direction,
    IN  CPSS_DXCH_PTP_TAI_ID_STC   *taiIdPtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_BOOL                            status;
    GT_U32                             offset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    GT_UNUSED_PARAM(direction);
    /* BobCat2 and above devices */

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);


    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl50;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;
        }

        rc = cpssDxChPtpTodCounterFunctionTriggerStatusGet(devNum, taiIdPtr, &status);
        if (rc != GT_OK)
        {
            return rc;
        }
        #ifndef ASIC_SIMULATION
        else
        if (status == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        #endif

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            offset = taiIterator.taiNumber*2;
            /* Global pulse; set taiNumber bits */
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, offset, 2, 0x3);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Global pulse software trigger to all TAIs; set trigger bit */
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 10, 1, 1);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* Global pulse software trigger to all TAIs; unset trigger bit */
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 10, 1, 0);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Global pulse; unset taiNumber bits */
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, offset, 2, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            /* set trigger bit */
            rc = prvCpssDrvHwPpPortGroupSetRegField(
                devNum, taiIterator.portGroupId, regAddr, 0, 1, 1);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;

}

/**
* @internal cpssDxChPtpTodCounterFunctionTriggerSet function
* @endinternal
*
* @brief   Trigger TOD (Time of Day) counter function according
*         to the function set by
*         cpssDxChPtpTodCounterFunctionSet().
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Tx, Rx or both directions
*                                      (APPLICABLE DEVICES   Lion2.)
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTodCounterFunctionTriggerSet
(
    IN GT_U8                       devNum,
    IN CPSS_PORT_DIRECTION_ENT     direction,
    IN  CPSS_DXCH_PTP_TAI_ID_STC   *taiIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTodCounterFunctionTriggerSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction, taiIdPtr));

    rc = internal_cpssDxChPtpTodCounterFunctionTriggerSet(devNum, direction, taiIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction, taiIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTodCounterFunctionTriggerStatusGet function
* @endinternal
*
* @brief   Get status of trigger TOD (Time of Day) counter function.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] finishPtr                - (pointer to) GT_TRUE - The trigger is on, the TOD
*                                      function is not finished.
*                                      GT_FALSE - The trigger is off, the TOD function is
*                                      finished.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS internal_cpssDxChPtpTodCounterFunctionTriggerStatusGet
(
    IN GT_U8                       devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC   *taiIdPtr,
    OUT GT_BOOL                    *finishPtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
        CPSS_NULL_PTR_CHECK_MAC(finishPtr);

        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl50;
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr, 0, 10, &hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (taiIdPtr->taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E)
        {
            *finishPtr = (hwValue == 0x3FF) ? GT_TRUE : GT_FALSE;
        }
        else
        {
            hwValue = hwValue >> (2*taiIdPtr->taiNumber);
            *finishPtr = ((hwValue & 0x3) == 0x3) ? GT_TRUE : GT_FALSE;
        }

    }
    else if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        /* BobCat2 and above devices */
        PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
        CPSS_NULL_PTR_CHECK_MAC(finishPtr);
        PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;

        rc = prvCpssDrvHwPpPortGroupGetRegField(
            devNum, taiIterator.portGroupId, regAddr, 0, 1, &hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        *finishPtr = BIT2BOOL_MAC(hwValue);

   }
    return GT_OK;
}


/**
* @internal cpssDxChPtpTodCounterFunctionTriggerStatusGet function
* @endinternal
*
* @brief   Get status of trigger TOD (Time of Day) counter function.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] finishPtr                - (pointer to) GT_TRUE - The trigger is on, the TOD
*                                      function is not finished.
*                                      GT_FALSE - The trigger is off, the TOD function is
*                                      finished.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTodCounterFunctionTriggerStatusGet
(
    IN GT_U8                       devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC   *taiIdPtr,
    OUT GT_BOOL                    *finishPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTodCounterFunctionTriggerStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, finishPtr));

    rc = internal_cpssDxChPtpTodCounterFunctionTriggerStatusGet(devNum, taiIdPtr, finishPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, finishPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiClockModeSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] clockMode                - clock interface using mode.
*                                      Output mode relevant to Global TAI Instance only.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiClockModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT        clockMode
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */
    GT_U32                             mppHwValue;     /* HW data  */
    GT_U32                             clkOvrdOffset;/* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    switch (clockMode)
    {
        case CPSS_DXCH_PTP_TAI_CLOCK_MODE_DISABLED_E:
            hwValue = 0;
            break;
        case CPSS_DXCH_PTP_TAI_CLOCK_MODE_OUTPUT_E:
            hwValue = 1;
            break;
        case CPSS_DXCH_PTP_TAI_CLOCK_MODE_INPUT_E:
            hwValue = 2;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;
        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, 9, 2, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(devNum))
        {
            /* Muxing between MPP and PTP
             * clkOvrdOffset[13] Enable - Indicates GPP[11] & GPP[3]
             *                            are used as PTP_CLK_IN[0] & PTP_CLK_OUT[0] respectively
             * clkOvrdOffset[12] Enable - Indicates GPP[7] & GPP[0] are
             *                            used as PTP_CLK_IN[1] & PTP_CLK_OUT[1] respectively */
            clkOvrdOffset = (taiIterator.taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_0_E)?13:12;
            mppHwValue = (clockMode == CPSS_DXCH_PTP_TAI_CLOCK_MODE_DISABLED_E)?0:1;
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnitsDeviceSpecificRegs.deviceCtrl33;
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum,
                    regAddr, clkOvrdOffset, 1, mppHwValue);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiClockModeSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] clockMode                - clock interface using mode.
*                                      Output mode relevant to Global TAI Instance only.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiClockModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT        clockMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiClockModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, clockMode));

    rc = internal_cpssDxChPtpTaiClockModeSet(devNum, taiIdPtr, clockMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, clockMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiClockModeGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] clockModePtr             - (pointer to) clock interface using mode.
*                                      Output mode relevant to Global TAI Instance only.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiClockModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT        *clockModePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;      /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(clockModePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 9, 2, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (hwValue)
    {
        case 0:
            *clockModePtr = CPSS_DXCH_PTP_TAI_CLOCK_MODE_DISABLED_E;
            break;
        case 1:
            *clockModePtr = CPSS_DXCH_PTP_TAI_CLOCK_MODE_OUTPUT_E;
            break;
        case 2:
            *clockModePtr = CPSS_DXCH_PTP_TAI_CLOCK_MODE_INPUT_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /*never occurs*/
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiClockModeGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] clockModePtr            - (pointer to) clock interface using mode.
*                                      Output mode relevant to Global TAI Instance only.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssDxChPtpTaiClockModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT        *clockModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiClockModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, clockModePtr));

    rc = internal_cpssDxChPtpTaiClockModeGet(devNum, taiIdPtr, clockModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, clockModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiInternalClockGenerateEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Internal Clock Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   - Internal Clock Generate Enable.
*                                      GT_TRUE - the internal clock generator
*                                      generates a clock signal
*                                      GT_FALSE - the internal clock not generated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiInternalClockGenerateEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    hwValue = BOOL2BIT_MAC(enable);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;
        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, 8, 1, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiInternalClockGenerateEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Internal Clock Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   - Internal Clock Generate Enable.
*                                      GT_TRUE - the internal clock generator
*                                      generates a clock signal
*                                      GT_FALSE - the internal clock not generated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiInternalClockGenerateEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiInternalClockGenerateEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enable));

    rc = internal_cpssDxChPtpTaiInternalClockGenerateEnableSet(devNum, taiIdPtr, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiInternalClockGenerateEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Internal Clock Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) Internal Clock Generate Enable.
*                                      GT_TRUE - the internal clock generator
*                                      generates a clock
*                                      GT_FALSE - the internal clock not generated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiInternalClockGenerateEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 8, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(hwValue);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiInternalClockGenerateEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Internal Clock Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) Internal Clock Generate Enable.
*                                      GT_TRUE - the internal clock generator
*                                      generates a clock
*                                      GT_FALSE - the internal clock not generated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiInternalClockGenerateEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiInternalClockGenerateEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enablePtr));

    rc = internal_cpssDxChPtpTaiInternalClockGenerateEnableGet(devNum, taiIdPtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPClkDriftAdjustEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   - PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiPClkDriftAdjustEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    hwValue = BOOL2BIT_MAC(enable);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {


            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;

        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, 7, 1, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiPClkDriftAdjustEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   - PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiPClkDriftAdjustEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPClkDriftAdjustEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enable));

    rc = internal_cpssDxChPtpTaiPClkDriftAdjustEnableSet(devNum, taiIdPtr, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPClkDriftAdjustEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiPClkDriftAdjustEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;


    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 7, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(hwValue);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiPClkDriftAdjustEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiPClkDriftAdjustEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPClkDriftAdjustEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enablePtr));

    rc = internal_cpssDxChPtpTaiPClkDriftAdjustEnableGet(devNum, taiIdPtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiCaptureOverrideEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Capture Override Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   - Capture Override Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiCaptureOverrideEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    hwValue = BOOL2BIT_MAC(enable);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;
        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, 6, 1, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiCaptureOverrideEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Capture Override Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   - Capture Override Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiCaptureOverrideEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiCaptureOverrideEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enable));

    rc = internal_cpssDxChPtpTaiCaptureOverrideEnableSet(devNum, taiIdPtr, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiCaptureOverrideEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Capture Override Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) Capture Override Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiCaptureOverrideEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 6, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(hwValue);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiCaptureOverrideEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Capture Override Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) Capture Override Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiCaptureOverrideEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiCaptureOverrideEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enablePtr));

    rc = internal_cpssDxChPtpTaiCaptureOverrideEnableGet(devNum, taiIdPtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiInputTriggersCountEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Count Input Triggers Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   - Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see cpssDxChPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiInputTriggersCountEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    hwValue = BOOL2BIT_MAC(enable);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).TAICtrlReg0;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;
        }

        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, 5, 1, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiInputTriggersCountEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Count Input Triggers Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   - Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see cpssDxChPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiInputTriggersCountEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiInputTriggersCountEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enable));

    rc = internal_cpssDxChPtpTaiInputTriggersCountEnableSet(devNum, taiIdPtr, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiInputTriggersCountEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Count Input Triggers Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see cpssDxChPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiInputTriggersCountEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).TAICtrlReg0;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 5, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(hwValue);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiInputTriggersCountEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Count Input Triggers Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see cpssDxChPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiInputTriggersCountEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiInputTriggersCountEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enablePtr));

    rc = internal_cpssDxChPtpTaiInputTriggersCountEnableGet(devNum, taiIdPtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiExternalPulseWidthSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Pulse Width.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] extPulseWidth            - External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on bad extPulseWidth
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiExternalPulseWidthSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_U32                                  extPulseWidth
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */
    GT_U32                             ptpClkInKhz;
    GT_U32                             regData;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    if (PRV_CPSS_SIP_5_16_CHECK_MAC(devNum))
    {
        ptpClkInKhz = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.ptpClkInKhz;
    }
    else
    {
        /* Get TAIs reference clock (its half of PLL4 frequency) */
        if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) != GT_FALSE)
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceSAR2;
            rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,regAddr, 12, 2,&regData);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        switch(regData)
        {
            case 0: ptpClkInKhz = 500000; /* 500 MHz */
                    break;
            case 1: ptpClkInKhz = 546875; /* 546.875MHz*/
                    break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        /* Convert pulse width from nanoseconds to TAI clock cycles resolution */
        extPulseWidth = extPulseWidth*ptpClkInKhz/1000000;
    }

    if (extPulseWidth >= BIT_28)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).triggerGenerationControl;
            rc = prvCpssDrvHwPpPortGroupSetRegField(
                    devNum, taiIterator.portGroupId, regAddr, 0, 28, extPulseWidth);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                    devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig1;
            hwValue = (extPulseWidth & 0xFFFF);
            rc = prvCpssDrvHwPpPortGroupSetRegField(
                    devNum, taiIterator.portGroupId, regAddr, 0, 16, hwValue);
            if (rc != GT_OK)
            {
                return rc;
            }
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                    devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig2;
            hwValue = ((extPulseWidth >> 16) & 0x0FFF);
            rc = prvCpssDrvHwPpPortGroupSetRegField(
                    devNum, taiIterator.portGroupId, regAddr, 0, 12, hwValue);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiExternalPulseWidthSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Pulse Width.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] extPulseWidth            - External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on bad extPulseWidth
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiExternalPulseWidthSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_U32                                  extPulseWidth
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiExternalPulseWidthSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, extPulseWidth));

    rc = internal_cpssDxChPtpTaiExternalPulseWidthSet(devNum, taiIdPtr, extPulseWidth);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, extPulseWidth));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiExternalPulseWidthGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Pulse Width
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] extPulseWidthPtr         -  (pointer to) External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiExternalPulseWidthGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_U32                                  *extPulseWidthPtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue = 0;     /* HW data  */
    GT_U32                             ptpClkInKhz;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(extPulseWidthPtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).triggerGenerationControl;
        rc = prvCpssDrvHwPpPortGroupGetRegField(
                devNum, taiIterator.portGroupId, regAddr, 0, 28, extPulseWidthPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig1;
        rc = prvCpssDrvHwPpPortGroupGetRegField(
                devNum, taiIterator.portGroupId, regAddr, 0, 16, &hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        *extPulseWidthPtr = hwValue;

        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig2;
        rc = prvCpssDrvHwPpPortGroupGetRegField(
            devNum, taiIterator.portGroupId, regAddr, 0, 12, &hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        *extPulseWidthPtr |= (hwValue << 16);
    }
    if (PRV_CPSS_SIP_5_16_CHECK_MAC(devNum))
    {
        ptpClkInKhz = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.ptpClkInKhz;
    }
    else
    {
        /* Get TAIs reference clock (its half of PLL4 frequency) */
        if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) != GT_FALSE)
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceSAR2;
            rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,regAddr, 12, 2,&hwValue);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        switch(hwValue)
        {
            case 0: ptpClkInKhz = 500000; /* 500 MHz */
                    break;
            case 1: ptpClkInKhz = 546875; /* 546.875MHz*/
                    break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        /* Convert pulse width from TAI clock cycle resolution to nanoseconds */
        *extPulseWidthPtr = *extPulseWidthPtr*1000000/ptpClkInKhz;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiExternalPulseWidthGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Pulse Width
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] extPulseWidthPtr         -  (pointer to) External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiExternalPulseWidthGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_U32                                  *extPulseWidthPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiExternalPulseWidthGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, extPulseWidthPtr));

    rc = internal_cpssDxChPtpTaiExternalPulseWidthGet(devNum, taiIdPtr, extPulseWidthPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, extPulseWidthPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPtpTaiTodValueRegisterAddressGet function
* @endinternal
*
* @brief   Get TOD values register addresses.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] gopNumber                - GOP Number
* @param[in] taiNumber                - TAI Number
* @param[in] readUse                  - GT_TRUE - read access, GT_FALSE - wrire access
* @param[in] todValueType             - type of TOD value.
*                                      Valid types are for both read and write:
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E.
*                                      Valid types are for read only:
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E,
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E.
*
* @param[out] regAddrArr[7]            - (pointer to) array of register addresses
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*
* @note This API does not activate any triggers, other APIs does it
*
*/
static GT_STATUS prvCpssDxChPtpTaiTodValueRegisterAddressGet
(
    IN  GT_U8                                 devNum,
    IN  GT_U32                                gopNumber,
    IN  GT_U32                                taiNumber,
    IN  GT_BOOL                               readUse,
    IN  CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT        todValueType,
    OUT GT_U32                                regAddrArr[7]
)
{
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance != GT_FALSE)
    {
        if ((gopNumber != 0) || (taiNumber != 0))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    switch (todValueType)
    {
        case CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E:
            regAddrArr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).triggerGenerationTODSecHigh;
            regAddrArr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).triggerGenerationTODSecMed;
            regAddrArr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).triggerGenerationTODSecLow;
            regAddrArr[3] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).triggerGenerationTODNanoHigh;
            regAddrArr[4] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).triggerGenerationTODNanoLow;
            regAddrArr[5] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).triggerGenerationTODFracHigh;
            regAddrArr[6] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).triggerGenerationTODFracLow;
            break;
        case CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E:
            regAddrArr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).generateFunctionMaskSecHigh;
            regAddrArr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).generateFunctionMaskSecMed;
            regAddrArr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).generateFunctionMaskSecLow;
            regAddrArr[3] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).generateFunctionMaskNanoHigh;
            regAddrArr[4] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).generateFunctionMaskNanoLow;
            regAddrArr[5] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).generateFunctionMaskFracHigh;
            regAddrArr[6] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).generateFunctionMaskFracLow;
            break;
        case CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E:
            regAddrArr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeLoadValueSecHigh;
            regAddrArr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeLoadValueSecMed;
            regAddrArr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeLoadValueSecLow;
            regAddrArr[3] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeLoadValueNanoHigh;
            regAddrArr[4] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeLoadValueNanoLow;
            regAddrArr[5] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeLoadValueFracHigh;
            regAddrArr[6] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeLoadValueFracLow;
            break;
        case CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E:
            if (readUse == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            regAddrArr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue0SecHigh;
            regAddrArr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue0SecMed;
            regAddrArr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue0SecLow;
            regAddrArr[3] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue0NanoHigh;
            regAddrArr[4] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue0NanoLow;
            regAddrArr[5] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue0FracHigh;
            regAddrArr[6] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue0FracLow;
            break;
        case CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E:
            if (readUse == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            regAddrArr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue1SecHigh;
            regAddrArr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue1SecMed;
            regAddrArr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue1SecLow;
            regAddrArr[3] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue1NanoHigh;
            regAddrArr[4] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue1NanoLow;
            regAddrArr[5] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue1FracHigh;
            regAddrArr[6] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, gopNumber, taiNumber).timeCaptureValue1FracLow;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPtpTaiTodSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD values.
*         The TOD will be updated by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] todValueType             - type of TOD value.
*                                      Valid types are
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E.
* @param[in] todValuePtr              - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API does not activate any triggers, other APIs does it
*
*/
static GT_STATUS internal_cpssDxChPtpTaiTodSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT        todValueType,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC           *todValuePtr
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr[7];     /* register address */
    GT_U32                             hwValue[7];     /* HW data  */
    GT_U32                             i;              /* loop index       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    CPSS_NULL_PTR_CHECK_MAC(todValuePtr);

    hwValue[0] = todValuePtr->seconds.l[1] & 0xFFFF;
    hwValue[1] = (todValuePtr->seconds.l[0] >> 16) & 0xFFFF;
    hwValue[2] = todValuePtr->seconds.l[0] & 0xFFFF;
    hwValue[3] = (todValuePtr->nanoSeconds >> 16) & 0xFFFF;
    hwValue[4] = todValuePtr->nanoSeconds & 0xFFFF;
    hwValue[5] = (todValuePtr->fracNanoSeconds >> 16) & 0xFFFF;
    hwValue[6] = todValuePtr->fracNanoSeconds & 0xFFFF;

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        rc = prvCpssDxChPtpTaiTodValueRegisterAddressGet(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber,
            GT_FALSE /*readUse*/, todValueType, regAddr);
        if (rc != GT_OK)
        {
            return rc;
        }

        for (i = 0; (i < 7); i++)
        {
            rc = prvCpssDrvHwPpSetRegField(
                devNum, regAddr[i], 0, 16, hwValue[i]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiTodSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD values.
*         The TOD will be updated by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] todValueType             - type of TOD value.
*                                      Valid types are
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E.
* @param[in] todValuePtr              - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API does not activate any triggers, other APIs does it
*
*/
GT_STATUS cpssDxChPtpTaiTodSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT        todValueType,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC           *todValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiTodSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, todValueType, todValuePtr));

    rc = internal_cpssDxChPtpTaiTodSet(devNum, taiIdPtr, todValueType, todValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, todValueType, todValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiTodGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD values.
*         The TOD was captured by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] todValueType             - type of TOD value.
*
* @param[out] todValuePtr              - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiTodGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT        todValueType,
    OUT CPSS_DXCH_PTP_TOD_COUNT_STC           *todValuePtr
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr[7];     /* register address */
    GT_U32                             hwValue[7];     /* HW data  */
    GT_U32                             i;              /* loop index       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(todValuePtr);

    cpssOsMemSet(todValuePtr, 0, sizeof(*todValuePtr));

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    rc = prvCpssDxChPtpTaiTodValueRegisterAddressGet(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber,
        GT_TRUE /*readUse*/, todValueType, regAddr);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i = 0; (i < 7); i++)
    {
        rc = prvCpssDrvHwPpGetRegField(
            devNum, regAddr[i], 0, 16, &(hwValue[i]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    todValuePtr->seconds.l[1]    = hwValue[0];
    todValuePtr->seconds.l[0]    = ((hwValue[1] << 16) | hwValue[2]);
    todValuePtr->nanoSeconds     = ((hwValue[3] << 16) | hwValue[4]);
    todValuePtr->fracNanoSeconds = ((hwValue[5] << 16) | hwValue[6]);

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiTodGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD values.
*         The TOD was captured by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] todValueType             - type of TOD value.
*
* @param[out] todValuePtr              - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT        todValueType,
    OUT CPSS_DXCH_PTP_TOD_COUNT_STC           *todValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiTodGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, todValueType, todValuePtr));

    rc = internal_cpssDxChPtpTaiTodGet(devNum, taiIdPtr, todValueType, todValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, todValueType, todValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiOutputTriggerEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   -   output trigger generation.
*                                      GT_TRUE  - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note use cpssDxChPtpTaiTodSet with
*       CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E and
*       the time to trigger generation.
*
*/
static GT_STATUS internal_cpssDxChPtpTaiOutputTriggerEnableSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_BOOL                               enable
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */
    GT_U32                             startBit;    /* bit number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    hwValue = BOOL2BIT_MAC(enable);

    startBit = (PRV_CPSS_SIP_6_CHECK_MAC(devNum)) ? 31 : 1;

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).triggerGenerationControl;
        }
        else
        {

            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;
        }

        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, startBit, 1, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiOutputTriggerEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   -   output trigger generation.
*                                      GT_TRUE  - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note use cpssDxChPtpTaiTodSet with
*       CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E and
*       the time to trigger generation.
*
*/
GT_STATUS cpssDxChPtpTaiOutputTriggerEnableSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_BOOL                               enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiOutputTriggerEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enable));

    rc = internal_cpssDxChPtpTaiOutputTriggerEnableSet(devNum, taiIdPtr, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiOutputTriggerEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) enable output trigger generation.
*                                      GT_TRUE  - enable GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiOutputTriggerEnableGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_BOOL                               *enablePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */
    GT_U32                             startBit;    /* bit number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        startBit = 31;
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).triggerGenerationControl;
    }
    else
    {
        startBit = 1;
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, startBit, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(hwValue);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiOutputTriggerEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) enable output trigger generation.
*                                      GT_TRUE  - enable GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiOutputTriggerEnableGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_BOOL                               *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiOutputTriggerEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enablePtr));

    rc = internal_cpssDxChPtpTaiOutputTriggerEnableGet(devNum, taiIdPtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiTodStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiTodStepSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC          *taiIdPtr,
    IN  CPSS_DXCH_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr[4];     /* register address */
    GT_U32                             hwValue[4];     /* HW data  */
    GT_U32                             hwValueSlave[3];/* HW data for slave TAIs */
    GT_U32                             i;              /* loop index       */
    GT_U32                             ptpClkInKhz;    /* Ptp TAI clock */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_FALCON_ENT  falconTaiSubUnitType; /* type of TAI sub unit */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HAWK_ENT    hawkTaiSubUnitType; /* type of TAI sub unit */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_ENT  phoenixTaiSubUnitType; /* type of TAI sub unit */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_ENT  harrierTaiSubUnitType; /* type of TAI sub ubit */
    CPSS_DXCH_PTP_TAI_TOD_STEP_STC     todStepSlave;
    GT_U32                             totalSubUnitType;/* total number TAI types */
    GT_U32                             taiGop;          /* Absolute GOP tile number */
    GT_U32                             tileId,numOfTiles;/* TILE ID */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(todStepPtr);

    if(!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        hwValue[0] = (todStepPtr->nanoSeconds >> 16) & 0xFFFF;
    }
    else
    {
        /* only 16 bits are used for nanoseconds starting from Caelum (BobK) */
        if (todStepPtr->nanoSeconds >= BIT_16)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    hwValue[1] = todStepPtr->nanoSeconds & 0xFFFF;
    hwValue[2] = (todStepPtr->fracNanoSeconds >> 16) & 0xFFFF;
    hwValue[3] = todStepPtr->fracNanoSeconds & 0xFFFF;

    numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ?
                 PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles : 1;

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        /* Master TAI(Only for TILE0) TOD Step set */
        if(!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            /* TODStepNanoConfigHigh not exist for sip 15 and above */
            regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).TODStepNanoConfigHigh;
        }
        regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).TODStepNanoConfigLow;
        regAddr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).TODStepFracConfigHigh;
        regAddr[3] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).TODStepFracConfigLow;

        for (i = 0; (i < 4); i++)
        {
            if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) && (i == 0))
            {
                /* TODStepNanoConfigHigh not exist for sip 15 and above */
                continue;
            }
            rc = prvCpssDrvHwPpPortGroupSetRegField(
                devNum, taiIterator.portGroupId, regAddr[i], 0, 16, hwValue[i]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {

            /* Slave TAIs(ALL TILE) TOD Step Set
             * Valid only for SIP_6 and when debug instance is not enabled */
            totalSubUnitType =
                (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && (PRV_SHARED_PTP_DIR_PTP_SRC_GLOBAL_VAR_GET(debugInstanceEnabled) == GT_FALSE))?FALCON_PER_TILE_TAI_SUB_UNITS_NUMBER_CNS:0;

            for (falconTaiSubUnitType = 1; (GT_U32)falconTaiSubUnitType < totalSubUnitType; falconTaiSubUnitType++)
            {
                switch (falconTaiSubUnitType)
                {
                    case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_CP_TAI_SLAVE_PIPE0_E:
                    case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_CP_TAI_SLAVE_PIPE1_E:
                        ptpClkInKhz = FALCON_PTP_CP_CLK_CNS;
                        break;

                    case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TXQ_TAI_SLAVE_PIPE0_E:
                    case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TXQ_TAI_SLAVE_PIPE1_E:
                        ptpClkInKhz = FALCON_PTP_TXQ_CLK_CNS;
                        break;

                    case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_0_E:
                    case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_1_E:
                    case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_2_E:
                    case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_3_E:
                        ptpClkInKhz = FALCON_PTP_RAVEN_CLK_CNS;
                        break;
                    default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                todStepSlave.nanoSeconds = 1000000/ptpClkInKhz;
                todStepSlave.fracNanoSeconds = (GT_U32)(0xFFFFFFFF *
                        (1000000.0/ptpClkInKhz - todStepSlave.nanoSeconds) +
                        (1000000.0/ptpClkInKhz - todStepSlave.nanoSeconds));

                /* only 16 bits are used for nanoseconds starting from Caelum (BobK)
                 * firstRegIdx already SET TO 1*/
                hwValueSlave[0] = todStepSlave.nanoSeconds & 0xFFFF;
                hwValueSlave[1] = (todStepSlave.fracNanoSeconds >> 16) & 0xFFFF;
                hwValueSlave[2] = todStepSlave.fracNanoSeconds & 0xFFFF;

                for(tileId = 0; tileId < numOfTiles; tileId++)
                {
                    taiGop = (tileId*FALCON_PER_TILE_TAI_SUB_UNITS_NUMBER_CNS) + falconTaiSubUnitType;
                    regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                            devNum, taiGop, taiIterator.taiNumber).TODStepNanoConfigLow;
                    regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                            devNum, taiGop, taiIterator.taiNumber).TODStepFracConfigHigh;
                    regAddr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                            devNum, taiGop, taiIterator.taiNumber).TODStepFracConfigLow;

                    for (i = 0; (i < 3); i++)
                    {
                        PRV_CPSS_SKIP_NOT_EXIST_RAVEN_ADDRESS_MAC(devNum, regAddr[i]);

                        rc = prvCpssDrvHwPpPortGroupSetRegField(
                                devNum, taiIterator.portGroupId, regAddr[i], 0, 16, hwValueSlave[i]);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                }
            }
        }
        else if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            /* do nothing; no slaves only masters. */
        }
        else if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
        {
            totalSubUnitType = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.numOfTaiUnits;

            for (harrierTaiSubUnitType = 1; (GT_U32)harrierTaiSubUnitType < totalSubUnitType; harrierTaiSubUnitType++)
            {
                switch (harrierTaiSubUnitType)
                {
                case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_TTI_TAI0_E:
                case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_PHA_TAI0_E:
                case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_EPCL_HA_TAI0_E:
                    ptpClkInKhz = HARRIER_PTP_CP_CLK_CNS;
                    break;

                case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_TXQS_TAI0_E:
                case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_TXQS1_TAI0_E:
                    ptpClkInKhz = HARRIER_PTP_PB_CLK_CNS;
                    break;

                case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_PCA0_TAI0_E:
                case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_PCA1_TAI0_E:
                case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_PCA2_TAI0_E:
                case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_400G0_TAI0_E:
                case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_400G1_TAI0_E:
                case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_200G2_TAI0_E:
                    ptpClkInKhz = HARRIER_PTP_APP_CLK_CNS;
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                todStepSlave.nanoSeconds = 1000000000/ptpClkInKhz;
                if (1000000000.0/ptpClkInKhz - todStepSlave.nanoSeconds > 0.5)
                {
                    todStepSlave.nanoSeconds++;
                    todStepSlave.fracNanoSeconds = (GT_32) ((1000000000.0/ptpClkInKhz - todStepSlave.nanoSeconds) * 0x100000000);
                }
                else
                {
                    todStepSlave.fracNanoSeconds = (GT_U32)(0xFFFFFFFF *
                            (1000000000.0/ptpClkInKhz - todStepSlave.nanoSeconds) +
                            (1000000000.0/ptpClkInKhz - todStepSlave.nanoSeconds));
                }

                /* only 16 bits are used for nanoseconds starting from Caelum (BobK)
                 * firstRegIdx already SET TO 1*/
                hwValueSlave[0] = todStepSlave.nanoSeconds & 0xFFFF;
                hwValueSlave[1] = (todStepSlave.fracNanoSeconds >> 16) & 0xFFFF;
                hwValueSlave[2] = todStepSlave.fracNanoSeconds & 0xFFFF;

                regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, harrierTaiSubUnitType, taiIterator.taiNumber).TODStepNanoConfigLow;
                regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, harrierTaiSubUnitType, taiIterator.taiNumber).TODStepFracConfigHigh;
                regAddr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, harrierTaiSubUnitType, taiIterator.taiNumber).TODStepFracConfigLow;

                for (i = 0; (i < 3); i++)
                {
                    rc = prvCpssDrvHwPpPortGroupSetRegField(
                            devNum, taiIterator.portGroupId, regAddr[i], 0, 16, hwValueSlave[i]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
        else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
        {
            totalSubUnitType = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.numOfTaiUnits;

            for (phoenixTaiSubUnitType = 1; (GT_U32)phoenixTaiSubUnitType < totalSubUnitType; phoenixTaiSubUnitType++)
            {

                switch (phoenixTaiSubUnitType)
                {
                    case  PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_EPCL_HA_TAI0_E :
                    case  PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_TTI_TAI0_E:
                    case  PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_PHA_TAI0_E:
                            ptpClkInKhz = PHOENIX_PTP_CP_CLK_CNS;
                            break;

                    case  PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_TXQS_TAI0_E:
                            ptpClkInKhz = PHOENIX_PTP_TXQ_CLK_CNS;
                            break;

                   case   PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_PCA0_TAI0_E :
                            ptpClkInKhz = PHOENIX_PTP_APP_CLK_CNS;
                            break;


                    case   PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_100G_TAI0_E:
                            ptpClkInKhz = PHOENIX_PTP_MAC0_CLK_CNS;
                            break;


                  case    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_MAC_CPUM_TAI0_E:
                  case    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_USX2_0_TAI0_E:
                  case    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_USX2_1_TAI0_E:
                            ptpClkInKhz = PHOENIX_PTP_MAC1_CLK_CNS;
                        break;

                 case     PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_MAC_CPUC_TAI0_E:
                            ptpClkInKhz = PHOENIX_PTP_MAC2_CLK_CNS;
                        break;

                    default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                todStepSlave.nanoSeconds = 1000000000/ptpClkInKhz;
                if (1000000000.0/ptpClkInKhz - todStepSlave.nanoSeconds > 0.5)
                {
                    todStepSlave.nanoSeconds++;
                    todStepSlave.fracNanoSeconds = (GT_32) ((1000000000.0/ptpClkInKhz - todStepSlave.nanoSeconds) * 0x100000000);
                }
                else
                {
                    todStepSlave.fracNanoSeconds = (GT_U32)(0xFFFFFFFF *
                            (1000000000.0/ptpClkInKhz - todStepSlave.nanoSeconds) +
                            (1000000000.0/ptpClkInKhz - todStepSlave.nanoSeconds));
                }


                /* only 16 bits are used for nanoseconds starting from Caelum (BobK)
                 * firstRegIdx already SET TO 1*/
                hwValueSlave[0] = todStepSlave.nanoSeconds & 0xFFFF;
                hwValueSlave[1] = (todStepSlave.fracNanoSeconds >> 16) & 0xFFFF;
                hwValueSlave[2] = todStepSlave.fracNanoSeconds & 0xFFFF;

                 regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, phoenixTaiSubUnitType, taiIterator.taiNumber).TODStepNanoConfigLow;
                 regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, phoenixTaiSubUnitType, taiIterator.taiNumber).TODStepFracConfigHigh;
                 regAddr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, phoenixTaiSubUnitType, taiIterator.taiNumber).TODStepFracConfigLow;
                    for (i = 0; (i < 3); i++)
                    {
                        rc = prvCpssDrvHwPpPortGroupSetRegField(
                                devNum, taiIterator.portGroupId, regAddr[i], 0, 16, hwValueSlave[i]);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                }
            }
        else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            totalSubUnitType = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.numOfTaiUnits;

            for (hawkTaiSubUnitType = 1; (GT_U32)hawkTaiSubUnitType < totalSubUnitType; hawkTaiSubUnitType++)
            {

                switch (hawkTaiSubUnitType)
                {

                   case   PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TTI_TAI0_E:
                   case   PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHA_TAI0_E:
                   case   PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_EPCL_HA_TAI0_E:
                            ptpClkInKhz = HAWK_PTP_CP_CLK_CNS;
                            break;

                   case   PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TXQS_TAI0_E:
                   case   PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TXQS1_TAI0_E:
                            ptpClkInKhz = HAWK_PTP_PB_CLK_CNS;
                            break;


                   case   PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_CPU_PORT_TAI0_E:
                   case   PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G0_TAI0_E:
                   case   PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G1_TAI0_E:
                   case   PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G2_TAI0_E:
                   case   PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G3_TAI0_E:
                            ptpClkInKhz = HAWK_PTP_GOP_MAC0_CLK_CNS;
                            break;


                  case    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_USX0_TAI0_E:
                  case    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_USX1_TAI0_E:
                        ptpClkInKhz = HAWK_PTP_GOP_MAC1_CLK_CNS;
                        break;

                 case     PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PCA2_TAI0_E:
                 case     PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PCA3_TAI0_E:
                 case     PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PCA0_TAI0_E:
                 case     PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PCA1_TAI0_E:
                        ptpClkInKhz = HAWK_PTP_GOP_APP_CLK_CNS;
                        break;

                    default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                todStepSlave.nanoSeconds = 1000000000/ptpClkInKhz;
                if (1000000000.0/ptpClkInKhz - todStepSlave.nanoSeconds > 0.5)
                {
                    todStepSlave.nanoSeconds++;
                    todStepSlave.fracNanoSeconds = (GT_32) ((1000000000.0/ptpClkInKhz - todStepSlave.nanoSeconds) * 0x100000000);
                }
                else
                {
                    todStepSlave.fracNanoSeconds = (GT_U32)(0xFFFFFFFF *
                            (1000000000.0/ptpClkInKhz - todStepSlave.nanoSeconds) +
                            (1000000000.0/ptpClkInKhz - todStepSlave.nanoSeconds));
                }

                /* only 16 bits are used for nanoseconds starting from Caelum (BobK)
                 * firstRegIdx already SET TO 1*/
                hwValueSlave[0] = todStepSlave.nanoSeconds & 0xFFFF;
                hwValueSlave[1] = (todStepSlave.fracNanoSeconds >> 16) & 0xFFFF;
                hwValueSlave[2] = todStepSlave.fracNanoSeconds & 0xFFFF;

                 regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, hawkTaiSubUnitType, taiIterator.taiNumber).TODStepNanoConfigLow;
                 regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, hawkTaiSubUnitType, taiIterator.taiNumber).TODStepFracConfigHigh;
                 regAddr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, hawkTaiSubUnitType, taiIterator.taiNumber).TODStepFracConfigLow;

                    for (i = 0; (i < 3); i++)
                    {
                        rc = prvCpssDrvHwPpPortGroupSetRegField(
                                devNum, taiIterator.portGroupId, regAddr[i], 0, 16, hwValueSlave[i]);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                }
            }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiTodStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodStepSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC          *taiIdPtr,
    IN  CPSS_DXCH_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiTodStepSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, todStepPtr));

    rc = internal_cpssDxChPtpTaiTodStepSet(devNum, taiIdPtr, todStepPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, todStepPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiTodStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiTodStepGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC          *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr[4];     /* register address */
    GT_U32                             hwValue[4];     /* HW data  */
    GT_U32                             i;              /* loop index       */
    GT_U32                             firstRegIdx;    /* index of first register for configuration */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(todStepPtr);

    cpssOsMemSet(todStepPtr, 0, sizeof(*todStepPtr));

    if(!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        firstRegIdx = 0;
    }
    else
    {
        /* only 16 bits are used for nanoseconds starting from Caelum (BobK) */
        firstRegIdx = 1;
        hwValue[0]  = 0;
    }

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).TODStepNanoConfigHigh;
    regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).TODStepNanoConfigLow;
    regAddr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).TODStepFracConfigHigh;
    regAddr[3] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).TODStepFracConfigLow;

    for (i = firstRegIdx; (i < 4); i++)
    {
        rc = prvCpssDrvHwPpPortGroupGetRegField(
            devNum, taiIterator.portGroupId, regAddr[i], 0, 16, &(hwValue[i]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    todStepPtr->nanoSeconds     = ((hwValue[0] << 16) | hwValue[1]);
    todStepPtr->fracNanoSeconds = ((hwValue[2] << 16) | hwValue[3]);

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiTodStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodStepGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC          *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiTodStepGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, todStepPtr));

    rc = internal_cpssDxChPtpTaiTodStepGet(devNum, taiIdPtr, todStepPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, todStepPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpTaiFractionalNanosecondDriftSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[in] fracNanoSecond           - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiFractionalNanosecondDriftSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_32                                 fracNanoSecond
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr[2];     /* register address */
    GT_U32                             hwValue[2];     /* HW data  */
    GT_U32                             i;              /* loop index       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    hwValue[0] = (fracNanoSecond >> 16) & 0xFFFF;
    hwValue[1] = fracNanoSecond & 0xFFFF;

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).driftAdjustmentConfigHigh;
        regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).driftAdjustmentConfigLow;

        for (i = 0; (i < 2); i++)
        {
            rc = prvCpssDrvHwPpPortGroupSetRegField(
                devNum, taiIterator.portGroupId, regAddr[i], 0, 16, hwValue[i]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiFractionalNanosecondDriftSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[in] fracNanoSecond           - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiFractionalNanosecondDriftSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_32                                 fracNanoSecond
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiFractionalNanosecondDriftSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, fracNanoSecond));

    rc = internal_cpssDxChPtpTaiFractionalNanosecondDriftSet(devNum, taiIdPtr, fracNanoSecond);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, fracNanoSecond));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiFractionalNanosecondDriftGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] fracNanoSecondPtr        - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiFractionalNanosecondDriftGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_32                                 *fracNanoSecondPtr
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr[2];     /* register address */
    GT_U32                             hwValue[2];     /* HW data  */
    GT_U32                             i;              /* loop index       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(fracNanoSecondPtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).driftAdjustmentConfigHigh;
    regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).driftAdjustmentConfigLow;

    for (i = 0; (i < 2); i++)
    {
        rc = prvCpssDrvHwPpPortGroupGetRegField(
            devNum, taiIterator.portGroupId, regAddr[i], 0, 16, &(hwValue[i]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    *fracNanoSecondPtr = ((hwValue[0] << 16) | hwValue[1]);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiFractionalNanosecondDriftGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] fracNanoSecondPtr        - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiFractionalNanosecondDriftGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_32                                 *fracNanoSecondPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiFractionalNanosecondDriftGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, fracNanoSecondPtr));

    rc = internal_cpssDxChPtpTaiFractionalNanosecondDriftGet(devNum, taiIdPtr, fracNanoSecondPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, fracNanoSecondPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPClkCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] nanoSeconds              - nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiPClkCycleSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_U32                                nanoSeconds
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr[2];     /* register address */
    GT_U32                             hwValue[2];     /* HW data  */
    GT_U32                             i;              /* loop index       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    if (nanoSeconds > BIT_30)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).PClkClockCycleConfigHigh;
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            rc = prvCpssDrvHwPpPortGroupSetRegField(
                devNum, taiIterator.portGroupId, regAddr[0], 0, 30, nanoSeconds);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            hwValue[0] = (nanoSeconds >> 16) & 0xFFFF;
            hwValue[1] = nanoSeconds & 0xFFFF;

            regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).PClkClockCycleConfigLow;

            for (i = 0; (i < 2); i++)
            {
                rc = prvCpssDrvHwPpPortGroupSetRegField(
                    devNum, taiIterator.portGroupId, regAddr[i], 0, 16, hwValue[i]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiPClkCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] nanoSeconds              - nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiPClkCycleSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_U32                                nanoSeconds
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPClkCycleSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, nanoSeconds));

    rc = internal_cpssDxChPtpTaiPClkCycleSet(devNum, taiIdPtr, nanoSeconds);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, nanoSeconds));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPClkCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiPClkCycleGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_U32                                *nanoSecondsPtr
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr[2];     /* register address */
    GT_U32                             hwValue[2];     /* HW data  */
    GT_U32                             i;              /* loop index       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(nanoSecondsPtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).PClkClockCycleConfigHigh;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = prvCpssDrvHwPpPortGroupGetRegField(
            devNum, taiIterator.portGroupId, regAddr[0], 0, 30, &(hwValue[0]));
        if (rc != GT_OK)
        {
            return rc;
        }
        *nanoSecondsPtr = hwValue[0];
    }
    else
    {
        regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).PClkClockCycleConfigLow;

        for (i = 0; (i < 2); i++)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(
                devNum, taiIterator.portGroupId, regAddr[i], 0, 16, &(hwValue[i]));
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        *nanoSecondsPtr = ((hwValue[0] << 16) | hwValue[1]);
    }
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiPClkCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiPClkCycleGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_U32                                *nanoSecondsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPClkCycleGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, nanoSecondsPtr));

    rc = internal_cpssDxChPtpTaiPClkCycleGet(devNum, taiIdPtr, nanoSecondsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, nanoSecondsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] seconds                  -  part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] nanoSeconds              - nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiClockCycleSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr[2];     /* register address */
    GT_U32                             hwValue[2];     /* HW data  */
    GT_U32                             i;              /* loop index       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    if (seconds >= BIT_2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (nanoSeconds >= BIT_30)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {

        hwValue[0] = ((nanoSeconds >> 16) & 0x3FFF) | ((seconds & 3) << 14);
        hwValue[1] = nanoSeconds & 0xFFFF;

        PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
        {
            regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockCycleConfigHigh;
            regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockCycleConfigLow;

            for (i = 0; (i < 2); i++)
            {
                rc = prvCpssDrvHwPpPortGroupSetRegField(
                    devNum, taiIterator.portGroupId, regAddr[i], 0, 16, hwValue[i]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)
    }
    /* for falcon*/
    else
    {
        hwValue[0] = seconds;
        hwValue[1] = nanoSeconds;
        PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
        {
            regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockCycleSeconds;
            regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockCycleNanoSeconds;
            rc = prvCpssDrvHwPpPortGroupSetRegField(
                devNum, taiIterator.portGroupId, regAddr[0], 0, 2,hwValue[0]);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDrvHwPpPortGroupSetRegField(
                devNum, taiIterator.portGroupId, regAddr[1], 0, 30, hwValue[1]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)
    }
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] seconds                  -  part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] nanoSeconds              - nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiClockCycleSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiClockCycleSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, seconds, nanoSeconds));

    rc = internal_cpssDxChPtpTaiClockCycleSet(devNum, taiIdPtr, seconds, nanoSeconds);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, seconds, nanoSeconds));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] secondsPtr               - (pointer to) seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiClockCycleGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_U32                                *secondsPtr,
    OUT GT_U32                                *nanoSecondsPtr
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr[2];     /* register address */
    GT_U32                             hwValue[2];     /* HW data  */
    GT_U32                             i;              /* loop index       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(secondsPtr);
    CPSS_NULL_PTR_CHECK_MAC(nanoSecondsPtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);
    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockCycleConfigHigh;
        regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockCycleConfigLow;
        for (i = 0; (i < 2); i++)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(
                devNum, taiIterator.portGroupId, regAddr[i], 0, 16, &(hwValue[i]));
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        *secondsPtr     = ((hwValue[0] >> 14) & 3);
        *nanoSecondsPtr = (((hwValue[0] & 0x3FFF) << 16) | hwValue[1]);
        return GT_OK;
    }
    /* for falcon  and above*/
    else
    {
        regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockCycleSeconds;
        regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockCycleNanoSeconds;
        rc = prvCpssDrvHwPpPortGroupGetRegField(
                devNum, taiIterator.portGroupId, regAddr[0], 0, 2, &(hwValue[0]));
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpPortGroupGetRegField(
                devNum, taiIterator.portGroupId, regAddr[1], 0, 30, &(hwValue[1]));
        if (rc != GT_OK)
        {
            return rc;
        }
        *secondsPtr = hwValue[0];
        *nanoSecondsPtr = hwValue[1];
        return GT_OK;
    }
}

/**
* @internal cpssDxChPtpTaiClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] secondsPtr               - (pointer to) seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiClockCycleGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_U32                                *secondsPtr,
    OUT GT_U32                                *nanoSecondsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiClockCycleGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, secondsPtr, nanoSecondsPtr));

    rc = internal_cpssDxChPtpTaiClockCycleGet(devNum, taiIdPtr, secondsPtr, nanoSecondsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, secondsPtr, nanoSecondsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpTaiTodCaptureStatusGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Capture Status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] captureIndex             - Capture Index.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] validPtr                 - (pointer to)Capture is valid.
*                                      GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiTodCaptureStatusGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_U32                                captureIndex,
    OUT GT_BOOL                               *validPtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);

    if (captureIndex > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCaptureStatus;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, captureIndex, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *validPtr = BIT2BOOL_MAC(hwValue);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiTodCaptureStatusGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Capture Status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] captureIndex             - Capture Index.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] validPtr                 - (pointer to)Capture is valid.
*                                      GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodCaptureStatusGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_U32                                captureIndex,
    OUT GT_BOOL                               *validPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiTodCaptureStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, captureIndex, validPtr));

    rc = internal_cpssDxChPtpTaiTodCaptureStatusGet(devNum, taiIdPtr, captureIndex, validPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, captureIndex, validPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiTodUpdateCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Update Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] valuePtr                 - (pointer to)TOD Update Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counter is Clear On Read. Value incremented at each
*       TOD Update/Increment/Decrement function.
*
*/
static GT_STATUS internal_cpssDxChPtpTaiTodUpdateCounterGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_U32                                *valuePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */
    GT_U32                             fieldLength; /* num of value bits */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeUpdateCntr;

    fieldLength = PRV_CPSS_SIP_6_CHECK_MAC(devNum)? 32:16;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 0, fieldLength, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *valuePtr = hwValue;
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiTodUpdateCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Update Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] valuePtr                 - (pointer to)TOD Update Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counter is Clear On Read. Value incremented at each
*       TOD Update/Increment/Decrement function.
*
*/
GT_STATUS cpssDxChPtpTaiTodUpdateCounterGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_U32                                *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiTodUpdateCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, valuePtr));

    rc = internal_cpssDxChPtpTaiTodUpdateCounterGet(devNum, taiIdPtr, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiIncomingTriggerCounterSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Incoming Trigger Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] value                    -  to set.
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiIncomingTriggerCounterSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_U32                                value
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    if (value >= BIT_8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    hwValue = value;

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).captureTriggerCntr;

        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, 0, 8, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiIncomingTriggerCounterSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Incoming Trigger Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] value                    -  to set.
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiIncomingTriggerCounterSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_U32                                value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiIncomingTriggerCounterSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, value));

    rc = internal_cpssDxChPtpTaiIncomingTriggerCounterSet(devNum, taiIdPtr, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiIncomingTriggerCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Trigger Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiIncomingTriggerCounterGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_U32                                *valuePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).captureTriggerCntr;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 0, 8, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *valuePtr = hwValue;
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiIncomingTriggerCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Trigger Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiIncomingTriggerCounterGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_U32                                *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiIncomingTriggerCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, valuePtr));

    rc = internal_cpssDxChPtpTaiIncomingTriggerCounterGet(devNum, taiIdPtr, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiIncomingClockCounterSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Incoming Clock Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   -  counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
* @param[in] value                    -  to set.
*                                      (APPLICABLE DEVICES Bobcat2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      (APPLICABLE RANGES: 0..2^16-1, Bobcat2,
*                                                          0..2^32-1, Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter became Read Only and Clear on Read starting from Caelum devices untill Falcon devices.
*
*/
static GT_STATUS internal_cpssDxChPtpTaiIncomingClockCounterSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_BOOL                               enable,
    IN  GT_U32                                value
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr;        /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    if ((value > BIT_31) && PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* the value parameter is ignored starting from Caelum devices untill Falcon devices */
    else if ((value >= BIT_16) && (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).
                    incomingClockInCountingEnable;
            rc = prvCpssDrvHwPpPortGroupSetRegField(
                devNum, taiIterator.portGroupId, regAddr, 0, 1,
                (BOOL2BIT_MAC(enable)));
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).TAICtrlReg0;
            rc = prvCpssDrvHwPpPortGroupSetRegField(
                devNum, taiIterator.portGroupId, regAddr, 6, 1,
                (BOOL2BIT_MAC(enable)));
        }

        if (rc != GT_OK)
        {
            return rc;
        }

        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).
                    incomingClockCounter;

            rc = prvCpssDrvHwPpPortGroupSetRegField(
                    devNum, taiIterator.portGroupId, regAddr, 0, 32, value);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        /* the value parameter is ignored starting from Caelum devices untill Falcon devices */
        else if(!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiIterator.taiNumber).
                    incomingClockInCountingConfigLow;

            rc = prvCpssDrvHwPpPortGroupSetRegField(
                    devNum, taiIterator.portGroupId, regAddr, 0, 16, value);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiIncomingClockCounterSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Incoming Clock Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   -  counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
* @param[in] value                    -  to set.
*                                      (APPLICABLE DEVICES Bobcat2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      (APPLICABLE RANGES: 0..2^16-1, Bobcat2,
*                                                          0..2^32-1, Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter became Read Only and Clear on Read starting from Caelum devices untill Falcon devices.
*
*/
GT_STATUS cpssDxChPtpTaiIncomingClockCounterSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_BOOL                               enable,
    IN  GT_U32                                value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiIncomingClockCounterSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enable, value));

    rc = internal_cpssDxChPtpTaiIncomingClockCounterSet(devNum, taiIdPtr, enable, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enable, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiIncomingClockCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Clock Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to)enable counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiIncomingClockCounterGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_BOOL                               *enablePtr,
    OUT GT_U32                                *valuePtr
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr;        /* register address */
    GT_U32                             hwValue;        /* HW data  */
    GT_U32                             fieldLength;     /* num of bits to read*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).
                incomingClockInCountingEnable;
        rc = prvCpssDrvHwPpPortGroupGetRegField(
                devNum, taiIterator.portGroupId, regAddr, 0, 1, &hwValue);
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).TAICtrlReg0;
        rc = prvCpssDrvHwPpPortGroupGetRegField(
            devNum, taiIterator.portGroupId, regAddr, 6, 1,
            &hwValue);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr  = BIT2BOOL_MAC(hwValue);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).
            incomingClockInCountingConfigLow;
    fieldLength = 16;
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).
                incomingClockCounter;
        fieldLength = 32;
    }

    return prvCpssDrvHwPpPortGroupGetRegField(
            devNum, taiIterator.portGroupId, regAddr, 0, fieldLength, valuePtr);
}

/**
* @internal cpssDxChPtpTaiIncomingClockCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Clock Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to)enable counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiIncomingClockCounterGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_BOOL                               *enablePtr,
    OUT GT_U32                                *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiIncomingClockCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enablePtr, valuePtr));

    rc = internal_cpssDxChPtpTaiIncomingClockCounterGet(devNum, taiIdPtr, enablePtr, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enablePtr, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpTaiGracefulStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] gracefulStep             - Graceful Step in nanoseconds.
*                                      (APPLICABLE RANGES: 0..31.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on wrong gracefulStep
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiGracefulStepSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC          *taiIdPtr,
    IN  GT_U32                             gracefulStep
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr;        /* register address */
    GT_U32                             fieldOffset;    /* field offset inside register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    if (gracefulStep >= BIT_5)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* For SIP6 device offset is "8" */
    fieldOffset = PRV_CPSS_SIP_6_CHECK_MAC(devNum)?8:2;

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        /* For SIP6 device gracefulstep is in timeCntrFunctionConfig0[8:12]
         * For other devices its in TAICtrlReg0[2:7] */
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                    devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                    devNum, taiIterator.gopNumber, taiIterator.taiNumber).TAICtrlReg0;
        }

        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, fieldOffset, 5, gracefulStep);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiGracefulStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] gracefulStep             - Graceful Step in nanoseconds.
*                                      (APPLICABLE RANGES: 0..31.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on wrong gracefulStep
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiGracefulStepSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC          *taiIdPtr,
    IN  GT_U32                             gracefulStep
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiGracefulStepSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, gracefulStep));

    rc = internal_cpssDxChPtpTaiGracefulStepSet(devNum, taiIdPtr, gracefulStep);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, gracefulStep));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiGracefulStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] gracefulStepPtr          - (pointer to) Graceful Step in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiGracefulStepGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC          *taiIdPtr,
    OUT GT_U32                            *gracefulStepPtr
)
{
    GT_STATUS                          rc;             /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    GT_U32                             regAddr;        /* register address */
    GT_U32                             fieldOffset;    /* field offset inside register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(gracefulStepPtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    /* For SIP6 device gracefulstep is in timeCntrFunctionConfig0[8:12]
     * For other devices its in TAICtrlReg0[2:7] */
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).TAICtrlReg0;
    }

    /* For SIP6 device offset is "8" */
    fieldOffset = PRV_CPSS_SIP_6_CHECK_MAC(devNum)?8:2;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, fieldOffset, 5, gracefulStepPtr);

    return rc;

}

/**
* @internal cpssDxChPtpTaiGracefulStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] gracefulStepPtr          - (pointer to) Graceful Step in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiGracefulStepGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC          *taiIdPtr,
    OUT GT_U32                            *gracefulStepPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiGracefulStepGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, gracefulStepPtr));

    rc = internal_cpssDxChPtpTaiGracefulStepGet(devNum, taiIdPtr, gracefulStepPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, gracefulStepPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpRxTaiSelectSet function
* @endinternal
*
* @brief   Set RX TAI select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] rxTaiSelect              - select TAI for ingress time stamping (APPLICABLE RANGES: 0..1.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChPtpRxTaiSelectSet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  GT_U32                            rxTaiSelect
)
{
    GT_STATUS rc;
    GT_U32  regAddr;        /* register address */
    GT_U32  portMacNum;     /* MAC number       */
    GT_U32  offset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    if (rxTaiSelect >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 5, 1, rxTaiSelect);
        return rc;
    }
    else
    {
        if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portControl;
            offset = 3;
        }
        else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portControl;
            offset = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? 11 : 4;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portControl;
            offset = 3;
        }
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, offset, 1, rxTaiSelect);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* configure registers for 200G/400G ports */
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).segPortControl;

        if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 5, 1, rxTaiSelect);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}
/**
* @internal cpssDxChPtpRxTaiSelectSet function
* @endinternal
*
* @brief   Set RX TAI select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] rxTaiSelect              - select TAI for ingress time stamping (APPLICABLE RANGES: 0..1.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpRxTaiSelectSet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  GT_U32                            rxTaiSelect
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpRxTaiSelectSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, rxTaiSelect));

    rc = internal_cpssDxChPtpRxTaiSelectSet(devNum, portNum, rxTaiSelect);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, rxTaiSelect));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpRxTaiSelectGet function
* @endinternal
*
* @brief   Get RX TAI select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] rxTaiSelectPtr           - (pointer to) rx TAI select.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChPtpRxTaiSelectGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT  GT_U32                           *rxTaiSelectPtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  portMacNum;     /* MAC number       */
    GT_STATUS rc;           /* return code      */
    GT_U32 offset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    CPSS_NULL_PTR_CHECK_MAC(rxTaiSelectPtr);

    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 5, 1, rxTaiSelectPtr);
        return rc;
    }
    else
    {
        if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portControl;
            offset = 3;
        }
        else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portControl;
            offset = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? 11 : 4;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portControl;
            offset = 3;
        }
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, offset, 1, rxTaiSelectPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* configure registers for 200G/400G ports */
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).segPortControl;

        if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 5, 1, rxTaiSelectPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}
/**
* @internal cpssDxChPtpRxTaiSelectGet function
* @endinternal
*
* @brief   Get RX TAI select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] rxTaiSelectPtr           - (pointer to) rx TAI select.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpRxTaiSelectGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT  GT_U32                           *rxTaiSelectPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpRxTaiSelectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, rxTaiSelectPtr));

    rc = internal_cpssDxChPtpRxTaiSelectGet(devNum, portNum, rxTaiSelectPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, rxTaiSelectPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsuBypassEnableSet function
* @endinternal
*
* @brief   Set TSU (Timestamping Unit) bypass enable.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] tsuBypassEnable          - Timestamping unit bypass enable.
*                                       GT_TRUE:  the TSU is bypassed, allowing
*                                                 egress traffic with reduced latency.
*                                       GT_FALSE: all egress traffic passes through
*                                                 CTSU pipeline, even if not processed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsuBypassEnableSet
(
    IN  GT_U8                             devNum,
    IN  GT_BOOL                           tsuBypassEnable
)
{
    GT_STATUS rc;
    GT_U32  regAddr;        /* register address */
    GT_U32  hwData;         /* HW data          */
    GT_U32  tsuIndex;       /* Timestamp unit index         */
    GT_U32  numOfIndices;   /* number of channel indices in device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    hwData = BOOL2BIT_MAC(tsuBypassEnable);
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        numOfIndices = 1;
    }
    else if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        numOfIndices = 3;
    }
    else
    {
        numOfIndices = PRV_CPSS_SIP_6_15_CHECK_MAC(devNum) ? 1 : 4;
    }
    for (tsuIndex = 0; tsuIndex < numOfIndices; tsuIndex++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].ctsuControl;
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 1, 1, hwData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsuBypassEnableSet function
* @endinternal
*
* @brief   Set TSU (Timestamping Unit) bypass enable.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] tsuBypassEnable          - Timestamping unit bypass enable.
*                                       GT_TRUE:  the TSU is bypassed, allowing
*                                                 egress traffic with reduced latency.
*                                       GT_FALSE: all egress traffic passes through
*                                                 CTSU pipeline, even if not processed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuBypassEnableSet
(
    IN  GT_U8                             devNum,
    IN  GT_BOOL                           tsuBypassEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuBypassEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tsuBypassEnable));

    rc = internal_cpssDxChPtpTsuBypassEnableSet(devNum, tsuBypassEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tsuBypassEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpTsuBypassEnableGet function
* @endinternal
*
* @brief   Get TSU (Timestamping Unit) bypass (enable/disable) state.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[out] tsuBypassEnablePtr      - (pointer to) Timestamping unit bypass enable.
*                                       GT_TRUE:  the TSU is bypassed, allowing
*                                                 egress traffic with reduced latency.
*                                       GT_FALSE: all egress traffic passes through
*                                                 TSU pipeline, even if not processed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsuBypassEnableGet

(
    IN  GT_U8                             devNum,
    OUT  GT_BOOL                          *tsuBypassEnablePtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  hwData;         /* HW data          */
    GT_STATUS rc;           /* return code      */
    GT_U32  tsuIndex;
    GT_U32  tsuLocalChannel;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AC5P_E  | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(tsuBypassEnablePtr);

    tsuIndex        = 0;
    tsuLocalChannel = 0;

    rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(devNum, 0, &tsuIndex, &tsuLocalChannel);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].ctsuControl;

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 1, 1, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    *tsuBypassEnablePtr = BIT2BOOL_MAC(hwData);

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsuBypassEnableGet function
* @endinternal
*
* @brief   Get TSU (Timestamping Unit) bypass (enable/disable) state.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[out] tsuBypassEnablePtr      - (pointer to) Timestamping unit bypass enable.
*                                       GT_TRUE:  the TSU is bypassed, allowing
*                                                 egress traffic with reduced latency.
*                                       GT_FALSE: all egress traffic passes through
*                                                 TSU pipeline, even if not processed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuBypassEnableGet
(
    IN  GT_U8                             devNum,
    OUT GT_BOOL                           *tsuBypassEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuBypassEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tsuBypassEnablePtr));

    rc = internal_cpssDxChPtpTsuBypassEnableGet(devNum, tsuBypassEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tsuBypassEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChPtpTsuEnableSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsuEnable               - Timestamp unit enable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChPtpTsuEnableSet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  GT_BOOL                           tsuEnable
)
{
    GT_STATUS rc;
    GT_U32  regAddr;        /* register address */
    GT_U32  hwData;         /* HW data          */
    GT_U32  offset;         /* offset          */
    GT_U32  portMacNum;     /* MAC number       */
    GT_U32      tsuIndex;       /* Timestamp unit index         */
    GT_U32      tsuLocalChannel;/* Timestamp unit local channel */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    hwData = BOOL2BIT_MAC(tsuEnable);
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        tsuIndex        = 0;
        tsuLocalChannel = 0;

        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelTsControl[tsuLocalChannel];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl; /* called TSU control in falcon */
    }

    offset = PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)  ? 1:0;

    return prvCpssDrvHwPpSetRegField(devNum, regAddr, offset, 1, hwData);
}
/**
* @internal cpssDxChPtpTsuEnableSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsuEnable               - Timestamp unit enable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuEnableSet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  GT_BOOL                           tsuEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tsuEnable));

    rc = internal_cpssDxChPtpTsuEnableSet(devNum, portNum, tsuEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tsuEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpTsuEnableGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[out] tsuEnablePtr            - (pointer to) timestamp unit enable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChPtpTsuEnableGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT  GT_BOOL                          *tsuEnablePtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  hwData;         /* HW data          */
    GT_U32  offset;         /* offset          */
    GT_U32  portMacNum;     /* MAC number       */
    GT_STATUS rc;           /* return code      */
    GT_U32  tsuIndex;
    GT_U32  tsuLocalChannel;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    CPSS_NULL_PTR_CHECK_MAC(tsuEnablePtr);

    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        tsuIndex        = 0;
        tsuLocalChannel = 0;

        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
         regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelTsControl[tsuLocalChannel];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl; /* called TSU control in falcon */
    }

    offset = PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 1:0;

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, offset, 1, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    *tsuEnablePtr = BIT2BOOL_MAC(hwData);

    return GT_OK;
}
/**
* @internal cpssDxChPtpTsuEnableGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[out] tsuEnablePtr            - (pointer to) timestamp unit enable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuEnableGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT GT_BOOL                           *tsuEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tsuEnablePtr));

    rc = internal_cpssDxChPtpTsuEnableGet(devNum, portNum, tsuEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tsuEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsuControlSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] controlPtr               - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsuControlSet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  CPSS_DXCH_PTP_TSU_CONTROL_STC     *controlPtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  hwData;         /* HW data          */
    GT_U32  hwMask;         /* HW mask          */
    GT_U32  portMacNum;     /* MAC number       */
    GT_U32  tsuIndex;       /* Timestamp unit index         */
    GT_U32  tsuLocalChannel;/* Timestamp unit local channel */
    GT_STATUS rc;           /* return code      */
    GT_U32  rxTaiSelect;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(controlPtr);
    /* controlPtr->taiNumber is not supported in Ironman */
    if ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E) && (controlPtr->taiNumber >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    hwMask = 0;
    hwData = 0;

    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        hwData |= BOOL2BIT_MAC(controlPtr->unitEnable);
        hwMask |= 1;

        hwData |= (controlPtr->taiNumber << 5);
        hwMask |= (1 << 5);
        hwMask |= (1 << 6);
    }
    else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        hwData = BOOL2BIT_MAC(controlPtr->unitEnable);
        hwMask = (1);
    }
    else
    {
        hwData = (BOOL2BIT_MAC(controlPtr->unitEnable)) << 1;
        hwMask = (1 << 1);
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelTsControl[tsuLocalChannel];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;
    }

    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, hwMask, hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* code below is for SIP_6 only */
    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum) || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E))
        return GT_OK;

    rxTaiSelect = (controlPtr->taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_0_E ? 0 : 1);
    return cpssDxChPtpRxTaiSelectSet(devNum,portNum,rxTaiSelect);

}

/**
* @internal cpssDxChPtpTsuControlSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] controlPtr               - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuControlSet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  CPSS_DXCH_PTP_TSU_CONTROL_STC     *controlPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuControlSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, controlPtr));

    rc = internal_cpssDxChPtpTsuControlSet(devNum, portNum, controlPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, controlPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsuControlGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] controlPtr               - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsuControlGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT CPSS_DXCH_PTP_TSU_CONTROL_STC     *controlPtr
)
{
    GT_STATUS   rc;             /* return code      */
    GT_U32      regAddr;        /* register address */
    GT_U32      hwData;         /* HW data          */
    GT_U32      portMacNum;     /* MAC number       */
    GT_U32      tsuIndex;       /* Timestamp unit index         */
    GT_U32      tsuLocalChannel;/* Timestamp unit local channel */
    GT_U32      rxTaiSelect;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(controlPtr);

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    cpssOsMemSet(controlPtr, 0, sizeof(*controlPtr));

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelTsControl[tsuLocalChannel];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;
    }

    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        controlPtr->unitEnable = BIT2BOOL_MAC((hwData & 1));

        controlPtr->taiNumber = ((hwData >> 5) & 1);

    }
    else
    {
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            controlPtr->unitEnable = BIT2BOOL_MAC(hwData & 1);
        }
        else
        {
            controlPtr->unitEnable = BIT2BOOL_MAC((hwData >> 1) & 1);
        }
        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            rc = cpssDxChPtpRxTaiSelectGet(devNum,portNum,&rxTaiSelect);
            if (rc != GT_OK)
            {
                return rc;
            }
            controlPtr->taiNumber = (CPSS_DXCH_PTP_TAI_NUMBER_ENT)rxTaiSelect;
        }
    }
    return GT_OK;
}

/**
* @internal cpssDxChPtpTsuControlGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] controlPtr               - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuControlGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT CPSS_DXCH_PTP_TSU_CONTROL_STC     *controlPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuControlGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, controlPtr));

    rc = internal_cpssDxChPtpTsuControlGet(devNum, portNum, controlPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, controlPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsuTxTimestampQueueRead function
* @endinternal
*
* @brief   Read TSU (Time Stamp Unit) TX Timestamp Queue Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] queueNum                 - queue number.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] entryPtr                 - (pointer to) TX Timestamp Queue Entry structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Entry deleted from the Queue by reading it.
*
*/
static GT_STATUS internal_cpssDxChPtpTsuTxTimestampQueueRead
(
    IN  GT_U8                                              devNum,
    IN  GT_PHYSICAL_PORT_NUM                               portNum,
    IN  GT_U32                                             queueNum,
    OUT CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC     *entryPtr
)
{
    GT_STATUS   rc;                         /* return code      */
    GT_U32      regAddr[3];                 /* register address */
    GT_U32      entryClearRegAddr = 0;      /* register address */
    GT_U32      hwData[3];                  /* HW data          */
    GT_U32      i;                          /* loop index       */
    GT_U32      portGroupId;                /* Port Group Id    */
    GT_U32      localPort;                  /* local port       */
    GT_U32      portMacNum;                 /* MAC number       */
    GT_U32      numRegisters;               /* number of registers  */
    GT_U32      tsuIndex;                   /* Timestamp unit index         */
    GT_U32      tsuLocalChannel;            /* Timestamp unit local channel */
    GT_BOOL     invalidate = GT_TRUE;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    if (queueNum >= PRV_CPSS_MAX_PTP_TIMESTAMP_QUEUES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "queueNum[%d] must be less than [%d]",
                                      queueNum,
                                      PRV_CPSS_MAX_PTP_TIMESTAMP_QUEUES_CNS);
    }

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    portGroupId     = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPort       = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        localPort   = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);
    }

    cpssOsMemSet(entryPtr, 0, sizeof(*entryPtr));

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        if (queueNum == 0)
        {
            regAddr[0] =
                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].
                    channelEgressTsQueue0EntryLow[tsuLocalChannel];
            regAddr[1] =
                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].
                    channelEgressTsQueue0EntryHigh[tsuLocalChannel];
            entryClearRegAddr =
                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].
                    channelTsQueue0EntryClear[tsuLocalChannel];
        }
        else
        {
            regAddr[0] =
                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].
                    channelEgressTsQueue1EntryLow[tsuLocalChannel];
            regAddr[1] =
                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].
                    channelEgressTsQueue1EntryHigh[tsuLocalChannel];
            entryClearRegAddr =
                PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].
                    channelTsQueue1EntryClear[tsuLocalChannel];
        }
    }
    else
    {
        switch (queueNum)
        {
            case 0:
                regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).PTPTXTimestampQueue0Reg0;
                regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).PTPTXTimestampQueue0Reg1;
                regAddr[2] = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).PTPTXTimestampQueue0Reg2;
                break;
            case 1:
                regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).PTPTXTimestampQueue1Reg0;
                regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).PTPTXTimestampQueue1Reg1;
                regAddr[2] = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).PTPTXTimestampQueue1Reg2;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    numRegisters = PRV_CPSS_SIP_6_CHECK_MAC(devNum)? 2:3;

    for (i = 0; (i < numRegisters); i++)
    {
        rc = prvCpssDrvHwPpPortGroupReadRegister(
            devNum, portGroupId, regAddr[i], &(hwData[i]));
        if (rc != GT_OK)
        {
            return rc;
        }

        /* check that the first word is valid */
        if ((i == 0) &&  ((hwData[0] & 1) == 0) )
        {

          /* Do not read/clear word 1/2.
             The design is not well protected if reading an empty queue word1/word2 while a new entry is pushed in */
          invalidate = GT_FALSE;
          break;
        }
    }

    /* invalidate (pop) the entry in the queue; indication for hw to invalidate the entry. */
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) && (invalidate == GT_TRUE))
    {
        rc = prvCpssDrvHwPpPortGroupWriteRegister(
        devNum, portGroupId, entryClearRegAddr, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpPortGroupWriteRegister(
        devNum, portGroupId, entryClearRegAddr, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }


    entryPtr->entryValid     = BIT2BOOL_MAC((hwData[0] & 1));
    entryPtr->queueEntryId   = ((hwData[0] >> 1) & 0x3FF);
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        entryPtr->taiSelect      = ((hwData[0] >> 13) & 3);
    }
    else
    {
        entryPtr->taiSelect      = ((hwData[0] >> 11) & 1);
    }
    entryPtr->todUpdateFlag  = ((hwData[0] >> 12) & 1);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        entryPtr->timestamp = hwData[1];
    }
    else
    {
        entryPtr->timestamp      =
            ((hwData[0] >> 13) & 7)
            | ((hwData[1] & 0xFFFF) << 3)
            | ((hwData[2] & 0x1FFF) << 19);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsuTxTimestampQueueRead function
* @endinternal
*
* @brief   Read TSU (Time Stamp Unit) TX Timestamp Queue Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] queueNum                 - queue number.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] entryPtr                 - (pointer to) TX Timestamp Queue Entry structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Entry deleted from the Queue by reading it.
*
*/
GT_STATUS cpssDxChPtpTsuTxTimestampQueueRead
(
    IN  GT_U8                                              devNum,
    IN  GT_PHYSICAL_PORT_NUM                               portNum,
    IN  GT_U32                                             queueNum,
    OUT CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC     *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuTxTimestampQueueRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, queueNum, entryPtr));

    rc = internal_cpssDxChPtpTsuTxTimestampQueueRead(devNum, portNum, queueNum, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, queueNum, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsuCountersClear function
* @endinternal
*
* @brief   Clear All TSU (Time Stamp Unit) Packet counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsuCountersClear
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum
)
{
    GT_STATUS rc;
    GT_U32  regAddr;        /* register address */
    GT_U32  portGroupId;    /* Port Group Id    */
    GT_U32  localPort;      /* local port       */
    GT_U32  portMacNum;     /* MAC number       */
    GT_U32  index;          /* counter index */
    GT_U32  tsuIndex;       /* Timestamp unit index         */
    GT_U32  tsuLocalChannel;/* Timestamp unit local channel */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    portGroupId     = 0;
    localPort       = 0;

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
        localPort   = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);
    }

    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).PTPGeneralCtrl;

        return prvCpssDrvHwPpPortGroupSetRegField(
            devNum, portGroupId, regAddr, 4/*offset*/, 1/*length*/, 1);
    }
    else
    {
        switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            for (index = 0; index < 4; index++)
            {
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).timestampFrameCntrControl[index];
                rc = prvCpssHwPpSetRegField(devNum, regAddr, 11, 1, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            break;

        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            for (index = 0; index < 31; index++)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].tsFrameCounterControl[index];
                rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 31, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            break;

        default:
            for (index = 0; index < 64; index++)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].tsFrameCounterControl[index];
                rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 31, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            break;
        }

        return GT_OK;
    }
}

/**
* @internal cpssDxChPtpTsuCountersClear function
* @endinternal
*
* @brief   Clear All TSU (Time Stamp Unit) Packet counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuCountersClear
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuCountersClear);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum));

    rc = internal_cpssDxChPtpTsuCountersClear(devNum, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChPtpTsuPacketCounterGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) packet counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] counterType              - counter type (see enum)
* @param[in] Index                    - counter index. (APPLICABLE DEVICES: Falcon, AC5P, AC5X, Harrier, Ironman.
*                                       Falcon: APPLICABLE RANGE: 0..3.
*                                       AC5P,Harrier: APPLICABLE RANGE: 0..31.
*                                       AC5X,Ironman: APPLICABLE RANGE: 0..63.)
*
* @param[out] valuePtr                 - (pointer to) counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsuPacketCounterGet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT counterType,
    IN  GT_U32                                    index,
    OUT GT_U32                                    *valuePtr
)
{
    GT_STATUS rc;           /* returned code*/
    GT_U32  regAddr;        /* register address */
    GT_U32  portGroupId;    /* Port Group Id    */
    GT_U32  localPort;      /* local port       */
    GT_U32  portMacNum;     /* MAC number       */
    GT_U32  length;         /* length of data   */
    GT_U32  tsuIndex;       /* Timestamp unit index         */
    GT_U32  tsuLocalChannel;/* Timestamp unit local channel */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && (counterType != CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E)
        && (counterType != CPSS_DXCH_PTP_TSU_PACKET_COUNTER_INDEX_E ))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
    case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
        if (index > 63)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        if (index > 31)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    default:
        if (index > 3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    }

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    length = PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? 32 : 8;
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPort   = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        switch (counterType)
        {
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E:
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_INDEX_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].tsFrameCounter[index];
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        switch (counterType)
        {
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E:
                regAddr = PRV_CPSS_SIP_6_CHECK_MAC(devNum)? PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).timestampFrameCntr[index]:
                  PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).totalPTPPktsCntr  ;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_INDEX_E:      /* for falcon only*/
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).timestampFrameCntr[index];
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V1_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).PTPv1PktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V2_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).PTPv2PktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_Y1731_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).Y1731PktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NTP_TS_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).NTPTsPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NTP_RX_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).NTPReceivePktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NTP_TX_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).NTPTransmitPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_WAMP_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).WAMPPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NONE_ACTION_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).noneActionPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_FORWARD_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).forwardActionPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_DROP_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).dropActionPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).captureActionPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ADD_TIME_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).addTimeActionPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ADD_CORR_TIME_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).addCorrectedTimeActionPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_TIME_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).captureAddTimeActionPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_CORR_TIME_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).captureAddCorrectedTimeActionPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ADD_INGRESS_TIME_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).addIngrTimeActionPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_INGRESS_TIME_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).captureAddIngrTimeActionPktCntr;
                break;
            case CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_INGRESS_TIME_E:
                regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).captureIngrTimeActionPktCntr;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return prvCpssDrvHwPpPortGroupGetRegField(
        devNum, portGroupId, regAddr, 0/*offset*/, length, valuePtr);
}

/**
* @internal cpssDxChPtpTsuPacketCounterGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) packet counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] counterType              - counter type (see enum)
* @param[in] Index                    - counter index. (APPLICABLE DEVICES: Falcon, AC5P, AC5X, Harrier, Ironman.
*                                       Falcon: APPLICABLE RANGE: 0..3.
*                                       AC5P,Harrier: APPLICABLE RANGE: 0..31.
*                                       AC5X,Ironman: APPLICABLE RANGE: 0..63.)
*
* @param[out] valuePtr                 - (pointer to) counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuPacketCounterGet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT counterType,
    IN  GT_U32                                    index,
    OUT GT_U32                                    *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuPacketCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, counterType, index, valuePtr));

    rc = internal_cpssDxChPtpTsuPacketCounterGet(devNum, portNum, counterType, index, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, counterType, index, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpTsuNtpTimeOffsetGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] ntpTimeOffsetPtr         - (pointer to) NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsuNtpTimeOffsetGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT GT_U32                            *ntpTimeOffsetPtr
)
{
    GT_STATUS   rc;             /* return code      */
    GT_U32      regAddr[2];     /* register address */
    GT_U32      hwData[2];      /* HW data          */
    GT_U32      i;              /* loop index       */
    GT_U32      portGroupId;    /* Port Group Id    */
    GT_U32      localPort;      /* local port       */
    GT_U32      portMacNum;     /* MAC number       */
    GT_U32  tsuIndex;       /* Timestamp unit index         */
    GT_U32  tsuLocalChannel;/* Timestamp unit local channel */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(ntpTimeOffsetPtr);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPort   = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        tsuIndex        = 0;
        tsuLocalChannel = 0;
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
                devNum, portNum, &tsuIndex, &tsuLocalChannel);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            regAddr[0] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].ptpNtpOffset;
        }
        else
        {
            regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).NTPPTPOffsetLow;
        }
        rc = prvCpssDrvHwPpPortGroupReadRegister(
            devNum, portGroupId, regAddr[0],ntpTimeOffsetPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).NTPPTPOffsetHigh;
        regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).NTPPTPOffsetLow;

        for (i = 0; (i < 2); i++)
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(
                devNum, portGroupId, regAddr[i], 0, 16, &(hwData[i]));
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        *ntpTimeOffsetPtr = ((hwData[1] << 16) | hwData[0]);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsuNtpTimeOffsetGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] ntpTimeOffsetPtr         - (pointer to) NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuNtpTimeOffsetGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT GT_U32                            *ntpTimeOffsetPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuNtpTimeOffsetGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ntpTimeOffsetPtr));

    rc = internal_cpssDxChPtpTsuNtpTimeOffsetGet(devNum, portNum, ntpTimeOffsetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ntpTimeOffsetPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsTagGlobalCfgSet function
* @endinternal
*
* @brief   Set Timestamp Global Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] tsTagGlobalCfgPtr        - (pointer to) TS Tag Global Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsTagGlobalCfgSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC *tsTagGlobalCfgPtr
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    regAddr;          /* register address             */
    GT_U32    hwData;           /* HW data                      */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(tsTagGlobalCfgPtr);

    if (tsTagGlobalCfgPtr->tsTagEtherType > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (tsTagGlobalCfgPtr->hybridTsTagEtherType > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* TTI data */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.timestampEtherTypes;

    hwData = tsTagGlobalCfgPtr->tsTagEtherType
        | (tsTagGlobalCfgPtr->hybridTsTagEtherType << 16);

    rc = prvCpssHwPpWriteRegister(devNum, regAddr, hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.timestampConfigs;

    hwData = BOOL2BIT_MAC(tsTagGlobalCfgPtr->tsTagParseEnable)
        | (BOOL2BIT_MAC(tsTagGlobalCfgPtr->hybridTsTagParseEnable) << 1);

    rc = prvCpssHwPpSetRegField(
        devNum, regAddr, 0, 2, hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* HA data */
    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).PTPTimestampTagEthertype;

    hwData = tsTagGlobalCfgPtr->hybridTsTagEtherType
        | (tsTagGlobalCfgPtr->tsTagEtherType << 16);

    rc = prvCpssHwPpWriteRegister(devNum, regAddr, hwData);

    return rc;
}

/**
* @internal cpssDxChPtpTsTagGlobalCfgSet function
* @endinternal
*
* @brief   Set Timestamp Global Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] tsTagGlobalCfgPtr        - (pointer to) TS Tag Global Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsTagGlobalCfgSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC *tsTagGlobalCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsTagGlobalCfgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tsTagGlobalCfgPtr));

    rc = internal_cpssDxChPtpTsTagGlobalCfgSet(devNum, tsTagGlobalCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tsTagGlobalCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsTagGlobalCfgGet function
* @endinternal
*
* @brief   Get Timestamp Global Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] tsTagGlobalCfgPtr        - (pointer to) TS Tag Global Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsTagGlobalCfgGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC *tsTagGlobalCfgPtr
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    regAddr;          /* register address             */
    GT_U32    hwData;           /* HW data                      */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(tsTagGlobalCfgPtr);

    cpssOsMemSet(tsTagGlobalCfgPtr, 0, sizeof(*tsTagGlobalCfgPtr));

    /* TTI data - the HA copy of the same Ethertypes ignored */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.timestampEtherTypes;

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    tsTagGlobalCfgPtr->tsTagEtherType       = (hwData & 0xFFFF);
    tsTagGlobalCfgPtr->hybridTsTagEtherType = ((hwData >> 16) & 0xFFFF);


    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.timestampConfigs;

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, 0, 2, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }
    tsTagGlobalCfgPtr->tsTagParseEnable       = BIT2BOOL_MAC((hwData & 1));
    tsTagGlobalCfgPtr->hybridTsTagParseEnable = BIT2BOOL_MAC(((hwData >> 1) & 1));

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsTagGlobalCfgGet function
* @endinternal
*
* @brief   Get Timestamp Global Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] tsTagGlobalCfgPtr        - (pointer to) TS Tag Global Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsTagGlobalCfgGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC *tsTagGlobalCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsTagGlobalCfgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tsTagGlobalCfgPtr));

    rc = internal_cpssDxChPtpTsTagGlobalCfgGet(devNum, tsTagGlobalCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tsTagGlobalCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsTagPortCfgSet function
* @endinternal
*
* @brief   Set Timestamp Port Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsTagPortCfgPtr          - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsTagPortCfgSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   *tsTagPortCfgPtr
)
{
    GT_STATUS rc;               /* return code                      */
    GT_U32    regAddr;          /* register address                 */
    GT_U32    fieldOffset;      /* field offset inside register     */
    GT_U32    regIndex;         /* index of register                */
    GT_U32    hwValue;          /* HW value                         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(tsTagPortCfgPtr);

    hwValue = BOOL2BIT_MAC(tsTagPortCfgPtr->tsPiggyBackEnable);
    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChWriteTableEntryField(
            devNum,
            CPSS_DXCH_SIP5_20_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            240,
            1 /*fieldLength*/,
            hwValue);
    }
    else
    {
        regIndex    = (portNum / 32);
        fieldOffset = (portNum % 32);
        if (regIndex >=8 )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.piggyBackTSConfigs[regIndex];
        rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, hwValue);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    switch (tsTagPortCfgPtr->tsTagMode)
    {
        case CPSS_DXCH_PTP_TS_TAG_MODE_NONE_E:
            hwValue = 0;
            break;
        case CPSS_DXCH_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E:
            hwValue = 1;
            break;
        case CPSS_DXCH_PTP_TS_TAG_MODE_ALL_EXTENDED_E:
            hwValue = 2;
            break;
        case CPSS_DXCH_PTP_TS_TAG_MODE_NON_EXTENDED_E:
            hwValue = 3;
            break;
        case CPSS_DXCH_PTP_TS_TAG_MODE_EXTENDED_E:
            hwValue = 4;
            break;
        case CPSS_DXCH_PTP_TS_TAG_MODE_PIGGYBACK_E:
            hwValue = 5;
            break;
        case CPSS_DXCH_PTP_TS_TAG_MODE_HYBRID_E:
            hwValue = 6;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* timeStampTagMode in Falcon is [34:36] */
    fieldOffset = PRV_CPSS_SIP_6_CHECK_MAC(devNum)?34:35;

    rc = prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        fieldOffset, 3 /*fieldLength*/, hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    hwValue = BOOL2BIT_MAC(tsTagPortCfgPtr->tsReceptionEnable);
    /* timeStamp Reception Mode in Falcon is 38 */
    fieldOffset = PRV_CPSS_SIP_6_CHECK_MAC(devNum)?38:39;

    rc = prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        fieldOffset, 1 /*fieldLength*/, hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* Piggyback TST Mode:
           The 2 LSBits of the ns value are overridden with the T (TAI select - 0/1) and U (update counter LSB).
           Marvell PHYs piggyback/Hybrid TST doesn't include them (i.e. they parse/build it assuming these are 2 ns bits).
           Set value MODE_1NS_RES '1' as default; use ns LSBits */
        rc = prvCpssDxChWriteTableEntryField(
            devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
            portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            321 /*fieldOffset*/, 1 /*fieldLength*/, 1);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsTagPortCfgSet function
* @endinternal
*
* @brief   Set Timestamp Port Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsTagPortCfgPtr          - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsTagPortCfgSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   *tsTagPortCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsTagPortCfgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tsTagPortCfgPtr));

    rc = internal_cpssDxChPtpTsTagPortCfgSet(devNum, portNum, tsTagPortCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tsTagPortCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsTagPortCfgGet function
* @endinternal
*
* @brief   Get Timestamp Port Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] tsTagPortCfgPtr          - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsTagPortCfgGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   *tsTagPortCfgPtr
)
{
    GT_STATUS rc;               /* return code                      */
    GT_U32    regAddr;          /* register address                 */
    GT_U32    fieldOffset;      /* field offset inside register     */
    GT_U32    regIndex;         /* index of register                */
    GT_U32    hwValue;          /* HW value                         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(tsTagPortCfgPtr);

    cpssOsMemSet(tsTagPortCfgPtr, 0, sizeof(*tsTagPortCfgPtr));

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChReadTableEntryField(
            devNum,
            CPSS_DXCH_SIP5_20_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
            240,
            1 /*fieldLength*/,
            &hwValue);
    }
    else
    {
        regIndex    = (portNum / 32);
        fieldOffset = (portNum % 32);
        if (regIndex >=8 )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.piggyBackTSConfigs[regIndex];

        rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 1, &hwValue);
    }
    if (rc != GT_OK)
    {
        return rc;
    }
    tsTagPortCfgPtr->tsPiggyBackEnable = BIT2BOOL_MAC(hwValue);

    /* timeStampTagMode in Falcon is [34:36] */
    fieldOffset = PRV_CPSS_SIP_6_CHECK_MAC(devNum)?34:35;

    rc = prvCpssDxChReadTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        fieldOffset, 3 /*fieldLength*/, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (hwValue)
    {
        default:
        case 0:
            tsTagPortCfgPtr->tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_NONE_E;
            break;
        case 1:
            tsTagPortCfgPtr->tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E;
            break;
        case 2:
            tsTagPortCfgPtr->tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_ALL_EXTENDED_E;
            break;
        case 3:
            tsTagPortCfgPtr->tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_NON_EXTENDED_E;
            break;
        case 4:
            tsTagPortCfgPtr->tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_EXTENDED_E;
            break;
        case 5:
            tsTagPortCfgPtr->tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_PIGGYBACK_E;
            break;
        case 6:
            tsTagPortCfgPtr->tsTagMode = CPSS_DXCH_PTP_TS_TAG_MODE_HYBRID_E;
            break;
    }

    /* timeStamp Reception Mode in Falcon is 38 */
    fieldOffset = PRV_CPSS_SIP_6_CHECK_MAC(devNum)?38:39;
    rc = prvCpssDxChReadTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        fieldOffset, 1 /*fieldLength*/, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    tsTagPortCfgPtr->tsReceptionEnable = BIT2BOOL_MAC(hwValue);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTsTagPortCfgGet function
* @endinternal
*
* @brief   Get Timestamp Port Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] tsTagPortCfgPtr          - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsTagPortCfgGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   *tsTagPortCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsTagPortCfgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tsTagPortCfgPtr));

    rc = internal_cpssDxChPtpTsTagPortCfgGet(devNum, portNum, tsTagPortCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tsTagPortCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayIngressPortDelaySet function
* @endinternal
*
* @brief   Set Ingress Port Delay Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] ingressDelayCorr         - Ingress Port Delay Correction.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayIngressPortDelaySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_32                   ingressDelayCorr
)
{

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    if ((ingressDelayCorr < -1000000000) || (ingressDelayCorr > 1000000000))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_SOURCE_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        0/*fieldOffset*/, 32 /*fieldLength*/, (GT_U32)ingressDelayCorr);

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsDelayIngressPortDelaySet function
* @endinternal
*
* @brief   Set Ingress Port Delay Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] ingressDelayCorr         - Ingress Port Delay Correction.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayIngressPortDelaySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_32                   ingressDelayCorr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayIngressPortDelaySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ingressDelayCorr));

    rc = internal_cpssDxChPtpTsDelayIngressPortDelaySet(devNum, portNum, ingressDelayCorr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ingressDelayCorr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayIngressPortDelayGet function
* @endinternal
*
* @brief   Get Ingress Port Delay Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] ingressDelayCorrPtr      - (pointer to) Ingress Port Delay Correction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayIngressPortDelayGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_32                   *ingressDelayCorrPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(ingressDelayCorrPtr);

    return prvCpssDxChReadTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_SOURCE_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        0/*fieldOffset*/, 32 /*fieldLength*/, (GT_U32*)ingressDelayCorrPtr);
}

/**
* @internal cpssDxChPtpTsDelayIngressPortDelayGet function
* @endinternal
*
* @brief   Get Ingress Port Delay Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] ingressDelayCorrPtr      - (pointer to) Ingress Port Delay Correction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayIngressPortDelayGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_32                   *ingressDelayCorrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayIngressPortDelayGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ingressDelayCorrPtr));

    rc = internal_cpssDxChPtpTsDelayIngressPortDelayGet(devNum, portNum, ingressDelayCorrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ingressDelayCorrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayIngressLinkDelaySet function
* @endinternal
*
* @brief   Set Ingress Link Delay.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile
*                                      (APPLICABLE RANGES: 0..8.)
* @param[in] ingressDelay             - Ingress Port Delay.
*                                      (APPLICABLE RANGES: -10^9..-10^9.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayIngressLinkDelaySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainProfile,
    IN  GT_32                   ingressDelay
)
{

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    if (domainProfile > 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if ((ingressDelay < -1000000000) || (ingressDelay > 1000000000))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_SOURCE_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        (32 + (32 * domainProfile))/*fieldOffset*/, 32 /*fieldLength*/, (GT_U32)ingressDelay);


    return GT_OK;
}

/**
* @internal cpssDxChPtpTsDelayIngressLinkDelaySet function
* @endinternal
*
* @brief   Set Ingress Link Delay.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile
*                                      (APPLICABLE RANGES: 0..8.)
* @param[in] ingressDelay             - Ingress Port Delay.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayIngressLinkDelaySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainProfile,
    IN  GT_32                   ingressDelay
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayIngressLinkDelaySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, domainProfile, ingressDelay));

    rc = internal_cpssDxChPtpTsDelayIngressLinkDelaySet(devNum, portNum, domainProfile, ingressDelay);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, domainProfile, ingressDelay));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayIngressLinkDelayGet function
* @endinternal
*
* @brief   Get Ingress Link Delay.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile
*                                      (APPLICABLE RANGES: 0..8.)
*
* @param[out] ingressDelayPtr          - (pointer to) Ingress Port Delay.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayIngressLinkDelayGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainProfile,
    OUT GT_32                   *ingressDelayPtr
)
{
    GT_STATUS rc;       /* return code */
    GT_U32    hwVal;    /* HW value    */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(ingressDelayPtr);

    if (domainProfile > 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChReadTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_SOURCE_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        (32 + (32 * domainProfile))/*fieldOffset*/, 32 /*fieldLength*/, &hwVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    *ingressDelayPtr = (GT_32)hwVal;
    return GT_OK;
}

/**
* @internal cpssDxChPtpTsDelayIngressLinkDelayGet function
* @endinternal
*
* @brief   Get Ingress Link Delay.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile
*                                      (APPLICABLE RANGES: 0..8.)
*
* @param[out] ingressDelayPtr          - (pointer to) Ingress Port Delay.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayIngressLinkDelayGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainProfile,
    OUT GT_32                   *ingressDelayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayIngressLinkDelayGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, domainProfile, ingressDelayPtr));

    rc = internal_cpssDxChPtpTsDelayIngressLinkDelayGet(devNum, portNum, domainProfile, ingressDelayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, domainProfile, ingressDelayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayEgressPipeDelaySet function
* @endinternal
*
* @brief   Set Egress Pipe Delay Corrections.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not supported for CPU ports).
* @param[in] egrDelayValPtr           - (pointer to) Egress Pipe Delay Corrections.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayEgressPipeDelaySet
(
    IN  GT_U8                                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                            portNum,
    IN  CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC   *egrDelayValPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 portMacNum;
    GT_U32 tsuIndex = 0;
    GT_U32 tsuLocalChannel = 0;
    GT_U32 fsuOffset;

    /* check parameters part 1 */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(egrDelayValPtr);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    if ((egrDelayValPtr->egressPipeDelay < 0) || (egrDelayValPtr->egressPipeDelay > 1000000000))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(devNum, portNum, &tsuIndex, &tsuLocalChannel);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portPeerDelay;
        }
        else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portPeerDelay;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portPeerDelay;
        }
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 30, egrDelayValPtr->egressPipeDelay);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Writing 1 triggers load of the 30-bit value in bits[29:0] of this register; Self-clear bit*/
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 31, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* configure registers for 200G/400G ports */
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).segPortPeerDelay;

        if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            /* The egress pipe delay of segmented port %n, represented as an unsigned 30 bit nanosecond value */
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 30, egrDelayValPtr->egressPipeDelay);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* Writing 1 triggers load of the 30-bit value in bits[29:0] of this register; Self-clear bit*/
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 31, 1, 1);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        rc = prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        2/*fieldOffset*/, 30 /*fieldLength*/, (GT_U32)egrDelayValPtr->egressPipeDelay);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Timestamp to the Future is not supported in AC5X; Ironman */
    if (!(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)) || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E))
         return GT_OK;

    /* check parameters part 2 */
    if ((egrDelayValPtr->egressPipeDelayEncrypted < 0) || (egrDelayValPtr->egressPipeDelayEncrypted > 1000000000))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        /* in AC5P put constant '0'*/
        if ((egrDelayValPtr->egressPipeDelayFsuOffset < 0) || (egrDelayValPtr->egressPipeDelayFsuOffset > 1023))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    /* configure Egress TS encrypted */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelEgressPipeDelayFuturistic[tsuLocalChannel];
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 30, egrDelayValPtr->egressPipeDelayEncrypted);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* configure fsu_offset */
    if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portTsdControl1;
        fsuOffset = 1;
    }
    else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portTsdControl0;
        fsuOffset = 2;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl0;
        fsuOffset = 2;
    }
    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_AC5P_TIMESTAMP_TO_THE_FUTURE_WA_E))
    {
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, fsuOffset, 10, 0);
    }
    else
    {
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, fsuOffset, 10, egrDelayValPtr->egressPipeDelayFsuOffset);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsDelayEgressPipeDelaySet function
* @endinternal
*
* @brief   Set Egress Pipe Delay Corrections.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not supported for CPU ports).
* @param[in] egrDelayValPtr           - (pointer to) Egress Pipe Delay Corrections.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressPipeDelaySet
(
    IN  GT_U8                                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                            portNum,
    IN  CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC   *egrDelayValPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayEgressPipeDelaySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, egrDelayValPtr));

    rc = internal_cpssDxChPtpTsDelayEgressPipeDelaySet(devNum, portNum, egrDelayValPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, egrDelayValPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internla_cpssDxChPtpTsDelayEgressPipeDelayGet function
* @endinternal
*
* @brief   Get Egress Pipe Delay Corrections.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not supported for CPU ports).
*
* @param[out] egrDelayValPtr          - (pointer to) Egress Pipe Delay Corrections.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayEgressPipeDelayGet
(
    IN  GT_U8                                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                            portNum,
    OUT CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC   *egrDelayValPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 portMacNum;
    GT_U32 tsuIndex = 0;
    GT_U32 tsuLocalChannel = 0;
    GT_U32 fsuOffset;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);
    CPSS_NULL_PTR_CHECK_MAC(egrDelayValPtr);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    cpssOsMemSet(egrDelayValPtr, 0, sizeof(CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC));

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portPeerDelay;
        }
        else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, portMacNum).portPeerDelay;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portPeerDelay;
        }

        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 30,(GT_U32*) &egrDelayValPtr->egressPipeDelay);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        return prvCpssDxChReadTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        2/*fieldOffset*/, 30 /*fieldLength*/, (GT_U32*)&egrDelayValPtr->egressPipeDelay);
    }

    /* Timestamp to the Future is not supported in AC5X; Ironamn */
    if (!(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)) || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E))
         return GT_OK;

    rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(devNum, portNum, &tsuIndex, &tsuLocalChannel);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelEgressPipeDelayFuturistic[tsuLocalChannel];
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 30, (GT_U32*)&egrDelayValPtr->egressPipeDelayEncrypted);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portTsdControl1;
        fsuOffset = 1;
    }
    else if (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum) == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portTsdControl0;
        fsuOffset = 2;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, portMacNum).portTsdControl0;
        fsuOffset = 2;
    }

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fsuOffset, 10, (GT_U32*)&egrDelayValPtr->egressPipeDelayFsuOffset);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal cpssDxChPtpTsDelayEgressPipeDelayGet function
* @endinternal
*
* @brief   Get Egress Pipe Delay Corrections.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not supported for CPU ports).
*
* @param[out] egrDelayValPtr          - (pointer to) Egress Pipe Delay Corrections.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressPipeDelayGet
(
    IN  GT_U8                                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                            portNum,
    OUT CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC   *egrDelayValPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayEgressPipeDelayGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, egrDelayValPtr));

    rc = internal_cpssDxChPtpTsDelayEgressPipeDelayGet(devNum, portNum, egrDelayValPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, egrDelayValPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet function
* @endinternal
*
* @brief   Set Egress Asymmetry Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile.
*                                      (APPLICABLE RANGES: 0..8.)
* @param[in] egrAsymmetryCorr         - Egress Asymmetry Correction.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainProfile,
    IN  GT_32                   egrAsymmetryCorr
)
{
    GT_U32    fieldOffset;      /* field offset inside register     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    if (domainProfile > 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((egrAsymmetryCorr < -1000000000) || (egrAsymmetryCorr > 1000000000))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    fieldOffset = 32 + (32 * domainProfile);

    return prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        fieldOffset, 32 /*fieldLength*/, (GT_U32)egrAsymmetryCorr);
}

/**
* @internal cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet function
* @endinternal
*
* @brief   Set Egress Asymmetry Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile.
*                                      (APPLICABLE RANGES: 0..8.)
* @param[in] egrAsymmetryCorr         - Egress Asymmetry Correction.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainProfile,
    IN  GT_32                   egrAsymmetryCorr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, domainProfile, egrAsymmetryCorr));

    rc = internal_cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet(devNum, portNum, domainProfile, egrAsymmetryCorr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, domainProfile, egrAsymmetryCorr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet function
* @endinternal
*
* @brief   Get Egress Asymmetry Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile.
*                                      (APPLICABLE RANGES: 0..8.)
*
* @param[out] egrAsymmetryCorrPtr      - (pointer to) Egress Asymmetry Correction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainProfile,
    OUT GT_32                   *egrAsymmetryCorrPtr
)
{
    GT_U32    fieldOffset;      /* field offset inside register     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(egrAsymmetryCorrPtr);

    if (domainProfile > 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    fieldOffset = 32 + (32 * domainProfile);

    return prvCpssDxChReadTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        fieldOffset, 32 /*fieldLength*/, (GT_U32*)egrAsymmetryCorrPtr);
}

/**
* @internal cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet function
* @endinternal
*
* @brief   Get Egress Asymmetry Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile.
*                                      (APPLICABLE RANGES: 0..8.)
*
* @param[out] egrAsymmetryCorrPtr      - (pointer to) Egress Asymmetry Correction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainProfile,
    OUT GT_32                   *egrAsymmetryCorrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, domainProfile, egrAsymmetryCorrPtr));

    rc = internal_cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet(devNum, portNum, domainProfile, egrAsymmetryCorrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, domainProfile, egrAsymmetryCorrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet function
* @endinternal
*
* @brief   Set Ingress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] ingrCorrFldPBEnable      - Ingress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 ingrCorrFldPBEnable
)
{
    GT_U32              hwData; /* HW data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    hwData = BOOL2BIT_MAC(ingrCorrFldPBEnable);

    return prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_SOURCE_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        320 /*fieldOffset*/, 1 /*fieldLength*/, hwData);
}

/**
* @internal cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet function
* @endinternal
*
* @brief   Set Ingress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] ingrCorrFldPBEnable      - Ingress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 ingrCorrFldPBEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ingrCorrFldPBEnable));

    rc = internal_cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet(devNum, portNum, ingrCorrFldPBEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ingrCorrFldPBEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet function
* @endinternal
*
* @brief   Get Ingress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] ingrCorrFldPBEnablePtr   - (pointer to) Ingress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *ingrCorrFldPBEnablePtr
)
{
    GT_U32              hwData; /* HW data     */
    GT_STATUS           rc;     /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(ingrCorrFldPBEnablePtr);

    rc = prvCpssDxChReadTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_SOURCE_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        320 /*fieldOffset*/, 1 /*fieldLength*/, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    *ingrCorrFldPBEnablePtr = BIT2BOOL_MAC(hwData);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet function
* @endinternal
*
* @brief   Get Ingress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] ingrCorrFldPBEnablePtr   - (pointer to) Ingress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *ingrCorrFldPBEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ingrCorrFldPBEnablePtr));

    rc = internal_cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet(devNum, portNum, ingrCorrFldPBEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ingrCorrFldPBEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet function
* @endinternal
*
* @brief   Set Egress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] egrCorrFldPBEnable       - Egress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 egrCorrFldPBEnable
)
{
    GT_U32              hwData; /* HW data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    hwData = BOOL2BIT_MAC(egrCorrFldPBEnable);

    return prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        320 /*fieldOffset*/, 1 /*fieldLength*/, hwData);
}

/**
* @internal cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet function
* @endinternal
*
* @brief   Set Egress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] egrCorrFldPBEnable       - Egress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 egrCorrFldPBEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, egrCorrFldPBEnable));

    rc = internal_cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet(devNum, portNum, egrCorrFldPBEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, egrCorrFldPBEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet function
* @endinternal
*
* @brief   Get Egress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] egrCorrFldPBEnablePtr    - (pointer to) Egress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *egrCorrFldPBEnablePtr
)
{
    GT_U32              hwData; /* HW data     */
    GT_STATUS           rc;     /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(egrCorrFldPBEnablePtr);

    rc = prvCpssDxChReadTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        320 /*fieldOffset*/, 1 /*fieldLength*/, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    *egrCorrFldPBEnablePtr = BIT2BOOL_MAC(hwData);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet function
* @endinternal
*
* @brief   Get Egress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] egrCorrFldPBEnablePtr    - (pointer to) Egress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *egrCorrFldPBEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, egrCorrFldPBEnablePtr));

    rc = internal_cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet(devNum, portNum, egrCorrFldPBEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, egrCorrFldPBEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet function
* @endinternal
*
* @brief   Set Egress Time Correction TAI Select Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] egrTimeCorrTaiSelMode    - Egress Time Correction TAI Select Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT   egrTimeCorrTaiSelMode
)
{
    GT_U32              hwData; /* HW data     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    switch (egrTimeCorrTaiSelMode)
    {
        case CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E:
            hwData = 0;
            break;
        case CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E:
            hwData = 1;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        0 /*fieldOffset*/, 1 /*fieldLength*/, hwData);
}

/**
* @internal cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet function
* @endinternal
*
* @brief   Set Egress Time Correction TAI Select Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] egrTimeCorrTaiSelMode    - Egress Time Correction TAI Select Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT   egrTimeCorrTaiSelMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, egrTimeCorrTaiSelMode));

    rc = internal_cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet(devNum, portNum, egrTimeCorrTaiSelMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, egrTimeCorrTaiSelMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet function
* @endinternal
*
* @brief   Get Egress Time Correction TAI Select Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] egrTimeCorrTaiSelModePtr - (pointer to)Egress Time Correction TAI Select Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    OUT CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT   *egrTimeCorrTaiSelModePtr
)
{
    GT_U32              hwData; /* HW data     */
    GT_STATUS           rc;     /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_IRONMAN_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(egrTimeCorrTaiSelModePtr);

    rc = prvCpssDxChReadTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        0 /*fieldOffset*/, 1 /*fieldLength*/, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    *egrTimeCorrTaiSelModePtr =
        (hwData == 0)
            ? CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E
            : CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E;

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet function
* @endinternal
*
* @brief   Get Egress Time Correction TAI Select Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] egrTimeCorrTaiSelModePtr - (pointer to)Egress Time Correction TAI Select Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    OUT CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT   *egrTimeCorrTaiSelModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, egrTimeCorrTaiSelModePtr));

    rc = internal_cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet(devNum, portNum, egrTimeCorrTaiSelModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, egrTimeCorrTaiSelModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayEgressTimestampTaiSelectSet function
* @endinternal
*
* @brief   Set Egress Timestamp TAI Select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Caelum; Aldrin; AC3X; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] egrTsTaiNum              - Egress Timestamp TAI Number.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayEgressTimestampTaiSelectSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  egrTsTaiNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    if (egrTsTaiNum >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        1 /*fieldOffset*/, 1 /*fieldLength*/, egrTsTaiNum);
}

/**
* @internal cpssDxChPtpTsDelayEgressTimestampTaiSelectSet function
* @endinternal
*
* @brief   Set Egress Timestamp TAI Select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Caelum; Aldrin; AC3X; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] egrTsTaiNum              - Egress Timestamp TAI Number.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressTimestampTaiSelectSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  egrTsTaiNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayEgressTimestampTaiSelectSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, egrTsTaiNum));

    rc = internal_cpssDxChPtpTsDelayEgressTimestampTaiSelectSet(devNum, portNum, egrTsTaiNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, egrTsTaiNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDelayEgressTimestampTaiSelectGet function
* @endinternal
*
* @brief   Get Egress Timestamp TAI Select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Caelum; Aldrin; AC3X; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] egrTsTaiNumPtr           - (pointer to)Egress Timestamp TAI Number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDelayEgressTimestampTaiSelectGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *egrTsTaiNumPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(egrTsTaiNumPtr);

    return prvCpssDxChReadTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
        portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
        1 /*fieldOffset*/, 1 /*fieldLength*/, egrTsTaiNumPtr);
}

/**
* @internal cpssDxChPtpTsDelayEgressTimestampTaiSelectGet function
* @endinternal
*
* @brief   Get Egress Timestamp TAI Select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Caelum; Aldrin; AC3X; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] egrTsTaiNumPtr           - (pointer to)Egress Timestamp TAI Number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressTimestampTaiSelectGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *egrTsTaiNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDelayEgressTimestampTaiSelectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, egrTsTaiNumPtr));

    rc = internal_cpssDxChPtpTsDelayEgressTimestampTaiSelectGet(devNum, portNum, egrTsTaiNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, egrTsTaiNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsCfgTableSet function
* @endinternal
*
* @brief   Set Timestamp Configuration Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index.
*                                      (APPLICABLE RANGES: 0..127.)
* @param[in] entryPtr                 - (pointer to)Timestamp Configuration Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsCfgTableSet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         entryIndex,
    IN  CPSS_DXCH_PTP_TS_CFG_ENTRY_STC *entryPtr
)
{
    GT_U32     hwData;            /* HW data                 */
    GT_U32     hwEntry[2] = {0, 0};  /* HW formated table entry */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    if (entryIndex > 127)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    switch (entryPtr->tsMode)
    {
        case CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E:
            hwData = 0;
            break;
        case CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E:
            hwData = 1;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    hwEntry[0] |= hwData;

    if (entryPtr->tsMode == CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E)
    {
        if (entryPtr->offsetProfile >= BIT_7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        hwEntry[0] |= (entryPtr->offsetProfile << 1);

        hwEntry[0] |= (BOOL2BIT_MAC(entryPtr->OE) << 8);
    }

    if (entryPtr->tsMode == CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E)
    {
        switch (entryPtr->tsAction)
        {
            case CPSS_DXCH_PTP_TS_ACTION_NONE_E:
                hwData = 0;
                break;
            case CPSS_DXCH_PTP_TS_ACTION_DROP_E:
                hwData = 2;
                break;
            case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E:
                hwData = 3;
                break;
            case CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E:
                hwData = 4;
                break;
            case CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E:
                hwData = 5;
                break;
            case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E:
                hwData = 6;
                break;
            case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E:
                hwData = 7;
                break;
            case CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_TIME_E:
                hwData = 8;
                break;
            case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E:
                hwData = 9;
                break;
            case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E:
                hwData = 10;
                break;
            case CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E:
                hwData = 11;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        hwEntry[0] |= (hwData << 1);

        switch (entryPtr->packetFormat)
        {
            case CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E:
                hwData = 0;
                break;
            case CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E:
                hwData = 1;
                break;
            case CPSS_DXCH_PTP_TS_PACKET_TYPE_Y1731_E:
                hwData = 2;
                break;
            case CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TS_E:
                hwData = 3;
                break;
            case CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_RX_E:
                hwData = 4;
                break;
            case CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TX_E:
                hwData = 5;
                break;
            case CPSS_DXCH_PTP_TS_PACKET_TYPE_WAMP_E:
                hwData = 6;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        hwEntry[0] |= (hwData << 5);

        switch (entryPtr->ptpTransport)
        {
            case CPSS_DXCH_PTP_TRANSPORT_TYPE_ETHERNET_E:
                hwData = 0;
                break;
            case CPSS_DXCH_PTP_TRANSPORT_TYPE_UDP_IPV4_E:
                hwData = 1;
                break;
            case CPSS_DXCH_PTP_TRANSPORT_TYPE_UDP_IPV6_E:
                hwData = 2;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        hwEntry[0] |= (hwData << 8);
    }

    if (entryPtr->ptpMessageType >= BIT_4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (entryPtr->domain > 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        if (entryPtr->offset >= BIT_8)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        if (entryPtr->tsMode == CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E)
        {
            hwEntry[0] |= (entryPtr->offset << 10);

            hwEntry[0] |= (entryPtr->ptpMessageType << 18);

            hwEntry[0] |= (entryPtr->domain << 22);
        }


        hwEntry[0] |= (BOOL2BIT_MAC(entryPtr->ingrLinkDelayEnable) << 25);

        hwEntry[0] |= (BOOL2BIT_MAC(entryPtr->packetDispatchingEnable) << 26);

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* LSB bit of offset2 should not be set*/
            if((entryPtr->offset2 & 1) || (entryPtr->offset2 >= BIT_8))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /**
             * The offset2 is an 7-bit value,
             * [4:0] 5 bits in 1st register
             * [6:5] 2 bits in 2nd register
             */
            U32_SET_FIELD_IN_ENTRY_MAC(hwEntry, 27, 7, (entryPtr->offset2>>1));
        }
    }
    else
    {
        if (entryPtr->offset >= BIT_7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        if (entryPtr->tsMode == CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E)
        {
            hwEntry[0] |= (entryPtr->offset << 10);

            hwEntry[0] |= (entryPtr->ptpMessageType << 17);

            hwEntry[0] |= (entryPtr->domain << 21);
        }

        hwEntry[0] |= (BOOL2BIT_MAC(entryPtr->ingrLinkDelayEnable) << 24);

        hwEntry[0] |= (BOOL2BIT_MAC(entryPtr->packetDispatchingEnable) << 25);
    }

    return prvCpssDxChWriteTableEntry(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_TIMESTAMP_CFG_E,
        entryIndex, hwEntry);
}

/**
* @internal cpssDxChPtpTsCfgTableSet function
* @endinternal
*
* @brief   Set Timestamp Configuration Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index.
*                                      (APPLICABLE RANGES: 0..127.)
* @param[in] entryPtr                 - (pointer to)Timestamp Configuration Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsCfgTableSet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         entryIndex,
    IN  CPSS_DXCH_PTP_TS_CFG_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsCfgTableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, entryPtr));

    rc = internal_cpssDxChPtpTsCfgTableSet(devNum, entryIndex, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsCfgTableGet function
* @endinternal
*
* @brief   Get Timestamp Configuration Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index.
*                                      (APPLICABLE RANGES: 0..127.)
*
* @param[out] entryPtr                 - (pointer to)Timestamp Configuration Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsCfgTableGet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         entryIndex,
    OUT CPSS_DXCH_PTP_TS_CFG_ENTRY_STC *entryPtr
)
{
    GT_U32     hwEntry[2] = {0, 0};  /* HW formated table entry */
    GT_U32     hwData;   /* HW data                 */
    GT_STATUS  rc;       /* return code             */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    cpssOsMemSet(entryPtr, 0, sizeof(*entryPtr));

    if (entryIndex > 127)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChReadTableEntry(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_TIMESTAMP_CFG_E,
        entryIndex, hwEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    hwData = hwEntry[0] & 1;

    switch (hwData)
    {
        case 0:
            entryPtr->tsMode = CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E;
            break;
        case 1:
            entryPtr->tsMode = CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* never occurs */
    }

    if (entryPtr->tsMode == CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E)
    {
        entryPtr->offsetProfile = ((hwEntry[0] >> 1) & 0x7F);
        hwData                  = ((hwEntry[0] >> 8) & 1);
        entryPtr->OE            = BIT2BOOL_MAC(hwData);
    }

    if (entryPtr->tsMode == CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E)
    {
        hwData = ((hwEntry[0] >> 1) & 0xF);

        switch (hwData)
        {
            default:
            case 0:
                entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_NONE_E;
                break;
            case 2:
                entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_DROP_E;
                break;
            case 3:
                entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E;
                break;
            case 4:
                entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E;
                break;
            case 5:
                entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E;
                break;
            case 6:
                entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E;
                break;
            case 7:
                entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E;
                break;
            case 8:
                entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_TIME_E;
                break;
            case 9:
                entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E;
                break;
            case 10:
                entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E;
                break;
            case 11:
                entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E;
                break;
        }

        hwData = ((hwEntry[0] >> 5) & 0x7);

        switch (hwData)
        {
            case 0:
                entryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E;
                break;
            case 1:
                entryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E;
                break;
            case 2:
                entryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_Y1731_E;
                break;
            case 3:
                entryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TS_E;
                break;
            case 4:
                entryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_RX_E;
                break;
            case 5:
                entryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TX_E;
                break;
            case 6:
                entryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_WAMP_E;
                break;
            case 7:
                entryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_RESERVED_E;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG); /* on reseved value 7 */
        }

        hwData = ((hwEntry[0] >> 8) & 0x3);

        switch (hwData)
        {
            case 0:
                entryPtr->ptpTransport = CPSS_DXCH_PTP_TRANSPORT_TYPE_ETHERNET_E;
                break;
            case 1:
                entryPtr->ptpTransport = CPSS_DXCH_PTP_TRANSPORT_TYPE_UDP_IPV4_E;
                break;
            case 2:
                entryPtr->ptpTransport = CPSS_DXCH_PTP_TRANSPORT_TYPE_UDP_IPV6_E;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG); /* on reserved value 3 */
        }
    }
    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        if (entryPtr->tsMode == CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E)
        {
            entryPtr->offset                  = ((hwEntry[0] >> 10) & 0xFF);

            entryPtr->ptpMessageType          = ((hwEntry[0] >> 18) & 0xF);

            entryPtr->domain                  = ((hwEntry[0] >> 22) & 0x7);
        }


        hwData                            = ((hwEntry[0] >> 25) & 1);

        entryPtr->ingrLinkDelayEnable     = BIT2BOOL_MAC(hwData);

        hwData                            = ((hwEntry[0] >> 26) & 1);

        entryPtr->packetDispatchingEnable = BIT2BOOL_MAC(hwData);

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /**
             * The offset2 is an 7-bit value,
             */
            U32_GET_FIELD_IN_ENTRY_MAC(hwEntry, 27, 7, hwData);
            entryPtr->offset2 = (hwData<<1);
        }
    }
    else
    {
        if (entryPtr->tsMode == CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E)
        {
            entryPtr->offset                  = ((hwEntry[0] >> 10) & 0x7F);

            entryPtr->ptpMessageType          = ((hwEntry[0] >> 17) & 0xF);

            entryPtr->domain                  = ((hwEntry[0] >> 21) & 0x7);
        }

        hwData                            = ((hwEntry[0] >> 24) & 1);

        entryPtr->ingrLinkDelayEnable     = BIT2BOOL_MAC(hwData);

        hwData                            = ((hwEntry[0] >> 25) & 1);

        entryPtr->packetDispatchingEnable = BIT2BOOL_MAC(hwData);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsCfgTableGet function
* @endinternal
*
* @brief   Get Timestamp Configuration Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index.
*                                      (APPLICABLE RANGES: 0..127.)
*
* @param[out] entryPtr                 - (pointer to)Timestamp Configuration Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsCfgTableGet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         entryIndex,
    OUT CPSS_DXCH_PTP_TS_CFG_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsCfgTableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, entryPtr));

    rc = internal_cpssDxChPtpTsCfgTableGet(devNum, entryIndex, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsLocalActionTableSet function
* @endinternal
*
* @brief   Set Timestamp Local Action Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile.
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] entryPtr                 - (pointer to)Timestamp Local Action Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsLocalActionTableSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainProfile,
    IN  GT_U32                                  messageType,
    IN  CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC *entryPtr
)
{
    GT_U32     hwEntry;     /* HW formated table entry */
    GT_U32     hwData;      /* HW data                 */
    GT_U32     entryIndex;  /* table entry index       */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    if (domainProfile > 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (messageType > 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    hwEntry = 0;
    entryIndex = (portNum * 128) + (messageType * 8) + domainProfile;

    switch (entryPtr->tsAction)
    {
        case CPSS_DXCH_PTP_TS_ACTION_NONE_E:
            hwData = 0;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_DROP_E:
            hwData = 2;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E:
            hwData = 3;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E:
            hwData = 4;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E:
            hwData = 5;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E:
            hwData = 6;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E:
            hwData = 7;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E:
            hwData = 10;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    hwEntry |= (hwData << 1);

    hwData   = BOOL2BIT_MAC(entryPtr->ingrLinkDelayEnable);

    hwEntry |= hwData;

    hwData   = BOOL2BIT_MAC(entryPtr->packetDispatchingEnable);

    hwEntry |= (hwData << 5);

    return prvCpssDxChWriteTableEntry(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_LOCAL_ACTION_E,
        entryIndex, &hwEntry);
}

/**
* @internal cpssDxChPtpTsLocalActionTableSet function
* @endinternal
*
* @brief   Set Timestamp Local Action Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile.
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] entryPtr                 - (pointer to)Timestamp Local Action Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsLocalActionTableSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainProfile,
    IN  GT_U32                                  messageType,
    IN  CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsLocalActionTableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, domainProfile, messageType, entryPtr));

    rc = internal_cpssDxChPtpTsLocalActionTableSet(devNum, portNum, domainProfile, messageType, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, domainProfile, messageType, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsLocalActionTableGet function
* @endinternal
*
* @brief   Get Timestamp Local Action Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile.
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] entryPtr                 - (pointer to)Timestamp Local Action Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsLocalActionTableGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainProfile,
    IN  GT_U32                                  messageType,
    OUT CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC *entryPtr
)
{
    GT_U32     hwEntry;     /* HW formated table entry */
    GT_U32     hwData;      /* HW data                 */
    GT_STATUS  rc;          /* return code             */
    GT_U32     entryIndex;  /* table entry index       */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    cpssOsMemSet(entryPtr, 0, sizeof(*entryPtr));

    if (domainProfile > 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (messageType > 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    entryIndex = (portNum * 128) + (messageType * 8) + domainProfile;

    rc = prvCpssDxChReadTableEntry(
        devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_LOCAL_ACTION_E,
        entryIndex, &hwEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    hwData = ((hwEntry >> 1) & 0xF);

    switch (hwData)
    {
        case 0:
            entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_NONE_E;
            break;
        case 2:
            entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_DROP_E;
            break;
        case 3:
            entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E;
            break;
        case 4:
            entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E;
            break;
        case 5:
            entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E;
            break;
        case 6:
            entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E;
            break;
        case 7:
            entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E;
            break;
        case 10:
            entryPtr->tsAction = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    hwData = (hwEntry & 1);
    entryPtr->ingrLinkDelayEnable = BIT2BOOL_MAC(hwData);

    hwData = ((hwEntry >> 5) & 1);
    entryPtr->packetDispatchingEnable = BIT2BOOL_MAC(hwData);

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsLocalActionTableGet function
* @endinternal
*
* @brief   Get Timestamp Local Action Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile.
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] entryPtr                 - (pointer to)Timestamp Local Action Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsLocalActionTableGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainProfile,
    IN  GT_U32                                  messageType,
    OUT CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsLocalActionTableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, domainProfile, messageType, entryPtr));

    rc = internal_cpssDxChPtpTsLocalActionTableGet(devNum, portNum, domainProfile, messageType, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, domainProfile, messageType, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsUdpChecksumUpdateModeSet function
* @endinternal
*
* @brief   Set Timestamp UDP Checksum Update Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] udpCsUpdModePtr          - (pointer to)Timestamp UDP Checksum Update Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsUdpChecksumUpdateModeSet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC *udpCsUpdModePtr
)
{
    GT_U32    regAddr;          /* register address             */
    GT_U32    hwData;           /* HW data                      */
    GT_U32    hwValue;          /* HW value                     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(udpCsUpdModePtr);

    hwData = 0;

    PRV_CPSS_CONVERT_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_TO_HW_VAL_MAC(
        hwValue, udpCsUpdModePtr->ptpIpv4Mode);
    hwData |= hwValue;

    PRV_CPSS_CONVERT_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_TO_HW_VAL_MAC(
        hwValue, udpCsUpdModePtr->ptpIpv6Mode);
    hwData |= (hwValue << 2);

    PRV_CPSS_CONVERT_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_TO_HW_VAL_MAC(
        hwValue, udpCsUpdModePtr->ntpIpv4Mode);
    hwData |= (hwValue << 4);

    PRV_CPSS_CONVERT_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_TO_HW_VAL_MAC(
        hwValue, udpCsUpdModePtr->ntpIpv6Mode);
    hwData |= (hwValue << 6);

    PRV_CPSS_CONVERT_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_TO_HW_VAL_MAC(
        hwValue, udpCsUpdModePtr->wampIpv4Mode);
    hwData |= (hwValue << 8);

    PRV_CPSS_CONVERT_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_TO_HW_VAL_MAC(
        hwValue, udpCsUpdModePtr->wampIpv6Mode);
    hwData |= (hwValue << 10);

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).timestampingUDPChecksumMode;

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 12, hwData);
}

/**
* @internal cpssDxChPtpTsUdpChecksumUpdateModeSet function
* @endinternal
*
* @brief   Set Timestamp UDP Checksum Update Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] udpCsUpdModePtr          - (pointer to)Timestamp UDP Checksum Update Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsUdpChecksumUpdateModeSet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC *udpCsUpdModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsUdpChecksumUpdateModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, udpCsUpdModePtr));

    rc = internal_cpssDxChPtpTsUdpChecksumUpdateModeSet(devNum, udpCsUpdModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, udpCsUpdModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsUdpChecksumUpdateModeGet function
* @endinternal
*
* @brief   Get Timestamp UDP Checksum Update Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] udpCsUpdModePtr          - (pointer to)Timestamp UDP Checksum Update Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsUdpChecksumUpdateModeGet
(
    IN  GT_U8                                         devNum,
    OUT CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC *udpCsUpdModePtr
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    regAddr;          /* register address             */
    GT_U32    hwData;           /* HW data                      */
    GT_U32    hwValue;          /* HW value                     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(udpCsUpdModePtr);

    cpssOsMemSet(udpCsUpdModePtr, 0, sizeof(*udpCsUpdModePtr));

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).timestampingUDPChecksumMode;

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    hwValue = (hwData & 3);
    PRV_CPSS_CONVERT_HW_VAL_TO_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_MAC(
        hwValue, udpCsUpdModePtr->ptpIpv4Mode);

    hwValue = ((hwData >> 2) & 3);
    PRV_CPSS_CONVERT_HW_VAL_TO_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_MAC(
        hwValue, udpCsUpdModePtr->ptpIpv6Mode);

    hwValue = ((hwData >> 4) & 3);
    PRV_CPSS_CONVERT_HW_VAL_TO_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_MAC(
        hwValue, udpCsUpdModePtr->ntpIpv4Mode);

    hwValue = ((hwData >> 6) & 3);
    PRV_CPSS_CONVERT_HW_VAL_TO_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_MAC(
        hwValue, udpCsUpdModePtr->ntpIpv6Mode);

    hwValue = ((hwData >> 8) & 3);
    PRV_CPSS_CONVERT_HW_VAL_TO_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_MAC(
        hwValue, udpCsUpdModePtr->wampIpv4Mode);

    hwValue = ((hwData >> 10) & 3);
    PRV_CPSS_CONVERT_HW_VAL_TO_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_MAC(
        hwValue, udpCsUpdModePtr->wampIpv6Mode);

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsUdpChecksumUpdateModeGet function
* @endinternal
*
* @brief   Get Timestamp UDP Checksum Update Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] udpCsUpdModePtr          - (pointer to)Timestamp UDP Checksum Update Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsUdpChecksumUpdateModeGet
(
    IN  GT_U8                                         devNum,
    OUT CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC *udpCsUpdModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsUdpChecksumUpdateModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, udpCsUpdModePtr));

    rc = internal_cpssDxChPtpTsUdpChecksumUpdateModeGet(devNum, udpCsUpdModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, udpCsUpdModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpOverEthernetEnableSet function
* @endinternal
*
* @brief   Set enable PTP over Ethernet packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpOverEthernetEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
{
    GT_STATUS rc;               /* return code                      */
    GT_U32    regAddr;          /* register address                 */
    GT_U32    hwValue;          /* HW value                         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPConfigs;
    hwValue = BOOL2BIT_MAC(enable);

    rc = prvCpssHwPpSetRegField(
        devNum, regAddr, 0, 1, hwValue);

    return rc;
}

/**
* @internal cpssDxChPtpOverEthernetEnableSet function
* @endinternal
*
* @brief   Set enable PTP over Ethernet packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpOverEthernetEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpOverEthernetEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPtpOverEthernetEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpOverEthernetEnableGet function
* @endinternal
*
* @brief   Get enable PTP over Ethernet packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpOverEthernetEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;               /* return code                      */
    GT_U32    regAddr;          /* register address                 */
    GT_U32    hwValue;          /* HW value                         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPConfigs;

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, 0, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(hwValue);

    return GT_OK;
}

/**
* @internal cpssDxChPtpOverEthernetEnableGet function
* @endinternal
*
* @brief   Get enable PTP over Ethernet packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpOverEthernetEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpOverEthernetEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPtpOverEthernetEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpOverUdpEnableSet function
* @endinternal
*
* @brief   Set enable PTP over UDP packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpOverUdpEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
{
    GT_STATUS rc;               /* return code                      */
    GT_U32    regAddr;          /* register address                 */
    GT_U32    hwValue;          /* HW value                         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPConfigs;
    hwValue = BOOL2BIT_MAC(enable);

    rc = prvCpssHwPpSetRegField(
        devNum, regAddr, 1, 1, hwValue);

    return rc;
}

/**
* @internal cpssDxChPtpOverUdpEnableSet function
* @endinternal
*
* @brief   Set enable PTP over UDP packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpOverUdpEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpOverUdpEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPtpOverUdpEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpOverUdpEnableGet function
* @endinternal
*
* @brief   Get enable PTP over UDP packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpOverUdpEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;               /* return code                      */
    GT_U32    regAddr;          /* register address                 */
    GT_U32    hwValue;          /* HW value                         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPConfigs;

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, 1, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(hwValue);

    return GT_OK;
}

/**
* @internal cpssDxChPtpOverUdpEnableGet function
* @endinternal
*
* @brief   Get enable PTP over UDP packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpOverUdpEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpOverUdpEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPtpOverUdpEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpDomainModeSet function
* @endinternal
*
* @brief   Set PTP domain mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainMode               - domain mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpDomainModeSet
(
    IN  GT_U8                                  devNum,
    IN  GT_U32                                 domainIndex,
    IN  CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT  domainMode
)
{
    GT_STATUS rc;               /* return code                      */
    GT_U32    regAddr;          /* register address                 */
    GT_U32    hwValue;          /* HW value                         */
    GT_U32    offset;           /* field offset                     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    if (domainIndex > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (domainMode)
    {
        case CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_DISABLE_E:
            hwValue = 0;
            break;
        case CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E:
            hwValue = 1;
            break;
        case CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E:
            hwValue = 2;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPConfigs;

    offset = 2 + (domainIndex * 2);

    rc = prvCpssHwPpSetRegField(
        devNum, regAddr, offset, 2, hwValue);

    return rc;
}

/**
* @internal cpssDxChPtpDomainModeSet function
* @endinternal
*
* @brief   Set PTP domain mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainMode               - domain mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpDomainModeSet
(
    IN  GT_U8                                  devNum,
    IN  GT_U32                                 domainIndex,
    IN  CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT  domainMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpDomainModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, domainIndex, domainMode));

    rc = internal_cpssDxChPtpDomainModeSet(devNum, domainIndex, domainMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, domainIndex, domainMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpDomainModeGet function
* @endinternal
*
* @brief   Get PTP domain mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainModePtr            - (pointer to) domain mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpDomainModeGet
(
    IN  GT_U8                                  devNum,
    IN  GT_U32                                 domainIndex,
    OUT CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT  *domainModePtr
)
{
    GT_STATUS rc;               /* return code                      */
    GT_U32    regAddr;          /* register address                 */
    GT_U32    hwValue;          /* HW value                         */
    GT_U32    offset;           /* field offset                     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(domainModePtr);

    if (domainIndex > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPConfigs;

    offset = 2 + (domainIndex * 2);

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, offset, 2, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (hwValue)
    {
        default:
        case 0:
            *domainModePtr = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_DISABLE_E;
            break;
        case 1:
            *domainModePtr = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E;
            break;
        case 2:
            *domainModePtr = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
            break;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpDomainModeGet function
* @endinternal
*
* @brief   Get PTP domain mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainModePtr            - (pointer to) domain mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpDomainModeGet
(
    IN  GT_U8                                  devNum,
    IN  GT_U32                                 domainIndex,
    OUT CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT  *domainModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpDomainModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, domainIndex, domainModePtr));

    rc = internal_cpssDxChPtpDomainModeGet(devNum, domainIndex, domainModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, domainIndex, domainModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpDomainV1IdSet function
* @endinternal
*
* @brief   Set PTP V1 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainIdArr[4]           - domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V1 Header contains 128-bit domain Id.
*       4 domain Id values mapped to domainIndex 0-3
*       all other domain Id values mapped to default domain
*
*/
static GT_STATUS internal_cpssDxChPtpDomainV1IdSet
(
    IN  GT_U8        devNum,
    IN  GT_U32       domainIndex,
    IN  GT_U32       domainIdArr[4]
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    regAddr;          /* register address             */
    GT_U32    i;                /* loop index                   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(domainIdArr);

    if (domainIndex > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (i = 0; (i < 4); i++)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.
                _1588V1DomainDomainNumber[i][domainIndex];
        rc = prvCpssHwPpWriteRegister(
            devNum, regAddr, domainIdArr[i]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpDomainV1IdSet function
* @endinternal
*
* @brief   Set PTP V1 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainIdArr[4]           - domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V1 Header contains 128-bit domain Id.
*       4 domain Id values mapped to domainIndex 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS cpssDxChPtpDomainV1IdSet
(
    IN  GT_U8        devNum,
    IN  GT_U32       domainIndex,
    IN  GT_U32       domainIdArr[4]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpDomainV1IdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, domainIndex, domainIdArr));

    rc = internal_cpssDxChPtpDomainV1IdSet(devNum, domainIndex, domainIdArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, domainIndex, domainIdArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpDomainV1IdGet function
* @endinternal
*
* @brief   Get PTP V1 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainIdArr[4]           - domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V1 Header contains 128-bit domain Id.
*       4 domain Id values mapped to domainIndex 0-3
*       all other domain Id values mapped to default domain
*
*/
static GT_STATUS internal_cpssDxChPtpDomainV1IdGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       domainIndex,
    OUT GT_U32       domainIdArr[4]
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    regAddr;          /* register address             */
    GT_U32    i;                /* loop index                   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(domainIdArr);

    if (domainIndex > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (i = 0; (i < 4); i++)
    {
        regAddr =
            PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.
                _1588V1DomainDomainNumber[i][domainIndex];
        rc = prvCpssHwPpReadRegister(
            devNum, regAddr, &(domainIdArr[i]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpDomainV1IdGet function
* @endinternal
*
* @brief   Get PTP V1 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainIdArr[4]           - domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V1 Header contains 128-bit domain Id.
*       4 domain Id values mapped to domainIndex 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS cpssDxChPtpDomainV1IdGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       domainIndex,
    OUT GT_U32       domainIdArr[4]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpDomainV1IdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, domainIndex, domainIdArr));

    rc = internal_cpssDxChPtpDomainV1IdGet(devNum, domainIndex, domainIdArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, domainIndex, domainIdArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpDomainV2IdSet function
* @endinternal
*
* @brief   Set PTP V2 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainId                 - domain Id
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V2 Header contains 8-bit domain Id.
*       4 domain Id values mapped to domainIndex 0-3
*       all other domain Id values mapped to default domain
*
*/
static GT_STATUS internal_cpssDxChPtpDomainV2IdSet
(
    IN  GT_U8        devNum,
    IN  GT_U32       domainIndex,
    IN  GT_U32       domainId
)
{
    GT_U32    regAddr;          /* register address             */
    GT_U32    offset;           /* field offset                 */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    if (domainIndex > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (domainId > 0xFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP._1588V2DomainDomainNumber;
    offset = (domainIndex * 8);

    return prvCpssHwPpSetRegField(
        devNum, regAddr, offset, 8, domainId);
}

/**
* @internal cpssDxChPtpDomainV2IdSet function
* @endinternal
*
* @brief   Set PTP V2 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainId                 - domain Id
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V2 Header contains 8-bit domain Id.
*       4 domain Id values mapped to domainIndex 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS cpssDxChPtpDomainV2IdSet
(
    IN  GT_U8        devNum,
    IN  GT_U32       domainIndex,
    IN  GT_U32       domainId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpDomainV2IdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, domainIndex, domainId));

    rc = internal_cpssDxChPtpDomainV2IdSet(devNum, domainIndex, domainId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, domainIndex, domainId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpDomainV2IdGet function
* @endinternal
*
* @brief   Get PTP V2 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainIdPtr              - (pointer to) domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V2 Header contains 8-bit domain Id.
*       4 domain Id values mapped to domainIndex 0-3
*       all other domain Id values mapped to default domain
*
*/
static GT_STATUS internal_cpssDxChPtpDomainV2IdGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       domainIndex,
    OUT GT_U32       *domainIdPtr
)
{
    GT_U32    regAddr;          /* register address             */
    GT_U32    offset;           /* field offset                 */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(domainIdPtr);

    if (domainIndex > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP._1588V2DomainDomainNumber;
    offset = (domainIndex * 8);

    return prvCpssHwPpGetRegField(
        devNum, regAddr, offset, 8, domainIdPtr);
}

/**
* @internal cpssDxChPtpDomainV2IdGet function
* @endinternal
*
* @brief   Get PTP V2 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainIdPtr              - (pointer to) domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V2 Header contains 8-bit domain Id.
*       4 domain Id values mapped to domainIndex 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS cpssDxChPtpDomainV2IdGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       domainIndex,
    OUT GT_U32       *domainIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpDomainV2IdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, domainIndex, domainIdPtr));

    rc = internal_cpssDxChPtpDomainV2IdGet(devNum, domainIndex, domainIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, domainIndex, domainIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpEgressDomainTableSet function
* @endinternal
*
* @brief   Set Egress Domain Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] entryPtr                 - (pointer to) Domain Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpEgressDomainTableSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainIndex,
    IN  CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   *entryPtr
)
{
    GT_U32     hwEntryArr[2];  /* HW formated table entry */
    GT_U32     hwData;         /* HW data                 */
    GT_U32     entryIndex;     /* table entry index       */
    GT_U32     portFactor;     /* port factor */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    if (domainIndex > 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* no physical modes in sip6.10 and above */
        portFactor = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    }
    else
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        portFactor = 1024;
    }
    else if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* BC3,Aldrin2 */
        portFactor = 512;
    }
    else
    {
        portFactor = (1+PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PHY_PORT_MAC(devNum));/*128/256*/
    }
    entryIndex = (domainIndex * portFactor) + portNum;
    hwEntryArr[0] = 0;
    hwEntryArr[1] = 0;

    hwData   = BOOL2BIT_MAC(entryPtr->ptpOverEhernetTsEnable);
    hwEntryArr[0] |= hwData;

    hwData   = BOOL2BIT_MAC(entryPtr->ptpOverUdpIpv4TsEnable);
    hwEntryArr[0] |= (hwData << 1);

    hwData   = BOOL2BIT_MAC(entryPtr->ptpOverUdpIpv6TsEnable);
    hwEntryArr[0] |= (hwData << 2);

    if (entryPtr->messageTypeTsEnableBmp > BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    hwEntryArr[0] |= (entryPtr->messageTypeTsEnableBmp << 3);

    if (entryPtr->transportSpecificTsEnableBmp > BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    hwEntryArr[0] |= ((entryPtr->transportSpecificTsEnableBmp & 0x1FFF) << 19);
    hwEntryArr[1] |= (entryPtr->transportSpecificTsEnableBmp >> 13);

    return prvCpssDxChWriteTableEntry(
        devNum, CPSS_DXCH_SIP5_TABLE_HA_PTP_DOMAIN_E,
        entryIndex, hwEntryArr);
}

/**
* @internal cpssDxChPtpEgressDomainTableSet function
* @endinternal
*
* @brief   Set Egress Domain Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] entryPtr                 - (pointer to) Domain Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpEgressDomainTableSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainIndex,
    IN  CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpEgressDomainTableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, domainIndex, entryPtr));

    rc = internal_cpssDxChPtpEgressDomainTableSet(devNum, portNum, domainIndex, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, domainIndex, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpEgressDomainTableGet function
* @endinternal
*
* @brief   Get Egress Domain Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..4.)
*
* @param[out] entryPtr                 - (pointer to) Egress Domain Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpEgressDomainTableGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainIndex,
    OUT CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   *entryPtr
)
{
    GT_STATUS  rc;             /* return code             */
    GT_U32     hwEntryArr[2];  /* HW formated table entry */
    GT_U32     hwData;         /* HW data                 */
    GT_U32     entryIndex;     /* table entry index       */
    GT_U32     portFactor;     /* port factor */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    cpssOsMemSet(entryPtr, 0, sizeof(*entryPtr));

    if (domainIndex > 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* no physical modes in sip6.10 and above */
        portFactor = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    }
    else
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        portFactor = 1024;
    }
    else if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* BC3,Aldrin2 */
        portFactor = 512;
    }
    else
    {
        portFactor = (1+PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PHY_PORT_MAC(devNum));/*128/256*/
    }

    entryIndex = (domainIndex * portFactor) + portNum;

    rc = prvCpssDxChReadTableEntry(
        devNum, CPSS_DXCH_SIP5_TABLE_HA_PTP_DOMAIN_E,
        entryIndex, hwEntryArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    hwData = (hwEntryArr[0] & 1);
    entryPtr->ptpOverEhernetTsEnable = BIT2BOOL_MAC(hwData);

    hwData = ((hwEntryArr[0] >> 1) & 1);
    entryPtr->ptpOverUdpIpv4TsEnable = BIT2BOOL_MAC(hwData);

    hwData = ((hwEntryArr[0] >> 2) & 1);
    entryPtr->ptpOverUdpIpv6TsEnable = BIT2BOOL_MAC(hwData);

    entryPtr->messageTypeTsEnableBmp = ((hwEntryArr[0] >> 3) & 0xFFFF);

    entryPtr->transportSpecificTsEnableBmp =
        ((hwEntryArr[0] >> 19) & 0x1FFF) | ((hwEntryArr[1] & 7) << 13);

    return GT_OK;
}

/**
* @internal cpssDxChPtpEgressDomainTableGet function
* @endinternal
*
* @brief   Get Egress Domain Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..4.)
*
* @param[out] entryPtr                 - (pointer to) Egress Domain Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpEgressDomainTableGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainIndex,
    OUT CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpEgressDomainTableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, domainIndex, entryPtr));

    rc = internal_cpssDxChPtpEgressDomainTableGet(devNum, portNum, domainIndex, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, domainIndex, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpIngressExceptionCfgSet function
* @endinternal
*
* @brief   Set PTP packet Ingress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] ingrExceptionCfgPtr      - (pointer to) PTP packet Ingress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpIngressExceptionCfgSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC   *ingrExceptionCfgPtr
)
{
    GT_U32                                  regAddr;    /* register address */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode; /* DSA code */
    GT_STATUS                               rc;         /* return code */
    GT_U32                                  hwData;     /* HW data     */
    GT_U32                                  hwMask;     /* HW mask     */
    GT_U32                                  hwCmd;      /* HW command  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(ingrExceptionCfgPtr);

    hwMask = 0;
    hwData = 0;
    dsaCpuCode = 0;

    hwMask |= 1;
    if (ingrExceptionCfgPtr->ptpExceptionCommandEnable != GT_FALSE)
    {
        PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(
            hwCmd, ingrExceptionCfgPtr->ptpExceptionCommand);
        hwData |= (hwCmd << 1) | 1;
        hwMask |= (7 << 1);

        rc = prvCpssDxChNetIfCpuToDsaCode(
            ingrExceptionCfgPtr->ptpExceptionCpuCode, &dsaCpuCode);
        if (rc != GT_OK)
        {
            if ((ingrExceptionCfgPtr->ptpExceptionCommand == CPSS_PACKET_CMD_TRAP_TO_CPU_E)
                || (ingrExceptionCfgPtr->ptpExceptionCommand == CPSS_PACKET_CMD_MIRROR_TO_CPU_E))
            {
                return rc;
            }
        }
        else
        {
            hwData |= (dsaCpuCode << 4);
            hwMask |= (0xFF << 4);
        }
    }

    hwData |= (BOOL2BIT_MAC(ingrExceptionCfgPtr->ptpVersionCheckEnable) << 22);
    hwMask |= (1 << 22);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPExceptionsAndCPUCodeConfig;

    return prvCpssHwPpWriteRegBitMask(devNum, regAddr, hwMask, hwData);
}

/**
* @internal cpssDxChPtpIngressExceptionCfgSet function
* @endinternal
*
* @brief   Set PTP packet Ingress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] ingrExceptionCfgPtr      - (pointer to) PTP packet Ingress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpIngressExceptionCfgSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC   *ingrExceptionCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpIngressExceptionCfgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ingrExceptionCfgPtr));

    rc = internal_cpssDxChPtpIngressExceptionCfgSet(devNum, ingrExceptionCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ingrExceptionCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpIngressExceptionCfgGet function
* @endinternal
*
* @brief   Get PTP packet Ingress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] ingrExceptionCfgPtr      - (pointer to) PTP packet Ingress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpIngressExceptionCfgGet
(
    IN  GT_U8                                     devNum,
    OUT CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC   *ingrExceptionCfgPtr
)
{
    GT_U32                                  regAddr;    /* register address */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode; /* DSA code */
    GT_STATUS                               rc;         /* return code */
    GT_U32                                  hwData;     /* HW data     */
    GT_U32                                  hwMask;     /* HW mask     */
    GT_U32                                  hwCmd;      /* HW command  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(ingrExceptionCfgPtr);

    cpssOsMemSet(ingrExceptionCfgPtr, 0, sizeof(*ingrExceptionCfgPtr));
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPExceptionsAndCPUCodeConfig;
    hwMask = 0x00400FFF;

    rc = prvCpssHwPpReadRegBitMask(devNum, regAddr, hwMask, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    ingrExceptionCfgPtr->ptpExceptionCommandEnable = BIT2BOOL_MAC((hwData & 1));
    ingrExceptionCfgPtr->ptpVersionCheckEnable = BIT2BOOL_MAC(((hwData >> 22) & 1));

    if (ingrExceptionCfgPtr->ptpVersionCheckEnable != GT_FALSE)
    {
        PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(
            ingrExceptionCfgPtr->ptpExceptionCommand, ((hwData >> 1) & 7));

        hwCmd = (hwData >> 4) & 0xFF;
        dsaCpuCode = (PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)(hwCmd);

        rc = prvCpssDxChNetIfDsaToCpuCode(
            dsaCpuCode, &(ingrExceptionCfgPtr->ptpExceptionCpuCode));
        if (rc != GT_OK)
        {
            if ((ingrExceptionCfgPtr->ptpExceptionCommand == CPSS_PACKET_CMD_TRAP_TO_CPU_E)
                || (ingrExceptionCfgPtr->ptpExceptionCommand == CPSS_PACKET_CMD_MIRROR_TO_CPU_E))
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpIngressExceptionCfgGet function
* @endinternal
*
* @brief   Get PTP packet Ingress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] ingrExceptionCfgPtr      - (pointer to) PTP packet Ingress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpIngressExceptionCfgGet
(
    IN  GT_U8                                     devNum,
    OUT CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC   *ingrExceptionCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpIngressExceptionCfgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ingrExceptionCfgPtr));

    rc = internal_cpssDxChPtpIngressExceptionCfgGet(devNum, ingrExceptionCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ingrExceptionCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpIngressPacketCheckingModeSet function
* @endinternal
*
* @brief   Set PTP packet Ingress Checking Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] checkingMode             - PTP packet ingress checking mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpIngressPacketCheckingModeSet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   domainIndex,
    IN  GT_U32                                   messageType,
    IN  CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT  checkingMode
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    regAddr;          /* register address             */
    GT_U32    hwData;           /* HW data                      */
    GT_U32    offset;           /* field offset                 */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    if (domainIndex > 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (messageType > 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr =
        PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.
            PTPExceptionCheckingModeDomain[domainIndex];

    offset = 2 * messageType;

    switch (checkingMode)
    {
        case CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_BASIC_E:
            hwData = 0;
            break;
        case CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_TRANS_CLK_E:
            hwData = 1;
            break;
        case CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_PIGGY_BACK_E:
            hwData = 2;
            break;
        case CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_BOUNDRY_CLK_E:
            hwData = 3;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpSetRegField(
        devNum, regAddr, offset, 2, hwData);

    return rc;
}

/**
* @internal cpssDxChPtpIngressPacketCheckingModeSet function
* @endinternal
*
* @brief   Set PTP packet Ingress Checking Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] checkingMode             - PTP packet ingress checking mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpIngressPacketCheckingModeSet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   domainIndex,
    IN  GT_U32                                   messageType,
    IN  CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT  checkingMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpIngressPacketCheckingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, domainIndex, messageType, checkingMode));

    rc = internal_cpssDxChPtpIngressPacketCheckingModeSet(devNum, domainIndex, messageType, checkingMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, domainIndex, messageType, checkingMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpIngressPacketCheckingModeGet function
* @endinternal
*
* @brief   Get PTP packet Ingress Checking Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] checkingModePtr          - (pointer to)PTP packet ingress checking mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpIngressPacketCheckingModeGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   domainIndex,
    IN  GT_U32                                   messageType,
    OUT CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT  *checkingModePtr
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    regAddr;          /* register address             */
    GT_U32    hwData;           /* HW data                      */
    GT_U32    offset;           /* field offset                 */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(checkingModePtr);

    if (domainIndex > 4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (messageType > 15)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr =
        PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.
            PTPExceptionCheckingModeDomain[domainIndex];

    offset = 2 * messageType;

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, offset, 2, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (hwData)
    {
        default:
        case 0:
            *checkingModePtr = CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_BASIC_E;
            break;
        case 1:
            *checkingModePtr = CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_TRANS_CLK_E;
            break;
        case 2:
            *checkingModePtr = CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_PIGGY_BACK_E;
            break;
        case 3:
            *checkingModePtr = CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_BOUNDRY_CLK_E;
            break;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpIngressPacketCheckingModeGet function
* @endinternal
*
* @brief   Get PTP packet Ingress Checking Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] checkingModePtr          - (pointer to)PTP packet ingress checking mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpIngressPacketCheckingModeGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   domainIndex,
    IN  GT_U32                                   messageType,
    OUT CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT  *checkingModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpIngressPacketCheckingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, domainIndex, messageType, checkingModePtr));

    rc = internal_cpssDxChPtpIngressPacketCheckingModeGet(devNum, domainIndex, messageType, checkingModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, domainIndex, messageType, checkingModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpIngressExceptionCounterGet function
* @endinternal
*
* @brief   Get PTP Ingress Exception packet Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] counterPtr               - (pointer to) PTP Ingress Exception packet Counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is Clear On Read.
*
*/
static GT_STATUS internal_cpssDxChPtpIngressExceptionCounterGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *counterPtr
)
{
    GT_U32    regAddr;          /* register address                 */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).PTP.PTPExceptionsCntr;

    return prvCpssPortGroupsCounterSummary(
        devNum, regAddr,
        0 /*fieldOffset*/, 8 /*fieldLength*/,
        counterPtr, NULL /*counter64bitValuePtr*/);
}

/**
* @internal cpssDxChPtpIngressExceptionCounterGet function
* @endinternal
*
* @brief   Get PTP Ingress Exception packet Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] counterPtr              - (pointer to) PTP Ingress Exception packet Counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is Clear On Read.
*
*/
GT_STATUS cpssDxChPtpIngressExceptionCounterGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpIngressExceptionCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counterPtr));

    rc = internal_cpssDxChPtpIngressExceptionCounterGet(devNum, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpEgressExceptionCfgSet function
* @endinternal
*
* @brief   Set PTP packet Egress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] egrExceptionCfgPtr       - (pointer to) PTP packet Egress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpEgressExceptionCfgSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC   *egrExceptionCfgPtr
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    regAddr;          /* register address             */
    GT_U32    hwData;           /* HW data                      */
    GT_U32    hwValue;          /* HW value                     */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(egrExceptionCfgPtr);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* PTP Exception CPU Code Config */
        hwData = 0;
        PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(
            hwValue, egrExceptionCfgPtr->invalidPtpPktCmd);
        hwData |= hwValue;
        if (egrExceptionCfgPtr->invalidPtpPktCmd != CPSS_PACKET_CMD_FORWARD_E)
        {
            rc = prvCpssDxChNetIfCpuToDsaCode(
                egrExceptionCfgPtr->invalidPtpCpuCode, &dsaCpuCode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            hwData |= (dsaCpuCode << 3);
        }

        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).PTPExceptionCPUCodeConfig;

        rc = prvCpssHwPpSetRegField(
            devNum, regAddr, 0, 11, hwData);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* TimeStamping Exception Config */
        hwData = 0;
        PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(
            hwValue, egrExceptionCfgPtr->invalidInPiggybackPktCmd);
        hwData |= hwValue;

        if (egrExceptionCfgPtr->invalidInPiggybackPktCmd != CPSS_PACKET_CMD_FORWARD_E)
        {
            rc = prvCpssDxChNetIfCpuToDsaCode(
                egrExceptionCfgPtr->invalidInPiggybackCpuCode, &dsaCpuCode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            hwData |= (dsaCpuCode << 3);
        }

        PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(
            hwValue, egrExceptionCfgPtr->invalidOutPiggybackPktCmd);
        hwData |= (hwValue << 11);

        if (egrExceptionCfgPtr->invalidOutPiggybackPktCmd != CPSS_PACKET_CMD_FORWARD_E)
        {
            rc = prvCpssDxChNetIfCpuToDsaCode(
                egrExceptionCfgPtr->invalidOutPiggybackCpuCode, &dsaCpuCode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            hwData |= (dsaCpuCode << 14);
        }

        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).PTPAndTimestampingExceptionConfig;

        rc = prvCpssHwPpSetRegField(
            devNum, regAddr, 0, 22, hwData);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Invalid Timestamp exception Config */
        hwData = 0;
        PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(
            hwValue, egrExceptionCfgPtr->invalidTsPktCmd);
        hwData |= hwValue;

        if (egrExceptionCfgPtr->invalidTsPktCmd != CPSS_PACKET_CMD_FORWARD_E)
        {
            rc = prvCpssDxChNetIfCpuToDsaCode(
                egrExceptionCfgPtr->invalidTsCpuCode, &dsaCpuCode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            hwData |= (dsaCpuCode << 3);
        }

        regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).ERMRKPTPConfig;

        rc = prvCpssHwPpSetRegField(
            devNum, regAddr, 11, 11, hwData);

        if (rc != GT_OK)
        {
            return rc;
        }

    }
    else
    {
        hwData = 0;
        PRV_CPSS_CONVERT_PTP_EGRESS_EXCEPTION_CMD_TO_HW_VAL_MAC(
            hwValue, egrExceptionCfgPtr->invalidPtpPktCmd);

        hwData |= hwValue;

        PRV_CPSS_CONVERT_PTP_EGRESS_EXCEPTION_CMD_TO_HW_VAL_MAC(
           hwValue, egrExceptionCfgPtr->invalidInPiggybackPktCmd);
        hwData |= (hwValue << 1);

        PRV_CPSS_CONVERT_PTP_EGRESS_EXCEPTION_CMD_TO_HW_VAL_MAC(
           hwValue, egrExceptionCfgPtr->invalidOutPiggybackPktCmd);
        hwData |= (hwValue << 2);

        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).PTPAndTimestampingExceptionConfig;

        rc = prvCpssHwPpSetRegField(
            devNum, regAddr, 0, 3, hwData);

        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_CONVERT_PTP_EGRESS_EXCEPTION_CMD_TO_HW_VAL_MAC(
            hwValue, egrExceptionCfgPtr->invalidTsPktCmd);

        regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).ERMRKPTPConfig;

        rc = prvCpssHwPpSetRegField(
            devNum, regAddr, 1, 1, hwValue);
    }
    return rc;
}

/**
* @internal cpssDxChPtpEgressExceptionCfgSet function
* @endinternal
*
* @brief   Set PTP packet Egress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] egrExceptionCfgPtr       - (pointer to) PTP packet Egress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpEgressExceptionCfgSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC   *egrExceptionCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpEgressExceptionCfgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, egrExceptionCfgPtr));

    rc = internal_cpssDxChPtpEgressExceptionCfgSet(devNum, egrExceptionCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, egrExceptionCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpEgressExceptionCfgGet function
* @endinternal
*
* @brief   Get PTP packet Egress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] egrExceptionCfgPtr       - (pointer to) PTP packet Egress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpEgressExceptionCfgGet
(
    IN  GT_U8                                    devNum,
    OUT CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC   *egrExceptionCfgPtr
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    regAddr;          /* register address             */
    GT_U32    hwData;           /* HW data                      */
    GT_U32    hwValue;          /* HW value                     */
    GT_U32    hwCmd;          /* HW value                     */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(egrExceptionCfgPtr);

    cpssOsMemSet(egrExceptionCfgPtr, 0, sizeof(*egrExceptionCfgPtr));
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).PTPExceptionCPUCodeConfig;

        rc = prvCpssHwPpGetRegField(
            devNum, regAddr, 0, 11, &hwData);

        if (rc != GT_OK)
        {
            return rc;
        }

        hwValue = (hwData & 0x7);
        PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC (
            egrExceptionCfgPtr->invalidPtpPktCmd, hwValue);

        hwCmd = (hwData >> 3) & 0xFF;
        dsaCpuCode = (PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)(hwCmd);

        if (egrExceptionCfgPtr->invalidPtpPktCmd != CPSS_PACKET_CMD_FORWARD_E)
        {
            rc = prvCpssDxChNetIfDsaToCpuCode(
            dsaCpuCode, &(egrExceptionCfgPtr->invalidPtpCpuCode));
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).PTPAndTimestampingExceptionConfig;
        rc = prvCpssHwPpGetRegField(
            devNum, regAddr, 0, 22, &hwData);

        if (rc != GT_OK)
        {
            return rc;
        }

        hwValue = (hwData & 0x7);
        PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC (
            egrExceptionCfgPtr->invalidInPiggybackPktCmd, hwValue);

        hwCmd = (hwData >> 3) & 0xFF;
        dsaCpuCode = (PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)(hwCmd);

        if (egrExceptionCfgPtr->invalidInPiggybackPktCmd != CPSS_PACKET_CMD_FORWARD_E)
        {
            rc = prvCpssDxChNetIfDsaToCpuCode(
                dsaCpuCode, &(egrExceptionCfgPtr->invalidInPiggybackCpuCode));

            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        hwValue = (hwData >>11) & 0x7;
        PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC (
            egrExceptionCfgPtr->invalidOutPiggybackPktCmd , hwValue);

        hwCmd = (hwData >> 14) & 0xFF;
        dsaCpuCode = (PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)(hwCmd);

        if (egrExceptionCfgPtr->invalidOutPiggybackPktCmd != CPSS_PACKET_CMD_FORWARD_E)
        {
            rc = prvCpssDxChNetIfDsaToCpuCode(
                dsaCpuCode, &(egrExceptionCfgPtr->invalidOutPiggybackCpuCode));

            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).ERMRKPTPConfig;

        rc = prvCpssHwPpGetRegField(
            devNum, regAddr, 11, 11, &hwData);
        if (rc != GT_OK)
        {
            return rc;
        }

        hwValue = (hwData & 0x7);
        PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC (
            egrExceptionCfgPtr->invalidTsPktCmd, hwValue);

        hwCmd = (hwData >> 3) & 0xFF;
        dsaCpuCode = (PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)(hwCmd);

        if (egrExceptionCfgPtr->invalidTsPktCmd != CPSS_PACKET_CMD_FORWARD_E)
        {
            rc = prvCpssDxChNetIfDsaToCpuCode(
                dsaCpuCode, &(egrExceptionCfgPtr->invalidTsCpuCode));
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).PTPAndTimestampingExceptionConfig;

        rc = prvCpssHwPpGetRegField(
            devNum, regAddr, 0, 3, &hwData);
        if (rc != GT_OK)
        {
            return rc;
        }

        hwValue = (hwData & 1);
        PRV_CPSS_CONVERT_HW_VAL_TO_PTP_EGRESS_EXCEPTION_CMD_MAC(
            hwValue, egrExceptionCfgPtr->invalidPtpPktCmd);

        hwValue = ((hwData >> 1) & 1);
        PRV_CPSS_CONVERT_HW_VAL_TO_PTP_EGRESS_EXCEPTION_CMD_MAC(
            hwValue, egrExceptionCfgPtr->invalidInPiggybackPktCmd);

        hwValue = ((hwData >> 2) & 1);
        PRV_CPSS_CONVERT_HW_VAL_TO_PTP_EGRESS_EXCEPTION_CMD_MAC(
            hwValue, egrExceptionCfgPtr->invalidOutPiggybackPktCmd);

        regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).ERMRKPTPConfig;

        rc = prvCpssHwPpGetRegField(
            devNum, regAddr, 1, 1, &hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_CONVERT_HW_VAL_TO_PTP_EGRESS_EXCEPTION_CMD_MAC(
            hwValue, egrExceptionCfgPtr->invalidTsPktCmd);

    }
    return GT_OK;
}

/**
* @internal cpssDxChPtpEgressExceptionCfgGet function
* @endinternal
*
* @brief   Get PTP packet Egress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] egrExceptionCfgPtr       - (pointer to) PTP packet Egress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpEgressExceptionCfgGet
(
    IN  GT_U8                                    devNum,
    OUT CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC   *egrExceptionCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpEgressExceptionCfgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, egrExceptionCfgPtr));

    rc = internal_cpssDxChPtpEgressExceptionCfgGet(devNum, egrExceptionCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, egrExceptionCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpEgressExceptionCountersGet function
* @endinternal
*
* @brief   Get PTP packet Egress Exception Counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] egrExceptionCntPtr       - (pointer to) PTP packet Egress Exception Counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are Clear On Read.
*
*/
static GT_STATUS internal_cpssDxChPtpEgressExceptionCountersGet
(
    IN  GT_U8                                         devNum,
    OUT CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC   *egrExceptionCntPtr
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    regAddr;          /* register address             */
    GT_U32    fieldLength;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(egrExceptionCntPtr);

    cpssOsMemSet(egrExceptionCntPtr, 0, sizeof(*egrExceptionCntPtr));

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).ERMRKPTPInvalidTimestampCntr;

    rc = prvCpssPortGroupsCounterSummary(
        devNum, regAddr,
        0 /*fieldOffset*/, 8 /*fieldLength*/,
        &(egrExceptionCntPtr->invalidTsPktCnt), NULL /*counter64bitValuePtr*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).invalidPTPHeaderCntr;

    rc = prvCpssPortGroupsCounterSummary(
        devNum, regAddr,
        0 /*fieldOffset*/, 8 /*fieldLength*/,
        &(egrExceptionCntPtr->invalidPtpPktCnt), NULL /*counter64bitValuePtr*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).invalidPTPIncomingPiggybackCntr;

    rc = prvCpssPortGroupsCounterSummary(
        devNum, regAddr,
        0 /*fieldOffset*/, 8 /*fieldLength*/,
        &(egrExceptionCntPtr->invalidInPiggybackPktCnt), NULL /*counter64bitValuePtr*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).invalidPTPOutgoingPiggybackCntr;
    fieldLength = PRV_CPSS_SIP_6_CHECK_MAC(devNum)?16:8;
    rc = prvCpssPortGroupsCounterSummary(
        devNum, regAddr,
        0 /*fieldOffset*/, fieldLength /*fieldLength*/,
        &(egrExceptionCntPtr->invalidOutPiggybackPktCnt), NULL /*counter64bitValuePtr*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpEgressExceptionCountersGet function
* @endinternal
*
* @brief   Get PTP packet Egress Exception Counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] egrExceptionCntPtr       - (pointer to) PTP packet Egress Exception Counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are Clear On Read.
*
*/
GT_STATUS cpssDxChPtpEgressExceptionCountersGet
(
    IN  GT_U8                                         devNum,
    OUT CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC   *egrExceptionCntPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpEgressExceptionCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, egrExceptionCntPtr));

    rc = internal_cpssDxChPtpEgressExceptionCountersGet(devNum, egrExceptionCntPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, egrExceptionCntPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPtpTsIngressOrEgressTimestampQueueEntryRead function
* @endinternal
*
* @brief   Read Ingress or Egress Timestamp Queue Entry in HW format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - Queue Number
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] portGroupId              - PortGroup (pipe) number
* @param[in] egressQueue              - GT_TRUE - egress Queue, GT_FALSE - ingress Queue
*
* @param[out] hwDataPtr                - (pointer to) buffer with the row entry.
*                                      3 words for ingress queue
*                                      2 words for ingress queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
*/
static GT_STATUS prvCpssDxChPtpTsIngressOrEgressTimestampQueueEntryRead
(
    IN  GT_U8    devNum,
    IN  GT_U32   queueNum,
    IN  GT_U32   portGroupId,
    IN  GT_BOOL  egressQueue,
    OUT GT_U32   *hwDataPtr
)
{
    GT_STATUS rc;               /* return code              */
    GT_U32    queueIndex;       /* queueIndex               */
    GT_U32    regAddr;          /* register address         */
    GT_U32    i;                /* loop index               */
    GT_U32    wordsNum;         /* num of queue entry words */

    queueIndex = ((BOOL2BIT_MAC(egressQueue) * 2) + queueNum);/* ingress - 0,1 , egress - 2,3 */
    wordsNum   = ((egressQueue == GT_FALSE) ? 3 : 2);

    for (i = 0; (i < wordsNum); i++)
    {
        switch (i)
        {
            default:
            case 0:
                regAddr =
                    PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).timestampQueueEntryWord0[queueIndex];
                break;
            case 1:
                regAddr =
                    PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).timestampQueueEntryWord1[queueIndex];
                break;
            case 2: /* ingress only */
                regAddr =
                    PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).timestampQueueEntryWord2[queueIndex];
                break;
        }
        rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &(hwDataPtr[i]));

        if (rc != GT_OK)
        {
            return rc;
        }
         /* check that the first word is valid */
        if ((i == 0) &&  ((hwDataPtr[0] & 1) == 0) )
        {
           if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
           {
                /* if entry contains garbage (not empty); read whole entry */
                if (hwDataPtr[0] != 0)
                {
                    continue;
                }
           }

           /*Do not read word 1/2.
             The design is not well protected if reading an empty queue word1/word2 while a new entry is pushed in */
           break;

        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPtpTsIngressTimestampQueueEntryRead function
* @endinternal
*
* @brief   Read Ingress Timestamp Queue Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - Queue Number
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] tsQueueEntryPtr          - (pointer to) Ingress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Queue read causes deleting the entry from the queue.
*       The next read will retrieve the next entry.
*
*/
static GT_STATUS internal_cpssDxChPtpTsIngressTimestampQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   *tsQueueEntryPtr
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    hwEntry[3];       /* HW Entry                     */
    GT_U32    hwValue;          /* HW Value                     */
    GT_U32    portGroupId;      /* the port group Id - support multi-
                                                port-groups device  */
    GT_U32    origPortGroupId;/* the original port group that we started with */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(tsQueueEntryPtr);

    if (queueNum >= PRV_CPSS_MAX_PTP_TIMESTAMP_QUEUES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "queueNum[%d] must be less than [%d]",
                                      queueNum,
                                      PRV_CPSS_MAX_PTP_TIMESTAMP_QUEUES_CNS);
    }
    cpssOsMemSet(tsQueueEntryPtr, 0, sizeof(*tsQueueEntryPtr));

    /* get the current port group ID to serve */
    if (GT_FALSE == PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        portGroupId = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup;
    }
    else
    {
        portGroupId =
            PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.ptpIngressEgressNextPortGroupToServe[CPSS_DIRECTION_INGRESS_E][queueNum];
    }

    origPortGroupId = portGroupId;

    do {
           rc = prvCpssDxChPtpTsIngressOrEgressTimestampQueueEntryRead(
                devNum, queueNum,portGroupId, GT_FALSE /*egressQueue*/, hwEntry);
            if (rc != GT_OK)
            {
                return rc;
            }

            portGroupId++;
            if(portGroupId > PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.lastActivePortGroup )
            {
                portGroupId = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup;
            }
            PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.ptpIngressEgressNextPortGroupToServe[CPSS_DIRECTION_INGRESS_E][queueNum] = portGroupId;

            hwValue = (hwEntry[0] & 1);
            tsQueueEntryPtr->entryValid = BIT2BOOL_MAC(hwValue);

            if(tsQueueEntryPtr->entryValid == GT_TRUE)
            {
                break;
            }
    }while (portGroupId != origPortGroupId);

    hwValue = ((hwEntry[0] >> 1) & 1);
    tsQueueEntryPtr->isPtpExeption = BIT2BOOL_MAC(hwValue);

    hwValue = ((hwEntry[0] >> 2) & 7);
    switch (hwValue)
    {
        case 0:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E;
            break;
        case 1:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E;
            break;
        case 2:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_Y1731_E;
            break;
        case 3:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TS_E;
            break;
        case 4:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_RX_E;
            break;
        case 5:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TX_E;
            break;
        case 6:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_WAMP_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        tsQueueEntryPtr->taiSelect = ((hwEntry[2] >> 13) & 3) | ((hwEntry[0] >> 5) & 1);
    }
    else
    {
        tsQueueEntryPtr->taiSelect = ((hwEntry[0] >> 5) & 1);
    }

    tsQueueEntryPtr->todUpdateFlag = ((hwEntry[0] >> 6) & 1);

    tsQueueEntryPtr->messageType = ((hwEntry[0] >> 7) & 0xF);

    tsQueueEntryPtr->domainNum = ((hwEntry[0] >> 11) & 0xFF);

    tsQueueEntryPtr->sequenceId =
        ((hwEntry[0] >> 19) & 0x1FFF) | ((hwEntry[1] & 0x7) << 13);

    tsQueueEntryPtr->timestamp =
        ((hwEntry[1] >> 3) & 0x1FFFFFFF) | ((hwEntry[2] & 0x7) << 29);

    tsQueueEntryPtr->portNum = ((hwEntry[2] >> 3) & 0xFF);

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsIngressTimestampQueueEntryRead function
* @endinternal
*
* @brief   Read Ingress Timestamp Queue Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - Queue Number
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] tsQueueEntryPtr          - (pointer to) Ingress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Queue read causes deleting the entry from the queue.
*       The next read will retrieve the next entry.
*
*/
GT_STATUS cpssDxChPtpTsIngressTimestampQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   *tsQueueEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsIngressTimestampQueueEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueNum, tsQueueEntryPtr));

    rc = internal_cpssDxChPtpTsIngressTimestampQueueEntryRead(devNum, queueNum, tsQueueEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueNum, tsQueueEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsEgressTimestampQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Queue Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - Queue Number
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] tsQueueEntryPtr          - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Queue read causes deleting the entry from the queue.
*       The next read will retrieve the next entry.
*
*/
static GT_STATUS internal_cpssDxChPtpTsEgressTimestampQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC    *tsQueueEntryPtr
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    hwEntry[2];       /* HW Entry                     */
    GT_U32    hwValue;          /* HW Value                     */
    GT_U32    portGroupId;      /* the port group Id - support multi-
                                                port-groups device  */
    GT_U32    origPortGroupId;/* the original port group that we started with */
    GT_U32    portMask; /* port field bit mask */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(tsQueueEntryPtr);

    if (queueNum >= PRV_CPSS_MAX_PTP_TIMESTAMP_QUEUES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "queueNum[%d] must be less than [%d]",
                                      queueNum,
                                      PRV_CPSS_MAX_PTP_TIMESTAMP_QUEUES_CNS);
    }
    cpssOsMemSet(tsQueueEntryPtr, 0, sizeof(*tsQueueEntryPtr));

    if (GT_FALSE == PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        portGroupId = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup;
    }
    else
    {
        portGroupId =
            PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.ptpIngressEgressNextPortGroupToServe[CPSS_DIRECTION_EGRESS_E][queueNum];
    }

    origPortGroupId = portGroupId;

    do {
        rc = prvCpssDxChPtpTsIngressOrEgressTimestampQueueEntryRead(
                devNum, queueNum,portGroupId, GT_TRUE /*egressQueue*/, hwEntry);
            if (rc != GT_OK)
            {
                return rc;
            }

            portGroupId++;
            if(portGroupId > PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.lastActivePortGroup )
            {
                portGroupId = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup;
            }
            PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.ptpIngressEgressNextPortGroupToServe[CPSS_DIRECTION_EGRESS_E][queueNum] = portGroupId;

            hwValue = (hwEntry[0] & 1);
            tsQueueEntryPtr->entryValid = BIT2BOOL_MAC(hwValue);

            if(tsQueueEntryPtr->entryValid == GT_TRUE)
            {
                break;
            }
    }while (portGroupId != origPortGroupId);

    hwValue = (hwEntry[0] & 1);
    tsQueueEntryPtr->entryValid = BIT2BOOL_MAC(hwValue);

    hwValue = ((hwEntry[0] >> 1) & 1);
    tsQueueEntryPtr->isPtpExeption = BIT2BOOL_MAC(hwValue);

    hwValue = ((hwEntry[0] >> 2) & 7);
    switch (hwValue)
    {
        case 0:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E;
            break;
        case 1:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E;
            break;
        case 2:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_Y1731_E;
            break;
        case 3:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TS_E;
            break;
        case 4:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_RX_E;
            break;
        case 5:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TX_E;
            break;
        case 6:
            tsQueueEntryPtr->packetFormat = CPSS_DXCH_PTP_TS_PACKET_TYPE_WAMP_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    tsQueueEntryPtr->messageType = ((hwEntry[0] >> 5) & 0xF);

    tsQueueEntryPtr->domainNum = ((hwEntry[0] >> 9) & 0xFF);

    tsQueueEntryPtr->sequenceId =
        ((hwEntry[0] >> 17) & 0x7FFF) | ((hwEntry[1] & 0x1) << 15);

    tsQueueEntryPtr->queueEntryId = ((hwEntry[1] >> 1) & 0x3FF);

    portMask = (PRV_CPSS_SIP_6_CHECK_MAC(devNum)) ? 0x3FF :
                ((PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) ? 0x1FF: 0xFF);

    tsQueueEntryPtr->portNum = ((hwEntry[1] >> 11) & portMask);

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsEgressTimestampQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Queue Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - Queue Number
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] tsQueueEntryPtr          - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Queue read causes deleting the entry from the queue.
*       The next read will retrieve the next entry.
*
*/
GT_STATUS cpssDxChPtpTsEgressTimestampQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC    *tsQueueEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsEgressTimestampQueueEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueNum, tsQueueEntryPtr));

    rc = internal_cpssDxChPtpTsEgressTimestampQueueEntryRead(devNum, queueNum, tsQueueEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueNum, tsQueueEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsMessageTypeToQueueIdMapSet function
* @endinternal
*
* @brief   Set PTP Message Type To Queue Id Map.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] idMapBmp                 - PTP Message Type To Queue Id Map.
*                                      16 bits, bit per message type
*                                      0 - queue0, 1 - queue1
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsMessageTypeToQueueIdMapSet
(
    IN  GT_U8        devNum,
    IN  GT_U32       idMapBmp
)
{
    GT_U32    regAddr;          /* register address         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);



    if (idMapBmp > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).timestampQueueMsgType;

    return prvCpssHwPpSetRegField(
        devNum, regAddr, 0, 16, idMapBmp);
}

/**
* @internal cpssDxChPtpTsMessageTypeToQueueIdMapSet function
* @endinternal
*
* @brief   Set PTP Message Type To Queue Id Map.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] idMapBmp                 - PTP Message Type To Queue Id Map.
*                                      16 bits, bit per message type
*                                      0 - queue0, 1 - queue1
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsMessageTypeToQueueIdMapSet
(
    IN  GT_U8        devNum,
    IN  GT_U32       idMapBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsMessageTypeToQueueIdMapSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, idMapBmp));

    rc = internal_cpssDxChPtpTsMessageTypeToQueueIdMapSet(devNum, idMapBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, idMapBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsMessageTypeToQueueIdMapGet function
* @endinternal
*
* @brief   Get PTP Message Type To Queue Id Map.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] idMapBmpPtr              - (pointer to) PTP Message Type To Queue Id Map.
*                                      16 bits, bit per message type
*                                      0 - queue0, 1 - queue1
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsMessageTypeToQueueIdMapGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *idMapBmpPtr
)
{
    GT_U32    regAddr;          /* register address         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(idMapBmpPtr);

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).timestampQueueMsgType;

    return prvCpssHwPpGetRegField(
        devNum, regAddr, 0, 16, idMapBmpPtr);
}

/**
* @internal cpssDxChPtpTsMessageTypeToQueueIdMapGet function
* @endinternal
*
* @brief   Get PTP Message Type To Queue Id Map.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] idMapBmpPtr              - (pointer to) PTP Message Type To Queue Id Map.
*                                      16 bits, bit per message type
*                                      0 - queue0, 1 - queue1
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsMessageTypeToQueueIdMapGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *idMapBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsMessageTypeToQueueIdMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, idMapBmpPtr));

    rc = internal_cpssDxChPtpTsMessageTypeToQueueIdMapGet(devNum, idMapBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, idMapBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpTsQueuesSizeSet function
* @endinternal
*
* @brief   Set Timestamp Queues Size.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] queueSize                - The size of each PTP Queue.
*                                      (APPLICABLE RANGES: 0..256.)
*                                      If Timestamp Queues Override is Enable,
*                                      the size should be (maximal - 2) == 254.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsQueuesSizeSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      queueSize
)
{
    GT_U32    regAddr;          /* register address         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);



    if (queueSize > 256)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).ERMRKPTPConfig;

    return prvCpssHwPpSetRegField(
        devNum, regAddr, 2, 9, queueSize);
}

/**
* @internal cpssDxChPtpTsQueuesSizeSet function
* @endinternal
*
* @brief   Set Timestamp Queues Size.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] queueSize                - The size of each PTP Queue.
*                                      (APPLICABLE RANGES: 0..256.)
*                                      If Timestamp Queues Override is Enable,
*                                      the size should be (maximal - 2) == 254.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsQueuesSizeSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      queueSize
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsQueuesSizeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueSize));

    rc = internal_cpssDxChPtpTsQueuesSizeSet(devNum, queueSize);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueSize));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsQueuesSizeGet function
* @endinternal
*
* @brief   Get Timestamp Queues Size.
*         Related to all 2 ingress and 2 egress Timestamp Queues; Hawk.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] queueSizePtr             - (pointer to)The size of each PTP Queue.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsQueuesSizeGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *queueSizePtr
)
{
    GT_U32    regAddr;          /* register address         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(queueSizePtr);

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).ERMRKPTPConfig;

    return prvCpssHwPpGetRegField(
        devNum, regAddr, 2, 9, queueSizePtr);
}

/**
* @internal cpssDxChPtpTsQueuesSizeGet function
* @endinternal
*
* @brief   Get Timestamp Queues Size.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] queueSizePtr             - (pointer to)The size of each PTP Queue.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsQueuesSizeGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *queueSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsQueuesSizeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueSizePtr));

    rc = internal_cpssDxChPtpTsQueuesSizeGet(devNum, queueSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDebugQueuesEntryIdsClear function
* @endinternal
*
* @brief   Set Timestamp Queues Current Entry Ids Clear.
*         For Debug only.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDebugQueuesEntryIdsClear
(
    IN  GT_U8       devNum
)
{
    GT_U32    regAddr;          /* register address         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);



    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).timestampQueueEntryIDClear;

    return prvCpssHwPpSetRegField(
        devNum, regAddr, 0, 1, 1);
}

/**
* @internal cpssDxChPtpTsDebugQueuesEntryIdsClear function
* @endinternal
*
* @brief   Set Timestamp Queues Current Entry Ids Clear.
*         For Debug only.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDebugQueuesEntryIdsClear
(
    IN  GT_U8       devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDebugQueuesEntryIdsClear);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChPtpTsDebugQueuesEntryIdsClear(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsDebugQueuesEntryIdsGet function
* @endinternal
*
* @brief   Get Timestamp Queues Current Entry IDs.
*         For Debug only.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] queueEntryId0Ptr         - (pointer to)The PTP Queue0 current entry Id.
* @param[out] queueEntryId1Ptr         - (pointer to)The PTP Queue1 current entry Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsDebugQueuesEntryIdsGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *queueEntryId0Ptr,
    OUT GT_U32      *queueEntryId1Ptr
)
{
    GT_U32    regAddr;          /* register address         */
    GT_U32    hwValue;          /* HW Value                  */
    GT_STATUS rc;               /* return code               */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);


    CPSS_NULL_PTR_CHECK_MAC(queueEntryId0Ptr);
    CPSS_NULL_PTR_CHECK_MAC(queueEntryId1Ptr);

    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).timestampQueueEntryID;

    rc = prvCpssHwPpGetRegField(
        devNum, regAddr, 0, 20, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *queueEntryId0Ptr = (hwValue & 0x3FF);
    *queueEntryId1Ptr = ((hwValue >> 10) & 0x3FF);

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsDebugQueuesEntryIdsGet function
* @endinternal
*
* @brief   Get Timestamp Queues Current Entry IDs.
*         For Debug only.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] queueEntryId0Ptr         - (pointer to)The PTP Queue0 current entry Id.
* @param[out] queueEntryId1Ptr         - (pointer to)The PTP Queue1 current entry Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDebugQueuesEntryIdsGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *queueEntryId0Ptr,
    OUT GT_U32      *queueEntryId1Ptr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsDebugQueuesEntryIdsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queueEntryId0Ptr, queueEntryId1Ptr));

    rc = internal_cpssDxChPtpTsDebugQueuesEntryIdsGet(devNum, queueEntryId0Ptr, queueEntryId1Ptr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queueEntryId0Ptr, queueEntryId1Ptr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsNtpTimeOffsetSet function
* @endinternal
*
* @brief   Set TS NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] ntpTimeOffset            - NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPtpTsNtpTimeOffsetSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            ntpTimeOffset
)
{
    GT_STATUS rc;
    GT_U32    regAddr[2];          /* register address         */
    GT_U32    portNum;
    GT_U32    portMacNum;
    GT_U32    portGroupId;
    GT_U32    localPort;
    GT_U32    hwData[2];
    GT_U32    i;
    GT_U32    numOfPorts;
    CPSS_DXCH_PORT_MAP_STC    portMap;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    regAddr[0] = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).PTPNTPOffset;

    rc = prvCpssHwPpWriteRegister(devNum, regAddr[0], ntpTimeOffset);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        for(i = 0 ; i < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp; i++)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[i].dataPathNumOfPorts == 0)
            {
                /* not valid DP (data path) */
                continue;
            }
            regAddr[0] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[i].ptpNtpOffset;
            rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr[0], ntpTimeOffset);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        return GT_OK;
    }

    numOfPorts = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum);
    for(portNum = 0 ; portNum < numOfPorts; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }
        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        portMacNum = portMap.interfaceNum;
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
        localPort   = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacNum);

        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).NTPPTPOffsetLow;
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr[0], ntpTimeOffset);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).NTPPTPOffsetHigh;
            regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, localPort).NTPPTPOffsetLow;

            hwData[1] = (ntpTimeOffset & 0xFFFF);
            hwData[0] = ((ntpTimeOffset >> 16) & 0xFFFF);

            for (i = 0; (i < 2); i++)
            {
                rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr[i], 0, 16, hwData[i]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTsNtpTimeOffsetSet function
* @endinternal
*
* @brief   Set TS NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] ntpTimeOffset            - NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPtpTsNtpTimeOffsetSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            ntpTimeOffset
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsNtpTimeOffsetSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ntpTimeOffset));

    rc = internal_cpssDxChPtpTsNtpTimeOffsetSet(devNum, ntpTimeOffset);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ntpTimeOffset));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpTsNtpTimeOffsetGet function
* @endinternal
*
* @brief   Get TS NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] ntpTimeOffsetPtr         - (pointer to) NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTsNtpTimeOffsetGet
(
    IN  GT_U8                             devNum,
    OUT GT_U32                            *ntpTimeOffsetPtr
)
{
    GT_U32    regAddr;          /* register address         */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    CPSS_NULL_PTR_CHECK_MAC(ntpTimeOffsetPtr);
    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).PTPNTPOffset;

    return prvCpssHwPpReadRegister(devNum, regAddr, ntpTimeOffsetPtr);
}

/**
* @internal cpssDxChPtpTsNtpTimeOffsetGet function
* @endinternal
*
* @brief   Get TS NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] ntpTimeOffsetPtr         - (pointer to) NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsNtpTimeOffsetGet
(
    IN  GT_U8                             devNum,
    OUT GT_U32                            *ntpTimeOffsetPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsNtpTimeOffsetGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ntpTimeOffsetPtr));

    rc = internal_cpssDxChPtpTsNtpTimeOffsetGet(devNum, ntpTimeOffsetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ntpTimeOffsetPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPulseInModeSet function
* @endinternal
*
* @brief  Set TAI PTP pulse mode.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] pulseMode             - TAI PulseIN signal mode
* @param[in] taiNumber             - TAI number selection.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPtpTaiPulseInModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT     pulseMode
)
{
    GT_U32    regAddr;              /* register address             */
    GT_STATUS rc = GT_OK;
    GT_U32    pulseInBitOffset = 0;  /* field bit offset             */
    GT_U32    pulseMuxBitOffset = 0; /* field bit offset             */
    GT_U32    pulseInFieldValue;    /* register field value         */
    GT_U32    pulseMuxFieldValue;   /* register field value         */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    CPSS_DXCH_PTP_TAI_ID_STC        taiId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TAI_CHECK_MAC(devNum,taiNumber);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                                  CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_IRONMAN_E);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) != GT_TRUE)
    {
        /* Decide the offset according to the TAI number*/
        switch(taiNumber)
        {
            case CPSS_DXCH_PTP_TAI_NUMBER_0_E:
                pulseInBitOffset = 6;
                pulseMuxBitOffset = 10;
                break;
            case CPSS_DXCH_PTP_TAI_NUMBER_1_E:
                pulseInBitOffset = 7;
                pulseMuxBitOffset = 11;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    /* Decide the Field value of Mux and PulseIn according to the pulseMode */
    switch(pulseMode)
    {
        case CPSS_DXCH_PTP_TAI_PULSE_IN_DISABLED_E:
            pulseInFieldValue = 0;
            pulseMuxFieldValue = 0;
            break;
        case CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E:
            pulseInFieldValue = 1;
            pulseMuxFieldValue = 0;
            break;
        case CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E:
            pulseInFieldValue = 1;
            pulseMuxFieldValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        pulseInBitOffset = 1;
        pulseMuxBitOffset = 2;
        taiId.taiNumber = taiNumber;
        taiId.taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E;
        PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, &taiId, taiIterator)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                    devNum, taiIterator.gopNumber, taiNumber).TAICtrlReg1;
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,
                    taiIterator.portGroupId, regAddr, pulseInBitOffset, 1, pulseInFieldValue);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssDrvHwPpPortGroupSetRegField(devNum,
                    taiIterator.portGroupId, regAddr, pulseMuxBitOffset, 1, pulseMuxFieldValue);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)
    }
    else
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl33;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum,
                    regAddr, pulseInBitOffset, 1, pulseInFieldValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        return prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum,
                regAddr, pulseMuxBitOffset, 1, pulseMuxFieldValue);
    }
    return rc;
}

/**
* @internal cpssDxChPtpTaiPulseInModeSet function
* @endinternal
*
* @brief  Set TAI PTP pulse mode.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] pulseMode             - TAI PulseIN signal mode
* @param[in] taiNumber             - TAI number selection.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssDxChPtpTaiPulseInModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT     pulseMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPulseInModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, pulseMode));

    rc = internal_cpssDxChPtpTaiPulseInModeSet(devNum, taiNumber, pulseMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, pulseMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPulseInModeGet function
* @endinternal
*
* @brief  Get TAI PTP pulse mode.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] taiNumber             - TAI number selection.
* @param[OUT] pulseModePtr         - (pointer to) TAI PulseIN signal mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPtpTaiPulseInModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    OUT CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT     *pulseModePtr
)
{
    GT_U32    regAddr;              /* register address             */
    GT_U32    pulseInBitOffset = 0; /* field bit offset             */
    GT_U32    pulseMuxBitOffset = 0;/* field bit offset             */
    GT_U32    pulseInFieldValue;    /* register field value         */
    GT_U32    pulseMuxFieldValue;   /* register field value         */
    GT_STATUS rc;                   /* return code                  */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    CPSS_DXCH_PTP_TAI_ID_STC        taiId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TAI_CHECK_MAC(devNum,taiNumber);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                         CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_IRONMAN_E);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    CPSS_NULL_PTR_CHECK_MAC(pulseModePtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) != GT_TRUE)
    {
        switch(taiNumber)
        {
            case CPSS_DXCH_PTP_TAI_NUMBER_0_E:
                pulseInBitOffset = 6;
                pulseMuxBitOffset = 10;
                break;
            case CPSS_DXCH_PTP_TAI_NUMBER_1_E:
                pulseInBitOffset = 7;
                pulseMuxBitOffset = 11;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        pulseInBitOffset = 1;
        pulseMuxBitOffset = 2;
        taiId.taiNumber = taiNumber;
        taiId.taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E;
        PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, &taiId, taiIterator);
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
                devNum, taiIterator.gopNumber, taiNumber).TAICtrlReg1;
        rc = prvCpssDrvHwPpPortGroupGetRegField(
            devNum, taiIterator.portGroupId, regAddr, pulseInBitOffset, 1, &pulseInFieldValue);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpPortGroupGetRegField(
            devNum, taiIterator.portGroupId, regAddr, pulseMuxBitOffset, 1, &pulseMuxFieldValue);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl33;
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr,
                pulseInBitOffset, 1, &pulseInFieldValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr,
                pulseMuxBitOffset, 1, &pulseMuxFieldValue);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* Decide the pulseMode value according to the Field value of Mux and PulseIn */
    if(pulseInFieldValue == 1)
    {
        *pulseModePtr = (pulseMuxFieldValue == 1)?CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E:CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E;
    }
    else
    {
        *pulseModePtr = CPSS_DXCH_PTP_TAI_PULSE_IN_DISABLED_E;
    }
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiPulseInModeGet function
* @endinternal
*
* @brief  Get TAI PTP pulse mode.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] taiNumber             - TAI number selection.
* @param[OUT] pulseModePtr         - (pointer to) TAI PulseIN signal mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiPulseInModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    OUT CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT     *pulseModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPulseInModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, pulseModePtr));

    rc = internal_cpssDxChPtpTaiPulseInModeGet(devNum, taiNumber, pulseModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, pulseModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPtpPulseInterfaceSet function
* @endinternal
*
* @brief   Set TAI PTP pulse interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] pulseInterfaceDirection  - PTP pulse interface direction.
*                                      (APPLICABLE VALUES:
*                                      CPSS_DIRECTION_INGRESS_E;
*                                      CPSS_DIRECTION_EGRESS_E.)
*                                      For Bobcat3: (APPLICABLE VALUES:
*                                      CPSS_DIRECTION_EGRESS_E.)
* @param[in] taiNumber                - TAI number selection.
*                                      (relevant for PTP output pulse)
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP pulse interface used for generating/receiving discrete pulses
*       that trigger a time-related operation such as Update or Capture.
*
*/

static GT_STATUS internal_cpssDxChPtpTaiPtpPulseInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      pulseInterfaceDirection,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber
)
{
    GT_U32    regAddr = 0;      /* register address             */
    GT_U32    regAddr32 = 0;    /* register address             */
    GT_STATUS rc;               /* return code                  */
    GT_U32    hwValue;          /* HW value                     */
    GT_U32    hwValue32 = 0;    /* HW value                     */
    GT_U32    ptpIntTrigBit = 0;/* bit number for PTP Internal Trigger bit */
    GT_U32    ptpPulseExternalIODirBit = 0; /* PTP Pulse External IO Direction bit */
    GT_U32    ptpTaiSelectBit = 0;  /* bit for TAI unit selection */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E| CPSS_IRONMAN_E);

    /* Bobcat2; Caelum; Bobcat3 */
    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl5;

            ptpTaiSelectBit = 19;
            ptpPulseExternalIODirBit = 28;

        }
        else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl20;

            regAddr32 = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl29;

            ptpTaiSelectBit = 23;
            ptpPulseExternalIODirBit = 13;

        }
        else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl14;

            regAddr32 = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl20;

            ptpTaiSelectBit = 21;
            ptpPulseExternalIODirBit = 1;
        }
        else /* SIP6 */
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;

            regAddr32 = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl32;

            ptpTaiSelectBit = 7;
            ptpPulseExternalIODirBit = 1;
        }
    }
    else if (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;
        ptpTaiSelectBit = 13;
        ptpPulseExternalIODirBit = 16;   /* not relevant for BC3 */
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;
        ptpTaiSelectBit = 13;
        ptpPulseExternalIODirBit = 0;   /* not relevant for BC3 */
    }
    else
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl4;
        ptpPulseExternalIODirBit = 28;
        ptpTaiSelectBit = 23;

        if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
        {
            ptpPulseExternalIODirBit = 31;
        }
    }

    rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr32, &hwValue32);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    switch(pulseInterfaceDirection)
    {
    case CPSS_DIRECTION_INGRESS_E:
            if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
            {
                hwValue |= (1 << ptpPulseExternalIODirBit);
                break;
            }
            else if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                hwValue32 |= (1 << ptpPulseExternalIODirBit);
                break;
            }
            hwValue |= (1 << ptpPulseExternalIODirBit);
            /* this bit[22] must be 0 when listening external pulses  */
            /* Switching value 0 to 1 in this bit triggers            */
            /* sending pulses to external devices - implemented in    */
            /* cpssDxChPtpTaiTodCounterFunctionAllTriggerSet          */
            hwValue &= ~(1 << ptpIntTrigBit);
            break;
        case CPSS_DIRECTION_EGRESS_E:
            if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
            {
                hwValue &= ~(1 << ptpPulseExternalIODirBit);
            }
            else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                hwValue32 &= ~(1 << ptpPulseExternalIODirBit);
            }
            else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_FALSE)
            {
                hwValue &= ~(1 << ptpPulseExternalIODirBit);
            }
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance == GT_FALSE)
            {
                switch(taiNumber)
                {
                    case CPSS_DXCH_PTP_TAI_NUMBER_0_E:
                        hwValue &= ~(1 << ptpTaiSelectBit);
                        break;
                    case CPSS_DXCH_PTP_TAI_NUMBER_1_E:
                        hwValue |=  (1 << ptpTaiSelectBit);
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum, regAddr32, hwValue32);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum, regAddr, hwValue);
}

/**
* @internal cpssDxChPtpTaiPtpPulseInterfaceSet function
* @endinternal
*
* @brief   Set TAI PTP pulse interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] pulseInterfaceDirection  - PTP pulse interface direction.
*                                      (APPLICABLE VALUES:
*                                      CPSS_DIRECTION_INGRESS_E;
*                                      CPSS_DIRECTION_EGRESS_E.)
*                                      For Bobcat3: (APPLICABLE VALUES:
*                                      CPSS_DIRECTION_EGRESS_E.)
* @param[in] taiNumber                - TAI number selection.
*                                      (relevant for PTP output pulse)
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP pulse interface used for generating/receiving discrete pulses
*       that trigger a time-related operation such as Update or Capture.
*
*/
GT_STATUS cpssDxChPtpTaiPtpPulseInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      pulseInterfaceDirection,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPtpPulseInterfaceSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pulseInterfaceDirection, taiNumber));

    rc = internal_cpssDxChPtpTaiPtpPulseInterfaceSet(devNum, pulseInterfaceDirection, taiNumber);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pulseInterfaceDirection, taiNumber));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPtpPulseInterfaceGet function
* @endinternal
*
* @brief   Get TAI PTP pulse interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] pulseInterfaceDirectionPtr - (pointer to) PTP pulse interface direction.
* @param[out] taiNumberPtr             - (pointer to) TAI number selection.
*                                      (relevant for PTP output pulse)
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP pulse interface used for generating/receiving discrete pulses
*       that trigger a time-related operation such as Update or Capture.
*
*/
static GT_STATUS internal_cpssDxChPtpTaiPtpPulseInterfaceGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DIRECTION_ENT                   *pulseInterfaceDirectionPtr,
    OUT CPSS_DXCH_PTP_TAI_NUMBER_ENT         *taiNumberPtr
)
{
    GT_U32    regAddr;          /* register address             */
    GT_U32    regAddr32 = 0;    /* register address             */
    GT_STATUS rc;               /* return code                  */
    GT_U32    hwValue = 0;      /* HW value                     */
    GT_U32    hwValue32 = 0;        /* HW value                     */
    GT_U32    ptpPulseExternalIODirBit = 0; /* PTP Pulse External IO Direction bit */
    GT_U32    ptpTaiSelectBit = 0;  /* bit for TAI unit selection */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E| CPSS_IRONMAN_E);


    CPSS_NULL_PTR_CHECK_MAC(pulseInterfaceDirectionPtr);
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance == GT_FALSE)
    {
        CPSS_NULL_PTR_CHECK_MAC(taiNumberPtr);
    }

    /* Bobcat2; Caelum; Bobcat3 */
    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl5;

            ptpTaiSelectBit = 19;
            ptpPulseExternalIODirBit = 28;

        }
        else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl20;

            regAddr32 = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl29;

            ptpTaiSelectBit = 23;
            ptpPulseExternalIODirBit = 13;

        }
        else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl14;

            regAddr32 = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl20;

            ptpTaiSelectBit = 21;
            ptpPulseExternalIODirBit = 1;
        }
        else /* SIP6 */
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;

            regAddr32 = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl32;

            ptpTaiSelectBit = 7;
            ptpPulseExternalIODirBit = 1;
        }
    }
    else if (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;
        ptpTaiSelectBit = 13;
        ptpPulseExternalIODirBit = 16;
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;
        ptpTaiSelectBit = 13;
        ptpPulseExternalIODirBit = 0;   /* not relevant for BC3 */
    }
    else
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl4;

        ptpPulseExternalIODirBit = 28;
        ptpTaiSelectBit = 23;
        if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
        {
            ptpPulseExternalIODirBit = 31;
        }
    }

    rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr32, &hwValue32);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        *pulseInterfaceDirectionPtr =
          ((hwValue >> ptpPulseExternalIODirBit) & 0x1)?CPSS_DIRECTION_INGRESS_E:CPSS_DIRECTION_EGRESS_E;
    }
    else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        *pulseInterfaceDirectionPtr =
          ((hwValue32 >> ptpPulseExternalIODirBit) & 0x1)?CPSS_DIRECTION_INGRESS_E:CPSS_DIRECTION_EGRESS_E;
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_FALSE)
    {
        if (((hwValue >> ptpPulseExternalIODirBit) & 0x1) == 0)
        {
            *pulseInterfaceDirectionPtr = CPSS_DIRECTION_EGRESS_E;
        }
        else
        {
            *pulseInterfaceDirectionPtr = CPSS_DIRECTION_INGRESS_E;
        }
    }
    else
    {
        *pulseInterfaceDirectionPtr = CPSS_DIRECTION_EGRESS_E;
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance == GT_FALSE)
    {
        if(((hwValue >> ptpTaiSelectBit) & 0x1) == 0)
        {
            *taiNumberPtr = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
        }
        else
        {
            *taiNumberPtr = CPSS_DXCH_PTP_TAI_NUMBER_1_E;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiPtpPulseInterfaceGet function
* @endinternal
*
* @brief   Get TAI PTP pulse interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] pulseInterfaceDirectionPtr - (pointer to) PTP pulse interface direction.
* @param[out] taiNumberPtr             - (pointer to) TAI number selection.
*                                      (relevant for PTP output pulse)
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP pulse interface used for generating/receiving discrete pulses
*       that trigger a time-related operation such as Update or Capture.
*
*/
GT_STATUS cpssDxChPtpTaiPtpPulseInterfaceGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DIRECTION_ENT                   *pulseInterfaceDirectionPtr,
    OUT CPSS_DXCH_PTP_TAI_NUMBER_ENT         *taiNumberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPtpPulseInterfaceGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pulseInterfaceDirectionPtr, taiNumberPtr));

    rc = internal_cpssDxChPtpTaiPtpPulseInterfaceGet(devNum, pulseInterfaceDirectionPtr, taiNumberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pulseInterfaceDirectionPtr, taiNumberPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPClkInterfaceSet function
* @endinternal
*
* @brief   Set TAI PTP clock interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI Number.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2, Falcon; AC5P; AC5X; Harrier)
* @param[in] clockInterfaceDirection  - PTP clock interface direction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP clock interface used for generating/receiving a periodic pulse
*       such as 1 PPS signal.
*
*/
static GT_STATUS internal_cpssDxChPtpTaiPClkInterfaceSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT         taiNumber,
    IN  CPSS_DIRECTION_ENT                   clockInterfaceDirection
)
{
    GT_U32    regAddr = 0;      /* register address             */
    GT_U32    fieldValue;       /* register field value         */
    GT_U32    bitOffset;        /* field bit offset             */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_IRONMAN_E);

    /* Bobcat2; Caelum; Bobcat3 */
    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) != GT_TRUE)
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl4;
    }

    switch(clockInterfaceDirection)
    {
        case CPSS_DIRECTION_INGRESS_E:
            fieldValue = 1;
            break;
        case CPSS_DIRECTION_EGRESS_E:
            fieldValue = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance == GT_FALSE)
    {
        switch(taiNumber)
        {
            case CPSS_DXCH_PTP_TAI_NUMBER_0_E:
                bitOffset = 24;
                if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl5;
                    bitOffset = 26;
                }
                else if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl25;
                    bitOffset = 8;
                }
                else if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl17;
                    bitOffset = 0;
                }
                else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl29;
                    bitOffset = 1;
                }
                break;
            case CPSS_DXCH_PTP_TAI_NUMBER_1_E:
                bitOffset = 25;
                if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl5;
                    bitOffset = 27;
                }
                else if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl25;
                    bitOffset = 9;
                }
                else if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl17;
                    bitOffset = 1;
                }
                else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl29;
                    bitOffset = 2;
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        bitOffset = 24;
        if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
        {
            bitOffset = 29;
        }
    }

    return prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr,
                                                           bitOffset, 1, fieldValue);
}

/**
* @internal cpssDxChPtpTaiPClkInterfaceSet function
* @endinternal
*
* @brief   Set TAI PTP clock interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI Number.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2, Falcon; AC5P; AC5X; Harrier)
* @param[in] clockInterfaceDirection  - PTP clock interface direction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP clock interface used for generating/receiving a periodic pulse
*       such as 1 PPS signal.
*
*/
GT_STATUS cpssDxChPtpTaiPClkInterfaceSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT         taiNumber,
    IN  CPSS_DIRECTION_ENT                   clockInterfaceDirection
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPClkInterfaceSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, clockInterfaceDirection));

    rc = internal_cpssDxChPtpTaiPClkInterfaceSet(devNum, taiNumber, clockInterfaceDirection);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, clockInterfaceDirection));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPClkInterfaceGet function
* @endinternal
*
* @brief   Get TAI PTP clock interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2, Falcon; AC5P; AC5X; Harrier)
*
* @param[out] clockInterfaceDirectionPtr - (pointer to) PTP clock interface direction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP clock interface used for generating/receiving a periodic pulse
*       such as 1 PPS signal.
*
*/
static GT_STATUS internal_cpssDxChPtpTaiPClkInterfaceGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT         taiNumber,
    OUT CPSS_DIRECTION_ENT                   *clockInterfaceDirectionPtr
)
{
    GT_U32    regAddr = 0;      /* register address             */
    GT_STATUS rc;               /* return code                  */
    GT_U32    hwValue;          /* HW value                     */
    GT_U32    bitOffset;        /* field bit offset             */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(clockInterfaceDirectionPtr);

    /* Bobcat2; Caelum; Bobcat3 */
    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) != GT_TRUE)
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl4;
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance == GT_FALSE)
    {
        switch(taiNumber)
        {
            case CPSS_DXCH_PTP_TAI_NUMBER_0_E:
                bitOffset = 24;
                if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl5;
                    bitOffset = 26;
                }
                else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl25;
                    bitOffset = 8;
                }
                else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl17;
                    bitOffset = 0;
                }
                else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl29;
                    bitOffset = 1;
                }
                break;
            case CPSS_DXCH_PTP_TAI_NUMBER_1_E:
                bitOffset = 25;
                if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl5;
                    bitOffset = 27;
                }
                else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl25;
                    bitOffset = 9;
                }
                else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl17;
                    bitOffset = 1;
                }
                else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                        DFXServerUnitsDeviceSpecificRegs.deviceCtrl29;
                    bitOffset = 2;
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        bitOffset = 24;
        if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
        {
            bitOffset = 29;
        }
    }

    rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(((hwValue >> bitOffset) & 0x1) == 0)
    {
        *clockInterfaceDirectionPtr = CPSS_DIRECTION_EGRESS_E;
    }
    else
    {
        *clockInterfaceDirectionPtr = CPSS_DIRECTION_INGRESS_E;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiPClkInterfaceGet function
* @endinternal
*
* @brief   Get TAI PTP clock interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2, Falcon; AC5P; AC5X; Harrier)
*
* @param[out] clockInterfaceDirectionPtr - (pointer to) PTP clock interface direction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP clock interface used for generating/receiving a periodic pulse
*       such as 1 PPS signal.
*
*/
GT_STATUS cpssDxChPtpTaiPClkInterfaceGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT         taiNumber,
    OUT CPSS_DIRECTION_ENT                   *clockInterfaceDirectionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPClkInterfaceGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, clockInterfaceDirectionPtr));

    rc = internal_cpssDxChPtpTaiPClkInterfaceGet(devNum, taiNumber, clockInterfaceDirectionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, clockInterfaceDirectionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPClkOutputInterfaceSet function
* @endinternal
*
* @brief   Set TAI PTP PClk OUT from TAI1/TAI0.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number - used as clock?s source for PClk output mode.
*                                      (relevant for "PTP" mode)
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
* @param[in] pclkRcvrClkMode          - clock output interface mode.

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiPClkOutputInterfaceSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT    pclkRcvrClkMode

)
{
    GT_U32    regAddr;          /* register address             */
    GT_U32    fieldValue;       /* register field value         */
    GT_U32    fieldLen;         /* register field length        */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_IRONMAN_E);


    /* Bobcat2; Caelum; Bobcat3 */
    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl5;
        fieldLen = 1;
    }
    else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        /* The mux between TAI0/1 is in the MPP unit (MPP Control 40-47) and not in the TAI unit */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_MPP_MAC(devNum).MPPControl40_47;
        fieldLen = 1;
    }
    else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl14;
        fieldLen = 1;
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;
        fieldLen = 1;
    }
    else
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl10;
        fieldLen = 2;
    }

    switch(pclkRcvrClkMode)
    {
        case CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_PCLK_E:
            fieldValue = 0;
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance == GT_FALSE)
            {
                switch(taiNumber)
                {
                    case CPSS_DXCH_PTP_TAI_NUMBER_0_E:
                        break;
                    case CPSS_DXCH_PTP_TAI_NUMBER_1_E:
                        fieldValue |= (fieldLen == 1) ? 1 : (1 << 1);
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            break;
        case CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_RCVR_CLK_E:
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            fieldValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        return prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr,
                                                           18, fieldLen, fieldValue);
    }
    else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        /* CPSS_DXCH_PTP_TAI_NUMBER_0_E : MPP[40]_Sel = 1
           CPSS_DXCH_PTP_TAI_NUMBER_1_E : MPP[40]_Sel = 2 */
        return prvCpssHwPpSetRegField(devNum, regAddr, 0, fieldLen, fieldValue+1);
    }
    else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        return prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr,
                                                           20, fieldLen, fieldValue);
    }
    else if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr,
                                                           6, fieldLen, fieldValue);
    }
    return prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr,
                                                           14, fieldLen, fieldValue);
}

/**
* @internal cpssDxChPtpTaiPClkOutputInterfaceSet function
* @endinternal
*
* @brief   Set TAI PTP PClk OUT from TAI1/TAI0.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number - used as clock?s source for PClk output mode.
*                                      (relevant for "PTP" mode)
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
* @param[in] pclkRcvrClkMode          - clock output interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiPClkOutputInterfaceSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT    pclkRcvrClkMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPClkOutputInterfaceSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, pclkRcvrClkMode));

    rc = internal_cpssDxChPtpTaiPClkOutputInterfaceSet(devNum, taiNumber, pclkRcvrClkMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, pclkRcvrClkMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPClkOutputInterfaceGet function
* @endinternal
*
* @brief   Get TAI PTP PClk OUT from TAI1/TAI0.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] taiNumberPtr             - (pointer to) TAI number - used as clock?s source for PClk output mode.
*                                      (relevant for "PTP" mode)
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
* @param[out] pclkRcvrClkModePtr       - (pointer to) clock output interface mode.

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiPClkOutputInterfaceGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_PTP_TAI_NUMBER_ENT                *taiNumberPtr,
    OUT CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT    *pclkRcvrClkModePtr

)
{
    GT_U32    regAddr;          /* register address             */
    GT_STATUS rc;               /* return code                  */
    GT_U32    fieldValue;       /* register field value         */
    GT_U32    fieldLen;         /* register field length        */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_IRONMAN_E);


    CPSS_NULL_PTR_CHECK_MAC(pclkRcvrClkModePtr);
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance == GT_FALSE)
    {
        CPSS_NULL_PTR_CHECK_MAC(taiNumberPtr);
    }

    /* Bobcat2; Caelum; Bobcat3 */
    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl5;
        fieldLen = 1;
    }
    else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        /* The mux between TAI0/1 is in the MPP unit (MPP Control 40-47) and not in the TAI unit */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_MPP_MAC(devNum).MPPControl40_47;
        fieldLen = 1;
    }
    else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl14;
        fieldLen = 1;
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;
        fieldLen = 1;
    }
    else
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl10;
        fieldLen = 2;
    }
    if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr,
                18, fieldLen, &fieldValue);
    }
    else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        /* CPSS_DXCH_PTP_TAI_NUMBER_0_E : MPP[40]_Sel = 1
           CPSS_DXCH_PTP_TAI_NUMBER_1_E : MPP[40]_Sel = 2 */
        rc =  prvCpssHwPpGetRegField(devNum, regAddr, 0, fieldLen, &fieldValue);
    }
    else if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr,
                20, fieldLen, &fieldValue);
    }
    else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr,
                6, fieldLen, &fieldValue);
    }
    else
    {
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr,
                14, fieldLen, &fieldValue);
    }
    if(rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_FALSE)
    {
        if ((fieldValue & 0x1) == 0)
        {
            *pclkRcvrClkModePtr = CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_PCLK_E;
        }
        else
        {
            *pclkRcvrClkModePtr = CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_RCVR_CLK_E;
        }
    }
    else
    {
        *pclkRcvrClkModePtr = CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_PCLK_E;
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance == GT_FALSE)
    {
        if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_FALSE || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E))
        {
            fieldValue >>= 1;
        }
        if ((fieldValue & 0x1) == 0)
        {
            *taiNumberPtr = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
        }
        else
        {
            *taiNumberPtr = CPSS_DXCH_PTP_TAI_NUMBER_1_E;
        }
    }


    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiPClkOutputInterfaceGet function
* @endinternal
*
* @brief   Get TAI PTP PClk OUT from TAI1/TAI0.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] taiNumberPtr             - (pointer to) TAI number - used as clock?s source for PClk output mode.
*                                      (relevant for "PTP" mode)
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
* @param[out] pclkRcvrClkModePtr       - (pointer to) clock output interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiPClkOutputInterfaceGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_PTP_TAI_NUMBER_ENT                *taiNumberPtr,
    OUT CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT    *pclkRcvrClkModePtr

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPClkOutputInterfaceGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumberPtr, pclkRcvrClkModePtr));

    rc = internal_cpssDxChPtpTaiPClkOutputInterfaceGet(devNum, taiNumberPtr, pclkRcvrClkModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumberPtr, pclkRcvrClkModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiInputClockSelectSet function
* @endinternal
*
* @brief   Configures input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Caelum; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] clockSelect              - input clock selection mode.
* @param[in] clockFrequency           - Frequency of the PTP PLL clock (default is 25).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiInputClockSelectSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT        taiNumber,
    IN  CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT  clockSelect,
    IN  CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT    clockFrequency
)
{
    GT_STATUS rc;
    GT_U32    regAddr;          /* register address             */
    GT_U32    fieldValue;       /* register field value         */
    GT_U32    bitOffset;        /* field bit offset             */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_CAELUM_E | CPSS_IRONMAN_E);
    /* Bobcat2; Caelum; Bobcat3 */
    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    /* reset PLL for aldrin*/
    if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl3;

        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 12, 6, 1);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 24, 1, 1);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 24, 1, 0);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 12, 6, 0);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl5;
    }
    else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl19;
    }
    else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl13;
    }
    else if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl15;
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;
    }
    else
    {
        if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnitsDeviceSpecificRegs.deviceCtrl7;
        }
        else
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnitsDeviceSpecificRegs.deviceCtrl4;
        }

    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance == GT_FALSE)
    {
        switch(taiNumber)
        {
        case CPSS_DXCH_PTP_TAI_NUMBER_0_E:
                if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
                {
                    bitOffset = 17;
                }
                else if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
                {
                    bitOffset = 14;
                }
                else if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                {
                    bitOffset = 16;
                }
                else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    bitOffset = 4;
                }
                else
                {
                    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        bitOffset = 9;
                    }
                    else
                    {
                        if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
                        {
                            bitOffset = 31;
                        }
                        else
                        {
                            bitOffset = 26;
                        }

                    }
                }
                break;
            case CPSS_DXCH_PTP_TAI_NUMBER_1_E:
                if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
                {
                    bitOffset = 17;
                }
                else if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
                {
                    bitOffset = 17;
                }
                else if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                {
                    bitOffset = 19;
                }
                else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    bitOffset = 7;
                }
                else
                {
                    bitOffset = (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) ? 12 : 27;
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
        {
            bitOffset = 31;
        }
        else
        {
            bitOffset = 26;
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        fieldValue = (clockSelect == CPSS_DXCH_PTP_TAI_CLOCK_SELECT_PTP_PLL_E)?1:0;
    }
    else
    {
        switch(clockSelect)
        {
            case CPSS_DXCH_PTP_TAI_CLOCK_SELECT_PTP_PLL_E:
                fieldValue = 1;
                break;
            case CPSS_DXCH_PTP_TAI_CLOCK_SELECT_CORE_PLL_E:
                fieldValue = 0;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    rc =  prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr,
                                                           bitOffset, 1, fieldValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((clockSelect == CPSS_DXCH_PTP_TAI_CLOCK_SELECT_PTP_PLL_E) && (PRV_CPSS_SIP_5_16_CHECK_MAC(devNum)))
    {
        return prvCpssDxChPtpPLLConfig(devNum, clockFrequency, taiNumber);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiInputClockSelectSet function
* @endinternal
*
* @brief   Configures input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Caelum; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] clockSelect              - input clock selection mode.
* @param[in] clockFrequency           - Frequency of the PTP PLL clock (default is 25).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiInputClockSelectSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT        taiNumber,
    IN  CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT  clockSelect,
    IN  CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT    clockFrequency
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiInputClockSelectSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, clockSelect, clockFrequency));

    rc = internal_cpssDxChPtpTaiInputClockSelectSet(devNum, taiNumber, clockSelect, clockFrequency);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, clockSelect, clockFrequency));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiInputClockSelectGet function
* @endinternal
*
* @brief   Get input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Caelum.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] clockSelectPtr           - (pointer to) input clock selection mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiInputClockSelectGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT        taiNumber,
    OUT CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT  *clockSelectPtr
)
{
    GT_U32    regAddr;          /* register address             */
    GT_STATUS rc;               /* return code                  */
    GT_U32    fieldValue;       /* register field value         */
    GT_U32    bitOffset;        /* field bit offset             */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_CAELUM_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(clockSelectPtr);

    /* Bobcat2; Caelum; Bobcat3 */
    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }
    if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl5;
    }
    else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl19;
    }
    else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl13;
    }
    else if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl15;
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;
    }
    else
    {
        if (!PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl4;
        }
        else
        {
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl7;
        }
    }

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_tai.supportSingleInstance == GT_FALSE)
    {
        switch(taiNumber)
        {
        case CPSS_DXCH_PTP_TAI_NUMBER_0_E:
                if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
                {
                    bitOffset = 17;
                }
                else if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
                {
                    bitOffset = 14;
                }
                else if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                {
                    bitOffset = 16;
                }
                else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    bitOffset = 4;
                }
                else
                {
                    bitOffset = (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) ? 9 : 26;
                }
                break;
            case CPSS_DXCH_PTP_TAI_NUMBER_1_E:
                if(PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
                {
                    bitOffset = 17;
                }
                if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
                {
                    bitOffset = 17;
                }
                else if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                {
                    bitOffset = 19;
                }
                else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    bitOffset = 7;
                }
                else
                {
                    bitOffset = (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) ? 12 : 27;
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        bitOffset = 26;
        if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
        {
            bitOffset = 31;
        }
    }

    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr,
                                                         bitOffset, 1, &fieldValue);
    if(rc != GT_OK)
    {
        return rc;
    }


    *clockSelectPtr = (fieldValue == 1)? CPSS_DXCH_PTP_TAI_CLOCK_SELECT_PTP_PLL_E:CPSS_DXCH_PTP_TAI_CLOCK_SELECT_CORE_PLL_E;


    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiInputClockSelectGet function
* @endinternal
*
* @brief   Get input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Caelum; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] clockSelectPtr           - (pointer to) input clock selection mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiInputClockSelectGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT        taiNumber,
    OUT CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT  *clockSelectPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiInputClockSelectGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiNumber, clockSelectPtr));

    rc = internal_cpssDxChPtpTaiInputClockSelectGet(devNum, taiNumber, clockSelectPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiNumber, clockSelectPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiTodCounterFunctionAllTriggerSet function
* @endinternal
*
* @brief   Triggeres TOD update operation for all TAI units.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiTodCounterFunctionAllTriggerSet
(
    IN GT_U8                    devNum
)
{
    GT_U32    regAddr;          /* register address             */
    GT_U32    fieldOffset;      /* field offset in register     */
    GT_STATUS rc;               /* return code                  */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;    /* TAI iterator     */
    CPSS_DXCH_PTP_TAI_ID_STC        taiId;
    GT_U32 taiSelectBmp;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E);

    /* Bobcat2; Caelum; Bobcat3 */
    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        /* Global pulse software trigger to all TAIs. */
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl50;
        fieldOffset = 10;

        /* save tai triggering selection */
        rc = cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet(devNum,&taiSelectBmp);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* select all tais for triggering */
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 0, 10, 0x3FF);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set trigger bit */
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, fieldOffset, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* unset trigger bit */
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, fieldOffset, 1, 0);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* restore tai triggering selection */
        rc = cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet(devNum,taiSelectBmp);
        return rc;
    }
    else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        fieldOffset = 0;
        taiId.taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
        taiId.taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_GLOBAL_E;
        PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, &taiId, taiIterator);
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiIterator.gopNumber, taiId.taiNumber).timeCntrFunctionConfig0;

        /* set two bits to enable trigger and initiate trigger to both TAIs  */
        return prvCpssDrvHwPpPortGroupSetRegField(devNum, taiIterator.portGroupId, regAddr, fieldOffset, 2, 3);

    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;
        fieldOffset = 15;
    }
    else
    {
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceCtrl4;
        fieldOffset = 22;

        if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
        {
            fieldOffset = 28;
        }
    }

    /* toggle required for trigger */
    /* unset trigger bit */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, fieldOffset, 1, 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set trigger bit */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, fieldOffset, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* For Bobcat3 need to SW clean bit after set */
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        return prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr,
                fieldOffset, 1, 0);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiTodCounterFunctionAllTriggerSet function
* @endinternal
*
* @brief   Triggeres TOD update operation for all TAI units.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodCounterFunctionAllTriggerSet
(
    IN GT_U8                    devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiTodCounterFunctionAllTriggerSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChPtpTaiTodCounterFunctionAllTriggerSet(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortPtpReset function
* @endinternal
*
* @brief   Reset/unreset PTP unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] resetTxUnit              - GT_TRUE - reset PTP trasmit unit,
*                                      GT_FALSE - unreset PTP trasmit unit
*                                      For Bobcat2 related to both transmit and receive units
* @param[in] resetRxUnit              - GT_TRUE - reset PTP receive unit
*                                      GT_FALSE - unreset PTP receive unit
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPtpReset
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         resetTxUnit,
    IN  GT_BOOL                         resetRxUnit
)
{
    GT_STATUS   rc;             /* returned code */
    GT_U32      portMacNum;     /* port MAC number */
    GT_U32      regAddr;        /* register address */
    GT_U32      resetMask;      /* reset bits mask */
    GT_U32      resetData;      /* reset bits data */
    GT_U32      rxResetMask;    /* bit mask for RX Reset */
    GT_U32      tsuIndex;       /* Timestamp unit index         */
    GT_U32      tsuLocalChannel;/* Timestamp unit local channel */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(
        devNum, portNum, portMacNum);

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].ctsuControl;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;
    }

    resetData = 0;

    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* Any PP: bit1 - PTP reset */
        resetMask = 2;
        if (resetTxUnit == GT_FALSE)
        {
            resetData |= 2;
        }
    }
    else
    {
        /* SIP_6: bit0 - TSU Reset */
        resetMask = 1;
        if (resetTxUnit == GT_FALSE)
        {
            resetData |= 1;
        }
    }

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) &&
        (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))) /* SIP_6 does not have RX Reset */
    {
        /* PTP Rx reset - Bobcat3 bit8, Caelum  and Aldrin bit13 */
        rxResetMask = (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) ? 0x100 : 0x2000;

        resetMask |= rxResetMask;
        if (resetRxUnit == GT_FALSE)
        {
            resetData |= rxResetMask;
        }
    }

    /* Set PTP unit software reset bit/bits */
    return prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, resetMask, resetData);
}

/**
* @internal internal_cpssDxChPtpPortUnitResetSet function
* @endinternal
*
* @brief   Reset/unreset Port PTP unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] resetTxUnit              - GT_TRUE - reset PTP trasmit unit,
*                                      GT_FALSE - unreset PTP trasmit unit
*                                      For Bobcat2 related to both transmit and receive units
* @param[in] resetRxUnit              - GT_TRUE - reset PTP receive unit
*                                      GT_FALSE - unreset PTP receive unit
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpPortUnitResetSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         resetTxUnit,
    IN  GT_BOOL                         resetRxUnit
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    return prvCpssDxChPortPtpReset(devNum,portNum,resetTxUnit,resetRxUnit);
}

/**
* @internal cpssDxChPtpPortUnitResetSet function
* @endinternal
*
* @brief   Reset/unreset Port PTP unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] resetTxUnit              - GT_TRUE - reset PTP trasmit unit,
*                                      GT_FALSE - unreset PTP trasmit unit
*                                      For Bobcat2 related to both transmit and receive units
* @param[in] resetRxUnit              - GT_TRUE - reset PTP receive unit
*                                      GT_FALSE - unreset PTP receive unit
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpPortUnitResetSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         resetTxUnit,
    IN  GT_BOOL                         resetRxUnit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpPortUnitResetSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, resetTxUnit, resetRxUnit));

    rc = internal_cpssDxChPtpPortUnitResetSet(
        devNum, portNum, resetTxUnit, resetRxUnit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, portNum, resetTxUnit, resetRxUnit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpPortUnitResetGet function
* @endinternal
*
* @brief   Get Reset/unreset Port PTP unit state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] resetTxUnitPtr           - (pointer to)
*                                      GT_TRUE - reset PTP trasmit unit,
*                                      GT_FALSE - unreset PTP trasmit unit
*                                      For Bobcat2 related to both transmit and receive units
* @param[out] resetRxUnitPtr           - (pointer to)
*                                      GT_TRUE - reset PTP receive unit
*                                      GT_FALSE - unreset PTP receive unit
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpPortUnitResetGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *resetTxUnitPtr,
    OUT GT_BOOL                         *resetRxUnitPtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      portMacNum;     /* port MAC number */
    GT_U32      regAddr;        /* register address */
    GT_U32      data;           /* register data */
    GT_U32      rxResetMask;    /* bit mask for RX Reset */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(
        devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(resetTxUnitPtr);


    regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;

    /* Get PTP unit software reset bit [1:1] [13:13] */
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* Any PP bit1 - PTP Tx reset */
    *resetTxUnitPtr = (data & 2) ? GT_FALSE : GT_TRUE;

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_NULL_PTR_CHECK_MAC(resetRxUnitPtr);

        /* PTP Rx reset - Bobcat3 bit8, Caelum and Aldrin bit13 */
        rxResetMask = (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) ? 0x100 : 0x2000;
        *resetRxUnitPtr = (data & rxResetMask) ? GT_FALSE : GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpPortUnitResetGet function
* @endinternal
*
* @brief   Get Reset/unreset Port PTP unit state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] resetTxUnitPtr           - (pointer to)
*                                      GT_TRUE - reset PTP trasmit unit,
*                                      GT_FALSE - unreset PTP trasmit unit
*                                      For Bobcat2 related to both transmit and receive units
* @param[out] resetRxUnitPtr           - (pointer to)
*                                      GT_TRUE - reset PTP receive unit
*                                      GT_FALSE - unreset PTP receive unit
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpPortUnitResetGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *resetTxUnitPtr,
    OUT GT_BOOL                         *resetRxUnitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpPortUnitResetGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, resetTxUnitPtr, resetRxUnitPtr));

    rc = internal_cpssDxChPtpPortUnitResetGet(
        devNum, portNum, resetTxUnitPtr, resetRxUnitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, devNum, portNum, resetTxUnitPtr, resetRxUnitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpPortTxPipeStatusDelaySet function
* @endinternal
*
* @brief   Set PTP Tx Pipe Status Delay.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] delay                    -  of empty egress pipe (in core clock cycles).
*                                      (APPLICABLE RANGE: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpPortTxPipeStatusDelaySet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          delay
)
{
    GT_STATUS   rc;             /* returned code */
    GT_U32      portMacNum;     /* port MAC number */
    GT_U32      regAddr;        /* register address */
    GT_U32      tsuIndex;       /* Timestamp unit index         */
    GT_U32      tsuLocalChannel;/* Timestamp unit local channel */
    GT_U32      maxDelayBits;   /* maximal delay bits */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E);


    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(
        devNum, portNum, portMacNum);

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].
                channelEgressPipeDelay[tsuLocalChannel];
        maxDelayBits = 30;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).txPipeStatusDelay;
        maxDelayBits = 16;
    }

    if (delay >= (GT_U32)(1 << maxDelayBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    return prvCpssDrvHwPpSetRegField(
        devNum, regAddr, 0, maxDelayBits, delay);
}

/**
* @internal cpssDxChPtpPortTxPipeStatusDelaySet function
* @endinternal
*
* @brief   Set PTP Tx Pipe Status Delay.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] delay                    -  of empty egress pipe (in core clock cycles).
*                                      (APPLICABLE RANGE: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpPortTxPipeStatusDelaySet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          delay
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpPortTxPipeStatusDelaySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, delay));

    rc = internal_cpssDxChPtpPortTxPipeStatusDelaySet(devNum, portNum, delay);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, delay));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpPortTxPipeStatusDelayGet function
* @endinternal
*
* @brief   Get PTP Tx Pipe Status Delay.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] delayPtr                 - (pointer to)delay of empty egress pipe (in core clock cycles).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpPortTxPipeStatusDelayGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *delayPtr
)
{
    GT_STATUS   rc;             /* returned code */
    GT_U32      portMacNum;     /* port MAC number */
    GT_U32      regAddr;        /* register address */
    GT_U32      tsuIndex;       /* Timestamp unit index         */
    GT_U32      tsuLocalChannel;/* Timestamp unit local channel */
    GT_U32      maxDelayBits;   /* maximal delay bits */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E);


    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(
        devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(delayPtr);

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].
                channelEgressPipeDelay[tsuLocalChannel];
        maxDelayBits = 30;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).txPipeStatusDelay;
        maxDelayBits = 16;
    }

    return prvCpssDrvHwPpGetRegField(
        devNum, regAddr, 0, maxDelayBits, delayPtr);
}

/**
* @internal cpssDxChPtpPortTxPipeStatusDelayGet function
* @endinternal
*
* @brief   Get PTP Tx Pipe Status Delay.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] delayPtr                 - (pointer to)delay of empty egress pipe (in core clock cycles).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpPortTxPipeStatusDelayGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *delayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpPortTxPipeStatusDelayGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, delayPtr));

    rc = internal_cpssDxChPtpPortTxPipeStatusDelayGet(devNum, portNum, delayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, delayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiCapturePreviousModeEnableSet function
* @endinternal
*
* @brief  Set TAI (Time Application Interface) Capture Previous Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[in] enable                   - Capture Previous Mode Enable.
*                                       GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                       GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiCapturePreviousModeEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);

    hwValue = BOOL2BIT_MAC(enable);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;
        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, 7, 1, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiCapturePreviousModeEnableSet function
* @endinternal
*
* @brief  Set TAI (Time Application Interface) Capture Previous Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[in] enable                   - Capture Previous Mode Enable.
*                                       GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                       GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiCapturePreviousModeEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiCapturePreviousModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enable));

    rc = internal_cpssDxChPtpTaiCapturePreviousModeEnableSet(devNum, taiIdPtr, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiCapturePreviousModeEnableGet function
* @endinternal
*
* @brief  Get TAI (Time Application Interface) Capture Previous Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[OUT] enable                  - (pointer to)Capture Previous Mode Enable.
*                                       GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                       GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiCapturePreviousModeEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).timeCntrFunctionConfig0;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 7, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(hwValue);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiCapturePreviousModeEnableGet function
* @endinternal
*
* @brief  Get TAI (Time Application Interface) Capture Previous Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[OUT] enable                  - (pointer to)Capture Previous Mode Enable.
*                                       GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                       GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiCapturePreviousModeEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiCapturePreviousModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, enablePtr));

    rc = internal_cpssDxChPtpTaiCapturePreviousModeEnableGet(devNum, taiIdPtr, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPClkSelectionModeSet function
* @endinternal
*
* @brief  Set the PClock Selection Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] pClkSelMode           - PClk Selection Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTaiPClkSelectionModeSet
(

    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    IN CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT    pClkSelMode
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;      /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    switch (pClkSelMode)
    {
        case CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_DISABLED_E:
            hwValue = 0;
            break;
        case CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_PCLK_E:
            hwValue = 1;
            break;
        case CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_CLOCK_GEN_E:
            hwValue = 2;
            break;
        case CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_CLOCK_RECEPTION_E:
            if (!(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
                hwValue = 3;
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockControl;
        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, 8, 2, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiPClkSelectionModeSet function
* @endinternal
*
* @brief  Set the PClock Selection Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] pClkSelMode           - PClk Selection Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiPClkSelectionModeSet
(
    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    IN CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT    pClkSelMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPClkSelectionModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, pClkSelMode));

    rc = internal_cpssDxChPtpTaiPClkSelectionModeSet(devNum, taiIdPtr, pClkSelMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, pClkSelMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiPClkSelectionModeGet function
* @endinternal
*
* @brief  Get the PClock Selection Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[OUT] pClkSelModePtr       - (pointer to) PClk Selection Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTaiPClkSelectionModeGet
(

    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT    *pClkSelModePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;      /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(pClkSelModePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockControl;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 8, 2, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (hwValue)
    {
        case 0:
            *pClkSelModePtr = CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_DISABLED_E;
            break;
        case 1:
            *pClkSelModePtr = CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_PCLK_E;
            break;
        case 2:
            *pClkSelModePtr = CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_CLOCK_GEN_E;
            break;
        case 3:
            *pClkSelModePtr = CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_CLOCK_RECEPTION_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /*never occurs*/
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiPClkSelectionModeGet function
* @endinternal
*
* @brief  Get the PClock Selection Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[OUT] pClkSelModePtr       - (pointer to) PClk Selection Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiPClkSelectionModeGet
(

    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT    *pClkSelModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiPClkSelectionModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, pClkSelModePtr));

    rc = internal_cpssDxChPtpTaiPClkSelectionModeGet(devNum, taiIdPtr, pClkSelModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, pClkSelModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiClockGenerationModeSet function
* @endinternal
*
* @brief  Set the Clock Generation Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] clockGenerationMode   - Clock Generation Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTaiClockGenerationModeSet
(

    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    IN CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT    clockGenerationMode
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;      /* HW data  */
    CPSS_DXCH_PTP_TOD_COUNT_STC        todValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    switch (clockGenerationMode)
    {
        case CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E:
            hwValue = 0;
            break;
        case CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_CLOCK_GEN_E:
            hwValue = 1;
            break;
        case CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_CLOCK_RECEPTION_E:
            if (!(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
                hwValue = 2;
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* WA to create 1 PPS for falcon */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E && hwValue == 1 )
    {
        GT_U32 unitBase = prvCpssDxChHwUnitBaseAddrGet(
            devNum, PRV_CPSS_DXCH_UNIT_CNM_RFU_E, NULL);

        todValue.nanoSeconds     = 0;
        todValue.seconds.l[0]    = 0;
        todValue.seconds.l[1]    = 0;
        todValue.fracNanoSeconds = 0;
        rc = prvCpssDrvHwPpSetRegField(
            devNum, unitBase + 0x00000108 , 13, 1, 1);
         if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPtpTaiTodSet(devNum, taiIdPtr, CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,&todValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        todValue.nanoSeconds     = 0xfffcffff;
        rc = cpssDxChPtpTaiTodSet(devNum, taiIdPtr, CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,&todValue);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPtpTaiExternalPulseWidthSet(devNum, taiIdPtr, 500000000);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPtpTaiOutputTriggerEnableSet(devNum, taiIdPtr, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPtpTaiPtpPulseInterfaceSet(devNum,CPSS_DIRECTION_INGRESS_E, taiIdPtr->taiNumber);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPtpTaiPtpPulseInterfaceSet(devNum,CPSS_DIRECTION_EGRESS_E, taiIdPtr->taiNumber);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockControl;
        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, 2, 2, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}
/**
* @internal cpssDxChPtpTaiClockGenerationModeSet function
* @endinternal
*
* @brief  Set the Clock Generation Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] clockGenerationMode   - Clock Generation Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockGenerationModeSet
(
    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    IN CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT    clockGenerationMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiClockGenerationModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, clockGenerationMode));

    rc = internal_cpssDxChPtpTaiClockGenerationModeSet(devNum, taiIdPtr, clockGenerationMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, clockGenerationMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiClockGenerationModeGet function
* @endinternal
*
* @brief  Get the Clock Generation Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number.
* @param[in] taiIdPtr               - (pointer to) TAI Units identification.
* @param[OUT] clockGenerationModePtr- (pointer to) Clock Genartion Mode.
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - on error
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_BAD_PARAM              - on wrong parameter
* @retval GT_BAD_PTR                - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTaiClockGenerationModeGet
(

    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT   *clockGenerationModePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;      /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(clockGenerationModePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockControl;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 2, 2, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (hwValue)
    {
        case 0:
            *clockGenerationModePtr = CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E;
            break;
        case 1:
            *clockGenerationModePtr = CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_CLOCK_GEN_E;
            break;
        case 2:
            *clockGenerationModePtr = CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_CLOCK_RECEPTION_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /*never occurs*/
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiClockGenerationModeGet function
* @endinternal
*
* @brief  Get the Clock Generation Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number.
* @param[in] taiIdPtr               - (pointer to) TAI Units identification.
* @param[OUT] clockGenerationModePtr- (pointer to) Clock Genartion Mode.
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - on error
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_BAD_PARAM              - on wrong parameter
* @retval GT_BAD_PTR                - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockGenerationModeGet
(

    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT   *clockGenerationModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiClockGenerationModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, clockGenerationModePtr));

    rc = internal_cpssDxChPtpTaiClockGenerationModeGet(devNum, taiIdPtr, clockGenerationModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, clockGenerationModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiClockReceptionModeSet function
* @endinternal
*
* @brief  Set the Clock Reception Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] clockReceptionMode    - Clock Reception Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTaiClockReceptionModeSet
(
    IN GT_U8                                         devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC                     *taiIdPtr,
    IN CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT    clockReceptionMode
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;      /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    switch (clockReceptionMode)
    {
        case CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_DISABLED_E:
            hwValue = 0;
            break;
        case CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_CLOCK_RECEPTION_E:
            hwValue = 1;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockControl;
        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, 0, 2, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiClockReceptionModeSet function
* @endinternal
*
* @brief  Set the Clock Reception Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] clockReceptionMode    - Clock Reception Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockReceptionModeSet
(
    IN GT_U8                                         devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC                     *taiIdPtr,
    IN CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT    clockReceptionMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiClockReceptionModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, clockReceptionMode));

    rc = internal_cpssDxChPtpTaiClockReceptionModeSet(devNum, taiIdPtr, clockReceptionMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, clockReceptionMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiClockReceptionModeGet function
* @endinternal
*
* @brief  Get the Clock Reception Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[OUT] clockReceptionModePtr- (pointer to) Clock Reception Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTaiClockReceptionModeGet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                     *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT   *clockReceptionModePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;      /* HW data  */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(clockReceptionModePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockControl;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 0, 2, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (hwValue)
    {
        case 0:
            *clockReceptionModePtr = CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_DISABLED_E;
            break;
        case 1:
            *clockReceptionModePtr = CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_CLOCK_RECEPTION_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /*never occurs*/
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiClockReceptionModeGet function
* @endinternal
*
* @brief  Get the Clock Reception Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[OUT] clockReceptionModePtr- (pointer to) Clock Reception Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockReceptionModeGet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                     *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT   *clockReceptionModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiClockReceptionModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, clockReceptionModePtr));

    rc = internal_cpssDxChPtpTaiClockReceptionModeGet(devNum, taiIdPtr, clockReceptionModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, clockReceptionModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiClockGenerationEdgeSet function
* @endinternal
*
* @brief  Set the Clock Generation Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] edge                  - Rising edge time of generation clock, in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTaiClockGenerationEdgeSet
(

    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    IN GT_U32                                  edge
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    return prvCpssDxChPtpTaiClockEdgeSet(
            devNum, taiIdPtr, edge, PRV_CPSS_DXCH_PTP_TAI_CLOCK_CONTROL_TYPE_GEN_E);
}

/**
* @internal cpssDxChPtpTaiClockGenerationEdgeSet function
* @endinternal
*
* @brief  Set the Clock Generation Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] edge                  - Rising edge time of generation clock, in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockGenerationEdgeSet
(

    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    IN GT_U32                                  edge
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiClockGenerationEdgeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, edge));

    rc = internal_cpssDxChPtpTaiClockGenerationEdgeSet(devNum, taiIdPtr, edge);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, edge));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiClockGenerationEdgeGet function
* @endinternal
*
* @brief  Get the Clock Generation Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number.
* @param[in] taiIdPtr               - (pointer to) TAI Units identification.
* @param[OUT] edgePtr               - (pointer to) Rising edge time of generation clock, in nanoseconds.
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - on error
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_BAD_PARAM              - on wrong parameter
* @retval GT_BAD_PTR                - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTaiClockGenerationEdgeGet
(

    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    OUT GT_U32                                 *edgePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(edgePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockGenerationEdge;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 0, 31, edgePtr);
    return rc;
}

/**
* @internal cpssDxChPtpTaiClockGenerationEdgeGet function
* @endinternal
*
* @brief  Get the Clock Generation Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number.
* @param[in] taiIdPtr               - (pointer to) TAI Units identification.
* @param[OUT] edgePtr               - (pointer to) Rising edge time of generation clock, in nanoseconds.
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - on error
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_BAD_PARAM              - on wrong parameter
* @retval GT_BAD_PTR                - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockGenerationEdgeGet
(

    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    OUT GT_U32                                 *edgePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiClockGenerationEdgeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, edgePtr));

    rc = internal_cpssDxChPtpTaiClockGenerationEdgeGet(devNum, taiIdPtr, edgePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, edgePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiClockReceptionEdgeSet function
* @endinternal
*
* @brief  Set the Clock Reception Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] edge                  - Rising edge time of reception clock, in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTaiClockReceptionEdgeSet
(
    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    IN GT_U32                                  edge
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    return prvCpssDxChPtpTaiClockEdgeSet(
            devNum, taiIdPtr, edge, PRV_CPSS_DXCH_PTP_TAI_CLOCK_CONTROL_TYPE_RECEPTION_E);
}

/**
* @internal cpssDxChPtpTaiClockReceptionEdgeSet function
* @endinternal
*
* @brief  Set the Clock Reception Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] edge                  - Rising edge time of reception clock, in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockReceptionEdgeSet
(
    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    IN GT_U32                                  edge
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiClockReceptionEdgeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, edge));

    rc = internal_cpssDxChPtpTaiClockReceptionEdgeSet(devNum, taiIdPtr, edge);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, edge));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiClockReceptionEdgeGet function
* @endinternal
*
* @brief  Get the Clock Reception Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[OUT] edgePtr               - (pointer to) Rising edge time of reception clock, in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTaiClockReceptionEdgeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    OUT GT_U32                                 *edgePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */
    GT_U32                             regAddr;     /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PTP_NOT_SINGLE_TAI_PTR_CHECK_MAC(devNum, taiIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(edgePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_GET_SINGLE_MAC(devNum, taiIdPtr, taiIterator);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
        devNum, taiIterator.gopNumber, taiIterator.taiNumber).clockReceptionEdge;

    rc = prvCpssDrvHwPpPortGroupGetRegField(
        devNum, taiIterator.portGroupId, regAddr, 0, 31, edgePtr);
    return rc;
}

/**
* @internal cpssDxChPtpTaiClockReceptionEdgeGet function
* @endinternal
*
* @brief  Get the Clock Reception Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[OUT] edgePtr               - (pointer to) Rising edge time of reception clock, in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockReceptionEdgeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    OUT GT_U32                                 *edgePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiClockReceptionEdgeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, edgePtr));

    rc = internal_cpssDxChPtpTaiClockReceptionEdgeGet(devNum, taiIdPtr, edgePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, edgePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsuTimeStampMaskingProfileSet function
* @endinternal
*
* @brief  Set the Timestamp Masking Profile Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - Masking Profile index.
*                                    (APPLICABLE RANGES: 1..7.)
* @param[in] maskingProfilePtr     - (pointer to) Timestamp Masking Profile Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTsuTimeStampMaskingProfileSet
(

    IN GT_U8                                            devNum,
    IN GT_PHYSICAL_PORT_NUM                             portNum,
    IN GT_U32                                           index,
    IN CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC  *maskingProfilePtr
)
{
    GT_STATUS                          rc;          /* return code */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue = 0; /* HW value */
    GT_U32                             portMacNum;  /* port MAC number */
    GT_U32                             tsuIndex;       /* Timestamp unit index         */
    GT_U32                             tsuLocalChannel;/* Timestamp unit local channel */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E);
    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    CPSS_NULL_PTR_CHECK_MAC(maskingProfilePtr);

    /* Valid Masking Profile [0:7] */
    if(index > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* numOfBits Validation */
    if((maskingProfilePtr->numOfBits > BIT_7) ||
            (maskingProfilePtr->numOfBits == 0) ||
            ((maskingProfilePtr->numOfBits % 16) != 0) ||
            (maskingProfilePtr->numOfBits < maskingProfilePtr->numOfLsbits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* numOfLsbits Validation */
    if(maskingProfilePtr->numOfLsbits > BIT_7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* numOfShifts Validation */
    if((maskingProfilePtr->numOfShifts > BIT_7) ||
            ((maskingProfilePtr->numOfBits % 8) != 0) ||
            (maskingProfilePtr->numOfShifts >= maskingProfilePtr->numOfBits))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MAC(hwValue, 0, 7, maskingProfilePtr->numOfBits);
    U32_SET_FIELD_MAC(hwValue, 8, 7, maskingProfilePtr->numOfLsbits);
    U32_SET_FIELD_MAC(hwValue, 16, 7, maskingProfilePtr->numOfShifts);

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].tsMaskingProfile[index];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).timestampMaskingProfile[index];
    }
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 23, hwValue);
}

/**
* @internal cpssDxChPtpTsuTimeStampMaskingProfileSet function
* @endinternal
*
* @brief  Set the Timestamp Masking Profile Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - Masking Profile index.
*                                    (APPLICABLE RANGES: 1..7.)
* @param[in] maskingProfilePtr     - (pointer to) Timestamp Masking Profile Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTsuTimeStampMaskingProfileSet
(
    IN GT_U8                                            devNum,
    IN GT_PHYSICAL_PORT_NUM                             portNum,
    IN GT_U32                                           index,
    IN CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC  *maskingProfilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuTimeStampMaskingProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, index, maskingProfilePtr));

    rc = internal_cpssDxChPtpTsuTimeStampMaskingProfileSet(devNum, portNum, index, maskingProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, index, maskingProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsuTimeStampMaskingProfileGet function
* @endinternal
*
* @brief  Get the Timestamp Masking profile Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - Masking Profile index.
*                                    (APPLICABLE RANGES: 1..7.)
* @param[OUT] maskingProfilePtr     - (pointer to) Timestamp Masking Profile Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTsuTimeStampMaskingProfileGet
(

    IN GT_U8                                            devNum,
    IN GT_PHYSICAL_PORT_NUM                             portNum,
    IN GT_U32                                           index,
    OUT CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC  *maskingProfilePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue = 0; /* HW value */
    GT_U32                             portMacNum;  /* port MAC number */
    GT_U32                             tsuIndex;       /* Timestamp unit index         */
    GT_U32                             tsuLocalChannel;/* Timestamp unit local channel */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E);
    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    CPSS_NULL_PTR_CHECK_MAC(maskingProfilePtr);

    /* Valid Masking Profile [0:7] */
    if(index > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].tsMaskingProfile[index];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).timestampMaskingProfile[index];
    }

    /* format */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 23, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    maskingProfilePtr->numOfBits   = U32_GET_FIELD_MAC(hwValue, 0, 7);
    maskingProfilePtr->numOfLsbits = U32_GET_FIELD_MAC(hwValue, 8, 7);
    maskingProfilePtr->numOfShifts = U32_GET_FIELD_MAC(hwValue, 16, 7);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTsuTimeStampMaskingProfileGet function
* @endinternal
*
* @brief  Get the Timestamp Masking profile Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - Masking Profile index.
*                                    (APPLICABLE RANGES: 1..7.)
* @param[OUT] maskingProfilePtr     - (pointer to) Timestamp Masking Profile Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTsuTimeStampMaskingProfileGet
(
    IN GT_U8                                            devNum,
    IN GT_PHYSICAL_PORT_NUM                             portNum,
    IN GT_U32                                           index,
    OUT CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC  *maskingProfilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuTimeStampMaskingProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, portNum, index, maskingProfilePtr));

    rc = internal_cpssDxChPtpTsuTimeStampMaskingProfileGet(devNum, portNum, index, maskingProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, index, maskingProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTsuTSFrameCounterControlSet function
* @endinternal
*
* @brief  Set the Time Stamping Frame Counter Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - port counter index.
*                                    Falcon: APPLICABLE RANGE: 0..3.
*                                    AC5P,Harrier: APPLICABLE RANGE: 0..31.
*                                    AC5X,Ironman: APPLICABLE RANGE: 0..63.)
* @param[in] frameCounterConfigPtr - (pointer to) frame counter Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTsuTSFrameCounterControlSet
(

    IN GT_U8                                            devNum,
    IN GT_PHYSICAL_PORT_NUM                             portNum,
    IN GT_U32                                           index,
    IN CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC   *frameCounterConfigPtr
)
{
    GT_STATUS                          rc;          /* return code */
    GT_U32                             regAddr = 0; /* register address */
    GT_U32                             hwData, hwValue, hwMask; /* HW value */
    GT_U32                             portMacNum;  /* port MAC number */
    GT_U32                             tsuIndex;       /* Timestamp unit index         */
    GT_U32                             tsuLocalChannel;/* Timestamp unit local channel */
    GT_U32                             frameCounterEnablePos; /* position of counter enable bit */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    CPSS_NULL_PTR_CHECK_MAC(frameCounterConfigPtr);

    hwValue = 0;
    hwMask  = 0;

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
        if (index > 63)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        if (index > 31)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    default:
        if (index > 3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        frameCounterEnablePos = 31;
    }
    else
    {
        frameCounterEnablePos = 11;
    }

    switch (frameCounterConfigPtr->format)
    {
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E:
            hwData = 0;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E:
            hwData = 1;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_Y1731_E:
            hwData = 2;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TS_E:
            hwData = 3;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_RX_E:
            hwData = 4;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TX_E:
            hwData = 5;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_WAMP_E:
            hwData = 6;
            break;
        case CPSS_DXCH_PTP_TS_PACKET_TYPE_ALL_E:
            if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                hwData = 0x7;
            else
                hwData = 0xF;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        U32_SET_FIELD_MAC(hwValue, 1, 3, hwData);
        U32_SET_FIELD_MAC(hwMask,  1, 3, 0x7);
    }
    else
    {
        U32_SET_FIELD_MAC(hwValue, 0, 4, hwData);
        U32_SET_FIELD_MAC(hwMask,  0, 4, 0xF);
    }

    switch(frameCounterConfigPtr->action)
    {
        case CPSS_DXCH_PTP_TS_ACTION_NONE_E:
            hwData = 0;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_DROP_E:
            hwData = 2;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E:
            hwData = 3;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E:
            hwData = 4;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E:
            hwData = 5;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E:
            hwData = 6;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E:
            hwData = 7;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_TIME_E:
            hwData = 8;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E:
            hwData = 9;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E:
            hwData = 10;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E:
            hwData = 4 /* passed as TS_ACTION_ADD_TIME */ /* id in TS CFG Table is 11 */;
            break;
        case CPSS_DXCH_PTP_TS_ACTION_ALL_E:
            hwData = 0xF;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(hwValue, 4, 4, hwData);
    U32_SET_FIELD_MAC(hwMask,  4, 4, 0xF);

    U32_SET_FIELD_MAC(
        hwValue, frameCounterEnablePos, 1, BOOL2BIT_MAC(frameCounterConfigPtr->enable));
    U32_SET_FIELD_MAC(hwMask, frameCounterEnablePos, 1, 1);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* Time Stamping Frame Counter Timestamp En */
        U32_SET_FIELD_MAC(hwValue, 0, 1, 1);
        U32_SET_FIELD_MAC(hwMask,  0, 1, 1);
        /* Time Stamping Frame Counter Selected Format En */
        /* Time Stamping Frame Counter Selected Action En */
        U32_SET_FIELD_MAC(hwValue, 25, 2, 3);
        U32_SET_FIELD_MAC(hwMask,  25, 2, 3);
    }

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* Global Statistics Enable */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].ctsuControl;
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 2, 1, 1);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* Channel %n Statistics Enable */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelTsControl[tsuLocalChannel];
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 4, 1, 1);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].tsFrameCounterControl[index];
    }
    else
    {
        /* clockwork warning */
        if (index <= 3)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).timestampFrameCntrControl[index];
        }
    }

    return prvCpssHwPpWriteRegBitMask(devNum, regAddr, hwMask, hwValue);
}

/**
* @internal cpssDxChPtpTsuTSFrameCounterControlSet function
* @endinternal
*
* @brief  Set the Time Stamping Frame Counter Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - port counter index.
*                                    Falcon: APPLICABLE RANGE: 0..3.
*                                    AC5P,Harrier: APPLICABLE RANGE: 0..31.
*                                    AC5X,Ironman: APPLICABLE RANGE: 0..63.)
* @param[in] frameCounterConfigPtr - (pointer to) frame counter Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTsuTSFrameCounterControlSet
(
    IN GT_U8                                            devNum,
    IN GT_PHYSICAL_PORT_NUM                             portNum,
    IN GT_U32                                           index,
    IN CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC   *frameCounterConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuTSFrameCounterControlSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, portNum, index, frameCounterConfigPtr));

    rc = internal_cpssDxChPtpTsuTSFrameCounterControlSet(devNum, portNum, index, frameCounterConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, index, frameCounterConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;

}
/**
* @internal internal_cpssDxChPtpTsuTSFrameCounterControlGet function
* @endinternal
*
* @brief  Get the Time Stamping Frame Counter Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - port counter index.
*                                    Falcon: APPLICABLE RANGE: 0..3.
*                                    AC5P,Harrier: APPLICABLE RANGE: 0..31.
*                                    AC5X,Ironman: APPLICABLE RANGE: 0..63.)
* @param[OUT] frameCounterConfigPtr- (pointer to) frame counter Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTsuTSFrameCounterControlGet
(
    IN  GT_U8                                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                             portNum,
    IN  GT_U32                                           index,
    OUT CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC   *frameCounterConfigPtr
)
{
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             regAddr = 0; /* register address */
    GT_U32                             hwData = 0; /* HW value */
    GT_U32                             hwValue = 0; /* HW value */
    GT_U32                             portMacNum;  /* port MAC number */
    GT_U32                             tsuIndex;       /* Timestamp unit index         */
    GT_U32                             tsuLocalChannel;/* Timestamp unit local channel */
    GT_U32                             frameCounterEnablePos; /* position of counter enable bit */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    CPSS_NULL_PTR_CHECK_MAC(frameCounterConfigPtr);

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
        if (index > 63)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        if (index > 31)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    default:
        if (index > 3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        frameCounterEnablePos = 31;
    }
    else
    {
        frameCounterEnablePos = 11;
    }

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].tsFrameCounterControl[index];
    }
    else
    {
        /* clockwork warning */
        if (index <= 3)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).timestampFrameCntrControl[index];
        }
    }

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        hwValue = U32_GET_FIELD_MAC(hwData, 1, 3);
    else
        hwValue = U32_GET_FIELD_MAC(hwData, 0, 4);
    switch (hwValue)
    {
        case 0:
            frameCounterConfigPtr->format = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E;
            break;
        case 1:
            frameCounterConfigPtr->format = CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E;
            break;
        case 2:
            frameCounterConfigPtr->format = CPSS_DXCH_PTP_TS_PACKET_TYPE_Y1731_E;
            break;
        case 3:
            frameCounterConfigPtr->format = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TS_E;
            break;
        case 4:
            frameCounterConfigPtr->format = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_RX_E;
            break;
        case 5:
            frameCounterConfigPtr->format = CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TX_E;
            break;
        case 6:
            frameCounterConfigPtr->format = CPSS_DXCH_PTP_TS_PACKET_TYPE_WAMP_E;
            break;
        case 7:
        case 15:
            frameCounterConfigPtr->format = CPSS_DXCH_PTP_TS_PACKET_TYPE_ALL_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG); /* on reseved value 7 */
    }

    hwValue = U32_GET_FIELD_MAC(hwData, 4, 4);
    switch (hwValue)
    {
        case 0:
            frameCounterConfigPtr->action = CPSS_DXCH_PTP_TS_ACTION_NONE_E;
            break;
        case 2:
            frameCounterConfigPtr->action = CPSS_DXCH_PTP_TS_ACTION_DROP_E;
            break;
        case 3:
            frameCounterConfigPtr->action = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E;
            break;
        case 4:
            frameCounterConfigPtr->action = CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E;
            break;
        case 5:
            frameCounterConfigPtr->action = CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E;
            break;
        case 6:
            frameCounterConfigPtr->action = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E;
            break;
        case 7:
            frameCounterConfigPtr->action = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E;
            break;
        case 8:
            frameCounterConfigPtr->action = CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_TIME_E;
            break;
        case 9:
            frameCounterConfigPtr->action = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E;
            break;
        case 10:
            frameCounterConfigPtr->action = CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E;
            break;
        case 11:
            frameCounterConfigPtr->action = CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E;
            break;
        case 15:
            frameCounterConfigPtr->action = CPSS_DXCH_PTP_TS_ACTION_ALL_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    frameCounterConfigPtr->enable = (GT_BOOL)U32_GET_FIELD_MAC(hwData, frameCounterEnablePos, 1);
    return GT_OK;
}

/**
* @internal cpssDxChPtpTsuTSFrameCounterControlGet function
* @endinternal
*
* @brief  Get the Time Stamping Frame Counter Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - port counter index.
*                                    Falcon: APPLICABLE RANGE: 0..3.
*                                    AC5P,Harrier: APPLICABLE RANGE: 0..31.
*                                    AC5X,Ironman: APPLICABLE RANGE: 0..63.)
* @param[OUT] frameCounterConfigPtr- (pointer to) frame counter Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTsuTSFrameCounterControlGet
(
    IN  GT_U8                                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                             portNum,
    IN  GT_U32                                           index,
    OUT CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC   *frameCounterConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuTSFrameCounterControlGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, portNum, index, frameCounterConfigPtr));

    rc = internal_cpssDxChPtpTsuTSFrameCounterControlGet(devNum, portNum, index, frameCounterConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, index, frameCounterConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpTsuTsFrameCounterIndexSet function
* @endinternal
*
* @brief  Set the TSU Time Stamping Frame Counter value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - port counter index.
*                                    Falcon: APPLICABLE RANGE: 0..3.
*                                    AC5P,Harrier: APPLICABLE RANGE: 0..31.
*                                    AC5X,Ironman: APPLICABLE RANGE: 0..63.)
* @param[in] frameCounter          - Time Frame Counter Value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChPtpTsuTsFrameCounterIndexSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  index,
    IN  GT_U32                                  frameCounter
)
{
    GT_STATUS                          rc;          /* return code */
    GT_U32                             regAddr = 0; /* register address */
    GT_U32                             portMacNum;  /* port MAC number */
    GT_U32                             tsuIndex;       /* Timestamp unit index         */
    GT_U32                             tsuLocalChannel;/* Timestamp unit local channel */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
    case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
        if (index > 63)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        if (index > 31)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    default:
        if (index > 3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    }

    tsuIndex        = 0;
    tsuLocalChannel = 0;
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].tsFrameCounter[index];
    }
    else
    {
        /* clockwork warning */
        if (index <= 3) {
            regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).timestampFrameCntr[index];
        }
    }
    return prvCpssHwPpWriteRegister(devNum, regAddr, frameCounter);
}

/**
* @internal cpssDxChPtpTsuTsFrameCounterIndexSet function
* @endinternal
*
* @brief  Set the TSU Time Stamping Frame Counter value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - port counter index.
* @param[in] index                 - port counter index.
*                                    Falcon: APPLICABLE RANGE: 0..3.
*                                    AC5P,Harrier: APPLICABLE RANGE: 0..31.
*                                    AC5X,Ironman: APPLICABLE RANGE: 0..63.)
* @param[in] frameCounterPtr      - Time Frame Counter Value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTsuTsFrameCounterIndexSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  index,
    IN  GT_U32                                  frameCounter
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTsuTsFrameCounterIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, portNum, index, frameCounter));

    rc = internal_cpssDxChPtpTsuTsFrameCounterIndexSet(devNum, portNum, index, frameCounter);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, index, frameCounter));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpUBitModeSet function
* @endinternal
*
* @brief  Set U-bit Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[OUT] uBitMode                - U-bit mode to set.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS internal_cpssDxChPtpUBitModeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                 *taiIdPtr,
    IN  CPSS_DXCH_PTP_U_BIT_MODE_ENT             uBitMode
)
{
    GT_STATUS                          rc;
    GT_U32                             regAddr;     /* register address */
    GT_U32                             value;
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator; /* TAI iterator     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        switch (uBitMode)
        {
            case CPSS_DXCH_PTP_TAI_U_BIT_MODE_ASSIGNED_E:
                value = 0;
                break;

            case CPSS_DXCH_PTP_TAI_U_BIT_MODE_COUNTER_E:
                value = 1;
                break;

            case CPSS_DXCH_PTP_TAI_U_BIT_MODE_PROGRAMMED_E:
                value = 2;
                break;
            default:
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).TAICtrlReg0;

        rc = prvCpssDrvHwPpPortGroupSetRegField(
            devNum, taiIterator.portGroupId, regAddr, 9, 2, value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)

    return GT_OK;
}

/**
* @internal cpssDxChPtpUBitModeSet function
* @endinternal
*
* @brief  Set U-bit Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[OUT] uBitMode                - U-bit mode to set.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssDxChPtpUBitModeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                 *taiIdPtr,
    IN  CPSS_DXCH_PTP_U_BIT_MODE_ENT             uBitMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpUBitModeSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, uBitMode));

    rc = internal_cpssDxChPtpUBitModeSet(devNum, taiIdPtr, uBitMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, uBitMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpUBitModeGet function
* @endinternal
*
* @brief  Get U-bit Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[OUT] uBitModePtr                  - (pointer to)Capture U-bit mode.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChPtpUBitModeGet
(
    IN   GT_U8                                    devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                  *taiIdPtr,
    OUT  CPSS_DXCH_PTP_U_BIT_MODE_ENT             *uBitModePtr
)
{
    GT_STATUS                          rc;
    GT_U32                             regAddr;     /* register address */
    GT_U32                             value;
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC taiIterator;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(uBitModePtr);

    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_START_MAC(devNum, taiIdPtr, taiIterator)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(
            devNum, taiIterator.gopNumber, taiIterator.taiNumber).TAICtrlReg0;
        rc = prvCpssDrvHwPpPortGroupGetRegField(
            devNum, taiIterator.portGroupId, regAddr, 9, 2, &value);
        if (rc != GT_OK)
        {
            return rc;
        }

        switch (value)
        {
            case 0:
                *uBitModePtr = CPSS_DXCH_PTP_TAI_U_BIT_MODE_ASSIGNED_E ;
                break;

            case 1:
                *uBitModePtr = CPSS_DXCH_PTP_TAI_U_BIT_MODE_COUNTER_E;
                break;

            case 2:
                *uBitModePtr = CPSS_DXCH_PTP_TAI_U_BIT_MODE_PROGRAMMED_E;
                break;
            default:
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_LOOP_END_MAC(devNum, taiIdPtr, taiIterator)
    return GT_OK;
}

/**
* @internal cpssDxChPtpUBitModeGet function
* @endinternal
*
* @brief  Get U-bit Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[OUT] uBitModePtr                  - (pointer to)Capture U-bit mode.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpUBitModeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                 *taiIdPtr,
    OUT  CPSS_DXCH_PTP_U_BIT_MODE_ENT            *uBitModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpUBitModeGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiIdPtr, uBitModePtr));

    rc = internal_cpssDxChPtpUBitModeGet(devNum, taiIdPtr, uBitModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiIdPtr, uBitModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChPtpPLLBypassEnableSet function
* @endinternal
*
* @brief   Enable/Disable PLL bypass.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Tx, Rx or both directions
* @param[in] enable                   - GT_TRUE - PLL is in bypass mode.
*                                      GT_FALSE - PLL in normal mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS internal_cpssDxChPtpPLLBypassEnableSet
(
    IN  GT_U8           devNum,
    IN  GT_BOOL         enable
)
{
    GT_U32    regAddr;
    GT_U32    hwData,offset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E);

    hwData = BOOL2BIT_MAC(enable);
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnitsDeviceSpecificRegs.pllPTPConfig;

    offset = PRV_CPSS_SIP_6_20_CHECK_MAC(devNum) ? 23 : 0;

    return  prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, offset, 1, hwData);
}
/**
* @internal cpssDxChPtpPLLBypassEnableSet function
* @endinternal
*
* @brief   Enable/Disable PLL bypass.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Tx, Rx or both directions
* @param[in] enable                   - GT_TRUE - PLL is in bypass mode.
*                                      GT_FALSE - PLL in normal mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpPLLBypassEnableSet
(
    IN  GT_U8           devNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpPLLBypassEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPtpPLLBypassEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpPLLBypassEnableGet function
* @endinternal
*
* @brief   Get PLL bypass status (enable/disable).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] enablePtr                - (Pointer to) GT_TRUE - PLL is in bypass mode.
*                                                    GT_FALSE - PLL in normal mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChPtpPLLBypassEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    hwData,offset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                DFXServerUnitsDeviceSpecificRegs.pllPTPConfig;

    offset = PRV_CPSS_SIP_6_20_CHECK_MAC(devNum) ? 23 : 0;

    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr, offset, 1, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(hwData);
    return GT_OK;

}

/**
* @internal cpssDxChPtpPLLBypassEnableGet function
* @endinternal
*
* @brief   Get PLL bypass status (enable/disable).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] enablePtr                - (Pointer to) GT_TRUE - PLL is in bypass mode.
*                                                    GT_FALSE - PLL in normal mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpPLLBypassEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpPLLBypassEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPtpPLLBypassEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpRefClockSourceSelectSet function
* @endinternal
*
* @brief   Configures reference clock source of PTP PLL.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] refClockSelect           - reference clock source selection mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpRefClockSourceSelectSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT  refClockSelect
)
{
    GT_STATUS rc;
    GT_U32    regAddr,value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AC5P_E);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    switch (refClockSelect)
    {
        case  CPSS_DXCH_PTP_REF_CLOCK_SELECT_MAIN_25_MHZ_E:
            value = 0;
            break;

        case CPSS_DXCH_PTP_REF_CLOCK_SELECT_DEDICATED_PTP_REF_CLK_E:
            value = 4;
            break;

        case CPSS_DXCH_PTP_REF_CLOCK_SELECT_SD_DIFFRENTIAL_REF_CLK_E:
            value = 6;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl29;

    rc =  prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 13, 3, value);

    return rc;
}


/**
* @internal cpssDxChPtpRefClockSourceSelectSet function
* @endinternal
*
* @brief   Configures reference clock source of PTP PLL.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] refClockSelect           - reference clock source selection mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpRefClockSourceSelectSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT  refClockSelect
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpRefClockSourceSelectSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, refClockSelect));

    rc = internal_cpssDxChPtpRefClockSourceSelectSet(devNum, refClockSelect);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, refClockSelect));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpRefClockSourceSelectGet function
* @endinternal
*
* @brief   Get reference clock source of PTP PLL.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[out] refClockSelectPtr       - (pointer to) reference clock source selection mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpRefClockSourceSelectGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT  *refClockSelectPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr,value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AC5P_E);
    CPSS_NULL_PTR_CHECK_MAC(refClockSelectPtr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl29;

    rc =  prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum, regAddr, 13, 3, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (value)
    {
        case  0:
            *refClockSelectPtr = CPSS_DXCH_PTP_REF_CLOCK_SELECT_MAIN_25_MHZ_E;
            break;

        case 4:
            *refClockSelectPtr = CPSS_DXCH_PTP_REF_CLOCK_SELECT_DEDICATED_PTP_REF_CLK_E;
            break;

        case 6:
            *refClockSelectPtr = CPSS_DXCH_PTP_REF_CLOCK_SELECT_SD_DIFFRENTIAL_REF_CLK_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpRefClockSourceSelectGet function
* @endinternal
*
* @brief   Get reference clock source of PTP PLL.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[out] refClockSelectPtr       - (pointer to) reference clock source selection mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpRefClockSourceSelectGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT  *refClockSelectPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpRefClockSourceSelectGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, refClockSelectPtr));

    rc = internal_cpssDxChPtpRefClockSourceSelectGet(devNum, refClockSelectPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, refClockSelectPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpNonPtpPacketTaiSelectSet function
* @endinternal
*
* @brief   Set TAI select in case packet is not a PTP packet.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - physical device number
* @param[in] taiSelect                - select TAI for timestamping for non PTP packets
*                                       (APPLICABLE RANGES: 0..4.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpNonPtpPacketTaiSelectSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            taiSelect
)
{
    GT_STATUS rc;
    GT_U32  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    if (taiSelect >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt3;
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 3, taiSelect);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* taiSelect should be configured the same as <TSN TAI select> in the DFX server (to apply for TSN Qch logic) */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl50;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 11, 3, taiSelect);
    return rc;
}

/**
* @internal cpssDxChPtpNonPtpPacketTaiSelectSet function
* @endinternal
*
* @brief   Set TAI select in case packet is not a PTP packet.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - physical device number
* @param[in] taiSelect                - select TAI for timestamping for non PTP packets
*                                       (APPLICABLE RANGES: 0..4.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpNonPtpPacketTaiSelectSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            taiSelect
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpNonPtpPacketTaiSelectSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiSelect));

    rc = internal_cpssDxChPtpNonPtpPacketTaiSelectSet(devNum, taiSelect);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiSelect));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpNonPtpPacketTaiSelectGet function
* @endinternal
*
* @brief   Get TAI select in case packet is not a PTP packet.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - physical device number
* @param[out] taiSelectPtr            - (pointer to) select TAI for timestamping for non PTP packets
*                                       (APPLICABLE RANGES: 0..4.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpNonPtpPacketTaiSelectGet
(
    IN  GT_U8                             devNum,
    OUT GT_U32                            *taiSelectPtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(taiSelectPtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt3;
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 3, taiSelectPtr);
    return rc;
}

/**
* @internal cpssDxChPtpNonPtpPacketTaiSelectGet function
* @endinternal
*
* @brief   Get TAI select in case packet is not a PTP packet.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - physical device number
* @param[out] taiSelectPtr            - (pointer to) select TAI for timestamping for non PTP packets
*                                       (APPLICABLE RANGES: 0..4.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpNonPtpPacketTaiSelectGet
(
    IN  GT_U8                             devNum,
    OUT GT_U32                            *taiSelectPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpNonPtpPacketTaiSelectGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiSelectPtr));

    rc = internal_cpssDxChPtpNonPtpPacketTaiSelectGet(devNum, taiSelectPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiSelectPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChPtpTaiOutputInterfaceSet function
* @endinternal
*
* @brief   Set TAI Output interface configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] pinId                    - pin identification.
*                                       (APPLICABLE VALUES: A,B,C).
* @param[in] interfaceType            - interface type.
* @param[in] taiSelect                - select TAI for driving the output signal.
*                                       (APPLICABLE RANGES: 0..4.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPtpTaiOutputInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT        pinId,
    IN  CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT    interfaceType,
    IN  GT_U32                                  taiSelect
)
{
    GT_STATUS rc;
    GT_U32  regAddr,regAddrEn=GT_FALSE;
    GT_U32  offset,offsetEn=GT_FALSE,value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    if (taiSelect >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    switch (interfaceType)
    {
    case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_CLOCK_E:
        regAddrEn = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl49;
        switch (pinId)
        {
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E:
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl48;
            offset = 9;
            offsetEn = 3;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E:
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl48;
            offset = 12;
            offsetEn = 4;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E:
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl49;
            offset = 0;
            offsetEn = 5;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;

    case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PULSE_E:
        regAddrEn = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl49;
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl49;
        switch (pinId)
        {
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E:
            offset = 6;
            offsetEn = 15;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E:
            offset = 9;
            offsetEn = 16;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E:
            offset = 12;
            offsetEn = 17;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;

    case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PCLK_E:
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl48;
        switch (pinId)
        {
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E:
            offset = 0;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E:
            offset = 3;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E:
            offset = 6;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, offset, 3, taiSelect);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* PCLK is only outout signal */
    if ((interfaceType == CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_CLOCK_E) || (interfaceType == CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PULSE_E))
    {
        /* Output signal enable */
        rc = prvCpssHwPpSetRegField(devNum, regAddrEn, offsetEn, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* MPP select */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_MPP_MAC(devNum).MPPControl24_31;
    switch (pinId)
    {
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E:
        offset = 12;
        break;
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E:
        offset = 16;
        break;
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E:
        offset = 20;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* MPP[i]_Sel: PTP_PULSE_A (inout)=1; PTP_CLK_A (inout)=2; PTP_PCLK_A (out)=3 */
    switch (interfaceType)
    {
     case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PULSE_E:
         value = 1;
         break;
     case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_CLOCK_E:
         value = 2;
         break;
     case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PCLK_E:
         value = 3;
         break;
     default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr,offset,4,value);
    return rc;
}
/**
* @internal cpssDxChPtpTaiOutputInterfaceSet function
* @endinternal
*
* @brief   Set TAI Output interface configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] pinId                    - pin identification.
*                                       (APPLICABLE VALUES: A,B,C).
* @param[in] interfaceType            - interface type.
* @param[in] taiSelect                - select TAI for driving the output signal.
*                                       (APPLICABLE RANGES: 0..4.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiOutputInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT        pinId,
    IN  CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT    interfaceType,
    IN  GT_U32                                  taiSelect
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiOutputInterfaceSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pinId, interfaceType, taiSelect));

    rc = internal_cpssDxChPtpTaiOutputInterfaceSet(devNum, pinId, interfaceType, taiSelect);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pinId, interfaceType, taiSelect));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChPtpTaiOutputInterfaceGet function
* @endinternal
*
* @brief   Get TAI Output interface configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] pinId                    - pin identification.
*                                       (APPLICABLE VALUES: A,B,C).
* @param[in] interfaceType            - interface type.
* @param[in] taiSelectPtr             - (pointer to) select TAI for driving the output signal.
*                                       (APPLICABLE RANGES: 0..4.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPtpTaiOutputInterfaceGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT        pinId,
    IN  CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT    interfaceType,
    OUT GT_U32                                  *taiSelectPtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  offset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(taiSelectPtr);

    switch (interfaceType)
    {
    case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_CLOCK_E:
        switch (pinId)
        {
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E:
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl48;
            offset = 9;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E:
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl48;
            offset = 12;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E:
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl49;
            offset = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;

    case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PULSE_E:
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl49;
        switch (pinId)
        {
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E:
            offset = 6;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E:
            offset = 9;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E:
            offset = 12;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;

    case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PCLK_E:
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl48;
        switch (pinId)
        {
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E:
            offset = 0;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E:
            offset = 3;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E:
            offset = 6;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, offset, 3, taiSelectPtr);
    return rc;
}
/**
* @internal cpssDxChPtpTaiOutputInterfaceGet function
* @endinternal
*
* @brief   Get TAI Output interface configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] pinId                    - pin identification.
*                                       (APPLICABLE VALUES: A,B,C).
* @param[in] interfaceType            - interface type.
* @param[in] taiSelectPtr             - (pointer to) select TAI for driving the output signal.
*                                       (APPLICABLE RANGES: 0..4.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiOutputInterfaceGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT        pinId,
    IN  CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT    interfaceType,
    OUT GT_U32                                  *taiSelectPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiOutputInterfaceGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pinId, interfaceType, taiSelectPtr));

    rc = internal_cpssDxChPtpTaiOutputInterfaceGet(devNum, pinId, interfaceType, taiSelectPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pinId, interfaceType, taiSelectPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiInputInterfaceSet function
* @endinternal
*
* @brief   Set TAI Input interface configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] pinId                    - pin identification.
*                                       (APPLICABLE VALUES: A,B,C are valid when interfaceType == CLOCK/PULSE;.
*                                                           NONE is valid when interfaceType == CLOCK).
* @param[in] interfaceType            - interface type.
*                                       (APPLICABLE VALUES: CLOCK, PULSE).
* @param[in] taiSelectBmp             - bitmap of selected TAIs for receiving the input signal.
*                                       1 - enable, 0 - disable. (APPLICABLE RANGES: 0..0x1F.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPtpTaiInputInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT        pinId,
    IN  CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT    interfaceType,
    IN  GT_TAI_SELECT_BMP                       taiSelectBmp
)
{
    GT_STATUS rc;
    GT_U32  regAddr,regAddrEn=GT_FALSE;
    GT_U32  offset,offsetEn=GT_FALSE,value,i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_TAI_SELECTION_BMP_CHECK_MAC(devNum,taiSelectBmp);

    switch (interfaceType)
    {
    case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_CLOCK_E:
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl49;
        regAddrEn = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl49;
        offset = 18;
        switch (pinId)
        {
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E:
            offsetEn = 3;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E:
            offsetEn = 4;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E:
            offsetEn = 5;
            break;
        default:
            /* do nothing */
            break;
        }
        break;

    case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PULSE_E:
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl50;
        regAddrEn = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl49;
        offset = 0;
        switch (pinId)
        {
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E:
            offsetEn = 15;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E:
            offsetEn = 16;
            break;
        case CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E:
            offsetEn = 17;
            break;
        default:
            /* do nothing */
            break;
        }
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (pinId)
    {
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E:
        value = 0;
        break;
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E:
        value = 1;
        break;
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E:
        value = 2;
        break;
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_NONE_E:
        if (interfaceType != CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_CLOCK_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        value = 3;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* an input could drive more than 1 TAI at once */
    for (i=0; i<PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais; i++)
    {
        if (((taiSelectBmp >> i) & 0x1) == 1)
        {
            rc = prvCpssHwPpSetRegField(devNum, regAddr, offset+i*2, 2, value);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* PCLK is only outout signal */
    if ((pinId == CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E) || (pinId == CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E) || (pinId == CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E))
    {
        /* Input signal enable */
        rc = prvCpssHwPpSetRegField(devNum, regAddrEn, offsetEn, 1, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* MPP select */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_MPP_MAC(devNum).MPPControl24_31;
    switch (pinId)
    {
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E:
        offset = 12;
        break;
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E:
        offset = 16;
        break;
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E:
        offset = 20;
        break;
    default:
        /* do nothing */
        return GT_OK;
    }

    /* MPP[i]_Sel: PTP_PULSE_A (inout)=1; PTP_CLK_A (inout)=2; PTP_PCLK_A (out)=3 */
    switch (interfaceType)
    {
     case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PULSE_E:
         value = 1;
         break;
     case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_CLOCK_E:
         value = 2;
         break;
     case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PCLK_E:
         value = 3;
         break;
     default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr,offset,4,value);
    return rc;
}

/**
* @internal cpssDxChPtpTaiInputInterfaceSet function
* @endinternal
*
* @brief   Set TAI Input interface configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] pinId                    - pin identification.
*                                       (APPLICABLE VALUES: A,B,C are valid when interfaceType == CLOCK/PULSE;.
*                                                           NONE is valid when interfaceType == CLOCK).
* @param[in] interfaceType            - interface type.
*                                       (APPLICABLE VALUES: CLOCK, PULSE).
* @param[in] taiSelectBmp             - bitmap of selected TAIs for receiving the input signal.
*                                       1 - enable, 0 - disable. (APPLICABLE RANGES: 0..0x1F.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiInputInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT        pinId,
    IN  CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT    interfaceType,
    IN  GT_TAI_SELECT_BMP                       taiSelectBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiInputInterfaceSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pinId, interfaceType, taiSelectBmp));

    rc = internal_cpssDxChPtpTaiInputInterfaceSet(devNum, pinId, interfaceType, taiSelectBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pinId, interfaceType, taiSelectBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiInputInterfaceGet function
* @endinternal
*
* @brief   Get TAI Input interface configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] pinId                    - pin identification.
*                                       (APPLICABLE VALUES: A,B,C are valid when interfaceType == CLOCK/PULSE;.
*                                                           NONE is valid when interfaceType == CLOCK).
* @param[in] interfaceType            - interface type.
*                                       (APPLICABLE VALUES: CLOCK, PULSE).
* @param[out] taiSelectBmpPtr         - (pointer to) bitmap of selected TAIs for receiving the input signal.
*                                       1 - enable, 0 - disable. (APPLICABLE RANGES: 0..0x1F.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPtpTaiInputInterfaceGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT        pinId,
    IN  CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT    interfaceType,
    OUT GT_TAI_SELECT_BMP                       *taiSelectBmpPtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  offset,value,i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(taiSelectBmpPtr);

    switch (interfaceType)
    {
    case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_CLOCK_E:
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl49;
        offset = 18;
        break;
    case CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PULSE_E:
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl50;
        offset = 0;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (pinId)
    {
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E:
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E:
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E:
        break;
    case CPSS_DXCH_PTP_TAI_PIN_SELECT_NONE_E:
        if (interfaceType != CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_CLOCK_E)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    *taiSelectBmpPtr = 0;
    /* an input could drive more than 1 TAI at once */
    for (i=0; i<PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais; i++)
    {
        rc = prvCpssHwPpGetRegField(devNum, regAddr, offset+i*2, 2, &value);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (value == pinId)
        {
            *taiSelectBmpPtr |= (0x1 << i);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiInputInterfaceGet function
* @endinternal
*
* @brief   Get TAI Input interface configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] pinId                    - pin identification.
*                                       (APPLICABLE VALUES: A,B,C are valid when interfaceType == CLOCK/PULSE;.
*                                                           NONE is valid when interfaceType == CLOCK).
* @param[in] interfaceType            - interface type.
*                                       (APPLICABLE VALUES: CLOCK, PULSE).
* @param[out] taiSelectBmpPtr         - (pointer to) bitmap of selected TAIs for receiving the input signal.
*                                       1 - enable, 0 - disable. (APPLICABLE RANGES: 0..0x1F.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiInputInterfaceGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT        pinId,
    IN  CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT    interfaceType,
    OUT GT_TAI_SELECT_BMP                       *taiSelectBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiInputInterfaceGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pinId, interfaceType, taiSelectBmpPtr));

    rc = internal_cpssDxChPtpTaiInputInterfaceGet(devNum, pinId, interfaceType, taiSelectBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pinId, interfaceType, taiSelectBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet function
* @endinternal
*
* @brief   Triggeres TOD update operation for selected TAI units.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] taiSelectBmp             - bitmap of selected TAIs for triggering TOD operation.
*                                       1 - enable, 0 - disable. (APPLICABLE RANGES: 0..0x1F.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet
(
    IN GT_U8                                    devNum,
    IN GT_TAI_SELECT_BMP                        taiSelectBmp
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  value=0,i;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PTP_TAI_SELECTION_BMP_CHECK_MAC(devNum,taiSelectBmp);

    /* Global pulse software trigger to selected TAIs.
       To apply a trigger to multiple TAIs at once, set the GLOBAL_PULSE option in all required TAIs */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl50;

    /* an input could drive more than 1 TAI at once */
    for (i=0; i<PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais; i++)
    {
        if (((taiSelectBmp >> i) & 0x1) == 1)
        {
            value |= 3 << i*2;
        }
    }

    /* select all TAIs for triggering */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 10, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set trigger bit */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 10, 1, 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* unset trigger bit */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 10, 1, 0);
    return rc;

}

/**
* @internal cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet function
* @endinternal
*
* @brief   Triggeres TOD update operation for selected TAI units.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] taiSelectBmp             - bitmap of selected TAIs for triggering TOD operation.
*                                       1 - enable, 0 - disable. (APPLICABLE RANGES: 0..0x1F.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet
(
    IN GT_U8                                    devNum,
    IN GT_TAI_SELECT_BMP                        taiSelectBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiSelectBmp));

    rc = internal_cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet(devNum, taiSelectBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiSelectBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet function
* @endinternal
*
* @brief   Get selected TAI units for triggering TOD update operation.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
*
* @param[out] taiSelectBmpPtr         - (pointer to) bitmap of selected TAIs for triggering TOD operation.
*                                       1 - enable, 0 - disable. (APPLICABLE RANGES: 0..0x1F.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet
(
    IN  GT_U8                                   devNum,
    OUT GT_TAI_SELECT_BMP                       *taiSelectBmpPtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  value,i;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(taiSelectBmpPtr);

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl50;

    *taiSelectBmpPtr = 0;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 10, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* an input could drive more than 1 TAI at once */
    for (i=0; i<PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais; i++)
    {
        if (((value >> i*2) & 0x3) == 3 /*GLOBAL_PULSE*/)
        {
            *taiSelectBmpPtr |= (0x1 << i);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet function
* @endinternal
*
* @brief   Get selected TAI units for triggering TOD update operation.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
*
* @param[out] taiSelectBmpPtr         - (pointer to) bitmap of selected TAIs for triggering TOD operation.
*                                       1 - enable, 0 - disable. (APPLICABLE RANGES: 0..0x1F.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet
(
    IN  GT_U8                                   devNum,
    OUT GT_TAI_SELECT_BMP                       *taiSelectBmpPtr
)
{

    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, taiSelectBmpPtr));

    rc = internal_cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet(devNum, taiSelectBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, taiSelectBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPtpPortTaiInstanceGet function
* @endinternal
*
* @brief   Get TAI Instance per port number.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[out] taiInstancePtr          - (pointer to) tai instance
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPtpPortTaiInstanceGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT GT_U32                            *taiInstancePtr
)
{
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SPEED_ENT speed;
    MV_HWS_PORT_STANDARD hwsIfMode;
    GT_BOOL isUsx;
    GT_U32  ravenNum,localRaven,tileNum,portMacNum;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(taiInstancePtr);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);


    if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        /* Harrier: Port --> SERDES (DP)--> TAI Instance
                    ------------------------------------
                     0..14    0..7  (DP0)  400G0
                    16..30    8..15 (DP1)  400G1
                    32..38   16..19 (DP2)  200G2

            NOTE: the following TAI instances are not mapped to port.
                  TTI
                  PHA
                  EPCL_HA
                  PCA0
                  PCA1
                  PCA2
                  TXQS
                  TXQS1
        */
        if (portNum < 16)
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_400G0_TAI0_E;
        else if (portNum < 32)
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_400G1_TAI0_E;
        else if (portNum <= 38)
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_200G2_TAI0_E;
        else
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong port number");
    }
    else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        /* AC5X: Port --> SERDES --> TAI Instance
                 ------------------------------------
                 0..15    0..1       USX2_0
                 16..31   2..3       USX2_1
                 32..47   4..5       MAC_CPUM (USX)
                 48..49   6..7       MAC_CPUC
                 50..53   8..11      100G

            NOTE: the following TAI instances are not mapped to port.
                  TTI
                  PHA
                  EPCL_HA
                  PCA0
                  TXQS
        */
        if (portNum < 16)
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_USX2_0_TAI0_E;
        else if (portNum < 32)
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_USX2_1_TAI0_E;
        else if (portNum < 48)
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_MAC_CPUM_TAI0_E;
        else if (portNum == 48 || portNum == 49)
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_MAC_CPUC_TAI0_E;
        else if (portNum <= 53)
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_100G_TAI0_E;
        else
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong port number");
    }
    else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* AC5P: Port --> SERDES (DP)--> TAI Instance
                 ------------------------------------
                 0..1     0..1   (DP0)  400G0
                 2..25    2..7   (DP0)  400G0/USX0
                 26..27   8..9   (DP1)  400G1
                 28..51   10..15 (DP1)  400G1/USX1
                 52..77   16..23 (DP2)  400G2
                 78..101  24..31 (DP3)  400G3
                 105      CPU    (DP1)  CPU_PORT

            NOTE: the following TAI instances are not mapped to port.
                  TXQS
                  TXQS1
                  PHA
                  EPCL
                  PCA0
                  PCA1
                  PCA2
                  PCA3
        */
        rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&ifMode);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChPortSpeedGet(devNum,portNum,&speed);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum),ifMode,speed,&hwsIfMode);
        if (rc != GT_OK)
        {
            return rc;
        }
        isUsx = mvHwsUsxModeCheck(devNum, portNum, hwsIfMode);

        if (portNum < 2)
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G0_TAI0_E;
        else if (portNum < 26)
        {
            if (isUsx)
                *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_USX0_TAI0_E;
            else
                *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G0_TAI0_E;
        }
        else if (portNum < 28)
        {
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G1_TAI0_E;
        }
        else if (portNum < 52)
        {
            if (isUsx)
                *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_USX1_TAI0_E;
            else
                *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G1_TAI0_E;
        }
        else if (portNum < 78)
        {
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G2_TAI0_E;
        }
        else if (portNum < 102)
        {
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G3_TAI0_E;
        }
        else if (portNum == 105 && mvHwsMtipIsReducedPort(devNum, portNum))
        {
            *taiInstancePtr = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_CPU_PORT_TAI0_E;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong port number");
        }
    }
    else
    {
        /* Falcon: Port --> SERDES    (Tile)  (Raven) --> TAI Instance
                   ---------------------------------------------------
                   0..15     0..15    (Tile0) (Raven0)   GOP_0
                   16..31    16..31   (Tile0) (Raven1)   GOP_1
                   32..47    32..47   (Tile0) (Raven2)   GOP_2
                   48..63    48..63   (Tile0) (Raven3)   GOP_3

                   64..79    64..79   (Tile1) (Raven4)   GOP_4
                   80..95    80..95   (Tile1) (Raven5)   GOP_5
                   96..111   96..111  (Tile1) (Raven6)   GOP_6
                   112..127  112..127 (Tile1) (Raven7)   GOP_7

                   128..143  128..143 (Tile2) (Raven8)   GOP_8
                   144..159  144..159 (Tile2) (Raven9)   GOP_9
                   160..175  160..175 (Tile2) (Raven10)  GOP_10
                   176..191  176..191 (Tile2) (Raven11)  GOP_11

                   192..207  192..207 (Tile3) (Raven12)  GOP_12
                   208..223  208..223 (Tile3) (Raven13)  GOP_13
                   224..239  224..239 (Tile3) (Raven14)  GOP_14
                   240..255  240..255 (Tile3) (Raven15)  GOP_15

            NOTE: the following TAI instances are not mapped to port.
                  CP_PIPE0
                  CP_PIPE1
                  TXQ_PIPE0,
                  TXQ_PIPE1
        */

        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

        prvCpssDxChFalconPortMacDeviceMapGet(devNum, portMacNum, &ravenNum, &localRaven, &tileNum);

        *taiInstancePtr = (PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_0_E + localRaven) + 9 * tileNum;

    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPtpPortTaiSerRxFrameLatencyDbgGet function
* @endinternal
*
* @brief   Debug API: Get SER-TX to SER-RX latency per
*          {port,taiNumber}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] taiNumber                - TAI number selection.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_NUMBER_0_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_1_E)
* @param[out] nanoSecondsPtr          - (pointer to) nano seconds part of SER-RX frame latency.
* @param[out] fracNanoSecondsPtr      - (pointer to) frac nano seconds part of SER-RX frame latency.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiSerRxFrameLatencyDbgGet
(
  IN  GT_U8                             devNum,
  IN  GT_PHYSICAL_PORT_NUM              portNum,
  IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT      taiNumber,
  OUT GT_U32                            *nanoSecondsPtr,
  OUT GT_U32                            *fracNanoSecondsPtr
)
{
    GT_STATUS                          rc;
    GT_U32                             taiInstance;
    GT_U32                             regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(nanoSecondsPtr);
    CPSS_NULL_PTR_CHECK_MAC(fracNanoSecondsPtr);
    PRV_CPSS_DXCH_TAI_CHECK_MAC(devNum,taiNumber);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* convert from portNum to TAI instance */
    rc = prvCpssDxChPtpPortTaiInstanceGet(devNum,portNum,&taiInstance);
    if (rc != GT_OK)
    {
        return rc;
    }
    regAddr = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).frameLatency;

    rc = prvCpssHwPpGetRegField(devNum, regAddr , 0, 16, nanoSecondsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,24, 8, fracNanoSecondsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpPortTaiSerRxFrameLatencyDbgGet function
* @endinternal
*
* @brief   Debug API: Get SER-TX to SER-RX latency per
*          {port,taiNumber}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] taiNumber                - TAI number selection.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_NUMBER_0_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_1_E)
* @param[out] nanoSecondsPtr          - (pointer to) nano seconds part of SER-RX frame latency.
* @param[out] fracNanoSecondsPtr      - (pointer to) frac nano seconds part of SER-RX frame latency.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiSerRxFrameLatencyDbgGet
(
  IN  GT_U8                             devNum,
  IN  GT_PHYSICAL_PORT_NUM              portNum,
  IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT      taiNumber,
  OUT GT_U32                            *nanoSecondsPtr,
  OUT GT_U32                            *fracNanoSecondsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiSerRxFrameLatencyDbgGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, taiNumber, nanoSecondsPtr, fracNanoSecondsPtr));

    rc = internal_cpssDxChPtpTaiSerRxFrameLatencyDbgGet(devNum, portNum, taiNumber, nanoSecondsPtr, fracNanoSecondsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, taiNumber, nanoSecondsPtr, fracNanoSecondsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPtpTaiTodStepDbgSet function
* @endinternal
*
* @brief   Debug API: Set TAI (Time Application Interface) TOD Step
*          per {port,taiNumber}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] taiNumber                - TAI number selection.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_NUMBER_0_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_1_E)
* @param[in] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiTodStepDbgSet
(
  IN  GT_U8                             devNum,
  IN  GT_PHYSICAL_PORT_NUM              portNum,
  IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT      taiNumber,
  IN  CPSS_DXCH_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
{

    GT_STATUS                          rc;
    GT_U32                             taiInstance;
    GT_U32                             hwValue[3];
    GT_U32                             regAddr[3];
    GT_U32                             mtipRegAddr[3] = {0};
    GT_U32                             i;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(todStepPtr);
    PRV_CPSS_DXCH_TAI_CHECK_MAC(devNum,taiNumber);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    if (todStepPtr->nanoSeconds >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    if (portNum >= PRV_CPSS_MAX_MAC_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    hwValue[0] = todStepPtr->nanoSeconds & 0xFFFF;
    hwValue[1] = (todStepPtr->fracNanoSeconds >> 16) & 0xFFFF;
    hwValue[2] = todStepPtr->fracNanoSeconds & 0xFFFF;

    /* convert from portNum to TAI instance */
    rc = prvCpssDxChPtpPortTaiInstanceGet(devNum,portNum,&taiInstance);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).TODStepNanoConfigLow;
    regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).TODStepFracConfigHigh;
    regAddr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).TODStepFracConfigLow;

    for (i = 0; i < 3; i++)
    {
        rc = prvCpssHwPpSetRegField(devNum, regAddr[i], 0, 16, hwValue[i]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        return GT_OK;

    /* TSM manipulates the TOD it receives according to indications from PCS.
       In order to have a constant 5000 in the CF, need to disable the TSX. */

    if (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        /* configure the MAC instances */
        switch (taiInstance)
        {
            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_400G0_TAI0_E:
            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_400G1_TAI0_E:
            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_200G2_TAI0_E:

                mtipRegAddr[0] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_EXT.tsuControl0;
                mtipRegAddr[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_EXT.tsuControl2;

                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[1], 0, 2, 0x3);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[0], 0, 6, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
                break;
        }

        /* in Harrier we need to config also the PCA instances */
        {
            if (portNum < 16)
                taiInstance = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_PCA0_TAI0_E;
            else if (portNum < 32)
                taiInstance = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_PCA1_TAI0_E;
            else if (portNum <= 38)
                taiInstance = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_PCA2_TAI0_E;
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong port number");

            regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).TODStepNanoConfigLow;
            regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).TODStepFracConfigHigh;
            regAddr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).TODStepFracConfigLow;

            for (i = 0; i < 3; i++)
            {
                rc = prvCpssHwPpSetRegField(devNum, regAddr[i], 0, 16, hwValue[i]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            mtipRegAddr[0] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_EXT.tsuControl0;
            mtipRegAddr[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_EXT.tsuControl2;

            rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[1], 0, 2, 0x3);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[0], 0, 6, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        switch (taiInstance)
        {
            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_USX2_0_TAI0_E:
            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_USX2_1_TAI0_E:
            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_MAC_CPUM_TAI0_E:

                /* in Pnx we need to config also the USX instances and PCA instance */
                taiInstance = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_PCA0_TAI0_E;

                regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).TODStepNanoConfigLow;
                regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).TODStepFracConfigHigh;
                regAddr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).TODStepFracConfigLow;

                for (i = 0; i < 3; i++)
                {
                    rc = prvCpssHwPpSetRegField(devNum, regAddr[i], 0, 16, hwValue[i]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }

                if (portNum < 16)
                    taiInstance = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_USX2_0_TAI0_E;
                else if (portNum < 32)
                    taiInstance = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_USX2_1_TAI0_E;
                else if (portNum < 48)
                    taiInstance = PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_MAC_CPUM_TAI0_E;
                else
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong port number");

                regAddr[0] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).TODStepNanoConfigLow;
                regAddr[1] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).TODStepFracConfigHigh;
                regAddr[2] = PRV_DXCH_REG1_UNIT_GOP_TAI_MAC(devNum, taiInstance, taiNumber).TODStepFracConfigLow;

                for (i = 0; i < 3; i++)
                {
                    rc = prvCpssHwPpSetRegField(devNum, regAddr[i], 0, 16, hwValue[i]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }

                mtipRegAddr[0] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_USX_EXT.portTsxControl2;
                mtipRegAddr[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_USX_EXT.portTsxControl3;
                mtipRegAddr[2] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_USX_EXT.portTsxControl4;

                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[2], 0, 3, 7);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[2], 11, 1, 1);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[1], 0, 3, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[0], 28, 3, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
                break;

            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_MAC_CPUC_TAI0_E:

                mtipRegAddr[0] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_CPU_EXT.tsuControl0;
                mtipRegAddr[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_CPU_EXT.tsuControl2;

                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[1], 1, 2, 0x3);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[0], 0, 6, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
                break;

            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_100G_TAI0_E:

                mtipRegAddr[0] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_EXT.tsuControl0;
                mtipRegAddr[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_EXT.tsuControl2;

                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[1], 0, 2, 0x3);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[0], 0, 6, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
                break;
        }
    }
    else /* SIP 6_10 */
    {
        switch (taiInstance)
        {
            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_CPU_PORT_TAI0_E:

                mtipRegAddr[0] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_CPU_EXT.tsuControl0;
                mtipRegAddr[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_CPU_EXT.tsuControl2;

                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[1], 1, 2, 0x3);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[0], 0, 6, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
                break;

            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G0_TAI0_E:
            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G1_TAI0_E:
            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G2_TAI0_E:
            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G3_TAI0_E:

                mtipRegAddr[0] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_EXT.tsuControl0;
                mtipRegAddr[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_EXT.tsuControl2;

                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[1], 0, 2, 0x3);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[0], 0, 6, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
                break;

            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_USX0_TAI0_E:
            case PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_USX1_TAI0_E:

                mtipRegAddr[0] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_USX_EXT.portTsxControl2;
                mtipRegAddr[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_USX_EXT.portTsxControl3;
                mtipRegAddr[2] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portNum].MTI_USX_EXT.portTsxControl4;

                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[2], 0, 2, 3);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[1], 0, 3, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
                rc = prvCpssHwPpSetRegField(devNum, mtipRegAddr[0], 28, 3, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }


  return GT_OK;
}

/**
* @internal cpssDxChPtpTaiTodStepDbgSet function
* @endinternal
*
* @brief   Debug API: Set TAI (Time Application Interface) TOD Step
*          per {port,taiNumber}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] taiNumber                - TAI number selection.
*                                       (APPLICABLE VALUES:
*                                           CPSS_DXCH_PTP_TAI_NUMBER_0_E,
*                                           CPSS_DXCH_PTP_TAI_NUMBER_1_E)
* @param[in] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodStepDbgSet
(
  IN  GT_U8                             devNum,
  IN  GT_PHYSICAL_PORT_NUM              portNum,
  IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT      taiNumber,
  IN  CPSS_DXCH_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiTodStepDbgSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, taiNumber, todStepPtr));

    rc = internal_cpssDxChPtpTaiTodStepDbgSet(devNum, portNum, taiNumber, todStepPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, taiNumber, todStepPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPtpTaiSelectDbgSet function
* @endinternal
*
* @brief   Set TAI select in unit CTSU/ERMRK.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] taiUnit                  - select TAI unit timestamping.
* @param[in] taiSelect                - select TAI for timestamping
*                                       for unit CTSU (APPLICABLE RANGES: 0..3.).
*                                       for unit ERMRK (APPLICABLE RANGES: 0..1.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPtpTaiSelectDbgSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT   taiUnit,
    IN  GT_U32                              taiSelect
)
{
    GT_STATUS rc;
    GT_U32  regAddr;        /* register address */
    GT_U32  portMacNum;     /* MAC number                   */
    GT_U32  tsuIndex;       /* Timestamp unit index         */
    GT_U32  tsuLocalChannel;/* Timestamp unit local channel */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_IRONMAN_E);
    switch (taiUnit)
    {
    case CPSS_DXCH_PTP_TAI_SELECT_UNIT_CTSU_E:
        if (taiSelect > 3)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

        PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

        tsuIndex        = 0;
        tsuLocalChannel = 0;

        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(devNum, portNum, &tsuIndex, &tsuLocalChannel);
        if (rc != GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->PCA_CTSU[tsuIndex].channelTsControl[tsuLocalChannel];
        return prvCpssDrvHwPpSetRegField(devNum, regAddr, 6, 2, taiSelect);

        break;

    case CPSS_DXCH_PTP_TAI_SELECT_UNIT_ERMRK_E:
        if (taiSelect > 1)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

        rc = prvCpssDxChWriteTableEntryField(devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
                                             portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                             0 /*fieldOffset*/, 1 /*fieldLength*/, 1 /*egress*/);
        if (rc != GT_OK)
        {
            return rc;
        }

        return prvCpssDxChWriteTableEntryField(devNum, CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
                                               portNum, PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                               1 /*fieldOffset*/, 1 /*fieldLength*/, taiSelect);
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPtpTaiSelectDbgSet function
* @endinternal
*
* @brief   Set TAI select in unit CTSU/ERMRK.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] taiUnit                  - select TAI unit timestamping.
* @param[in] taiSelect                - select TAI for timestamping
*                                       for unit CTSU (APPLICABLE RANGES: 0..3.).
*                                       for unit ERMRK (APPLICABLE RANGES: 0..1.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiSelectDbgSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT   taiUnit,
    IN  GT_U32                              taiSelect
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPtpTaiSelectDbgSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, taiUnit, taiSelect));

    rc = internal_cpssDxChPtpTaiSelectDbgSet(devNum, portNum, taiUnit, taiSelect);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, taiUnit, taiSelect));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

