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
* @file cpssDxChHwInitLedCtrl.c
*
* @brief Includes Leds control functions implementations. -- Dx
*
* @version   26
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChCpssHwInitLog.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* LED server clock minimal frequency in KHz */
#define FALCON_LED_RAVEN_CLK_MIN_CNS                500     /* 0.5 MHz */
#define FALCON_LED_RAVEN_CLK_MAX_CNS                80000   /* 80 MHz */

/* check LED interface number correctness */
#define LED_INTERFACE_NUM_CHECK_MAC(_devNum,_ledInterfaceNum)             \
    if(!prvCpssLedStreamNumInterfaceNumCheck(_devNum, _ledInterfaceNum))  \
    {                                                                     \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);    \
    }

/* check LED interface number correctness for xCat3 and AC5 devices */
#define XCAT3_LED_INTERFACE_NUM_CHECK_MAC(_ledInterfaceNum)             \
    if(_ledInterfaceNum >= PRV_CPSS_DXCH_XCAT3_LED_IF_NUM_CNS)           \
    {                                                                     \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);    \
    }


/* additional LED dummy indication to separate two LED streams in strobe */
#define FALCON_6_4_LED_INDICATION_GAP_COMPENSATION_CNS        1
#define AC5X_LED_INDICATION_GAP_COMPENSATION_CNS              8

/* Falcon 12.8 use N dummy indications where N is number indications in First LED unit. */
#define FALCON_12_8_LED_INDICATION_GAP_COMPENSATION_CNS       0


/* Falcon tiles 1 and 3 are mirrored images of tiles 0 and 2 accordingly */
static const GT_U32 chipletToLedUnitNumber[16] = {
                            /*Tile 0 Chiplets  0..3  */ 0,1,2,3,
                            /*Tile 1 Chiplets  4..7  */ 7,6,5,4,
                            /*Tile 2 Chiplets  8..11 */ 8,9,10,11,
                            /*Tile 3 Chiplets 12..15 */ 15,14,13,12};
/* Convert LED chiplet to LED unit number */
#define LED_UNIT_NUMBER_GET(_chiplet) chipletToLedUnitNumber[_chiplet]

#define NOT_USED_CNS    0xffffffff

/* LED servers register DB index in chain. Array accessed by LED server unit */
static const GT_U32 hawkLedDbUnitRegDbIndexArr[7] = {
        2  /* LED Unit 0 - PIPE1 400G */,
        5  /* LED Unit 1 - PIPE1 USX  */,
        0  /* LED Unit 2 - PIPE1 CPU PORT */,
        1  /* LED Unit 3 - PIPE0 400G */,
        6  /* LED Unit 4 - PIPE0 USX */,
        4  /* LED Unit 5 - PIPE3 400G */,
        3  /* LED Unit 6 - PIPE2 400G */
    };

#define LED_REG_DB_INDEX_BY_UNIT_ID(_dev, _unit_id)   \
    (PRV_CPSS_PP_MAC(_dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ? hawkLedDbUnitRegDbIndexArr[_unit_id] : _unit_id

/**
* @enum LED_CHAIN_ORDER
 *
 * @brief Classifies LED server's order in the LED chain
*/
typedef enum {
    /** LED server is the first not bypassed server in the LED chain */
    FIRST_LED_E,
    /** LED server is the last not bypassed server in the LED chain */
    LAST_LED_E,
    /** The single (the first and the last) not bypassed server in the LED chain */
    FIRST_LAST_LED_E,
    /** LED server is neither first nor last in the LED chain */
    OTHER_LED_E
}LED_CHAIN_ORDER;

/**
* @internal prvCpssDxChSip6LedUnitNumberByIndexGet function
* @endinternal
*
* @brief   Gets LED unit number by LED index.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum               - device number
* @param[in]  ledUnitIndex         - LED unit index (see description of LED index in description of CPSS_LED_SIP6_CONF_STC))
* @param[out] ledUnitNumberPtr     - (pointer to) LED unit number converted by LED unit index:
*                                               For Falcon devices: depends on tile index - tiles 1 and 3 are mirrored images of tiles 0 and 2 accordingly
*                                               For AC5X and above: LED unit index == LED init number
*
*/
static GT_VOID prvCpssDxChSip6LedUnitNumberByIndexGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledUnitIndex,
    OUT GT_U32                          *ledUnitNumberPtr
)
{
    GT_BOOL skipUnit;
    GT_U32 tileIndex  = (ledUnitIndex / 4);
    GT_U32 ledUnitNumber;

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            ledUnitNumber = LED_UNIT_NUMBER_GET(ledUnitIndex);
            prvCpssFalconRavenMemoryAccessCheck(devNum, tileIndex, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + ledUnitNumber % 4), &skipUnit);
            if (skipUnit == GT_TRUE)
            {
                *ledUnitNumberPtr = NOT_USED_CNS;
                return;
            }
            *ledUnitNumberPtr = ledUnitNumber;
            break;
        default:
            *ledUnitNumberPtr = ledUnitIndex;
            break;
    }
}

/* Gets LED unit number by LED index - see description of prvCpssDxChSip6LedUnitNumberByIndexGet */
#define LED_UNIT_NUMBER_BY_INDEX_GET_MAC(devNum, ledUnitIndex, ledUnitNumber)       \
    prvCpssDxChSip6LedUnitNumberByIndexGet(devNum, ledUnitIndex, &ledUnitNumber);   \
    if (ledUnitNumber == NOT_USED_CNS)                                              \
    {                                                                               \
        return GT_OK;                                                               \
    }

/* Max number of AC5 LED indication classes */
#define PRV_CPSS_DXCH_AC5_MAX_LED_CLASS_CNS                 14

/**
* @internal prvCpssDxChDevLedInterfacePortCheck function
* @endinternal
*
* @brief   Function checks device LED port for out of range.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] ledInterfaceNum          - led stream (interface)
* @param[in] ledPort                  - led port
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - on not initialized led port
*/
GT_STATUS prvCpssDxChDevLedInterfacePortCheck
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  GT_U32                          ledPort
)
{
    GT_U32 maxLedPorts = 0;
    GT_U32 maxInterfaceNum;
    const GT_U32 *convertPtr;

    static const GT_U32  aldrinPerIfLedNumArr[PRV_CPSS_DXCH_ALDRIN_LED_IF_NUM_CNS] =
    {
         PRV_CPSS_DXCH_ALDRIN_LED_IF_0_PORTS_NUM_CNS
        ,PRV_CPSS_DXCH_ALDRIN_LED_IF_1_PORTS_NUM_CNS
    };

    static const GT_U32  bc3perIfLedNumArr[PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS] =
    {
         PRV_CPSS_DXCH_BOBCAT3_LED_IF_0_PORTS_NUM_CNS
        ,PRV_CPSS_DXCH_BOBCAT3_LED_IF_1_PORTS_NUM_CNS
        ,PRV_CPSS_DXCH_BOBCAT3_LED_IF_2_PORTS_NUM_CNS
        ,PRV_CPSS_DXCH_BOBCAT3_LED_IF_3_PORTS_NUM_CNS
    };

    static const GT_U32  aldrin2perIfLedNumArr[PRV_CPSS_DXCH_ALDRIN2_LED_IF_NUM_CNS] =
    {
         /* ports 0..11 , 11..23, 48..71 , 24..47,72 (CPU) */
         12,12,24,24+1
    };

    if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
    {
        maxInterfaceNum = PRV_CPSS_DXCH_ALDRIN_LED_IF_NUM_CNS;
        convertPtr = &aldrinPerIfLedNumArr[0];
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        maxInterfaceNum = PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS;
        convertPtr = &bc3perIfLedNumArr[0];
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        maxInterfaceNum = PRV_CPSS_DXCH_ALDRIN2_LED_IF_NUM_CNS;
        convertPtr = &aldrin2perIfLedNumArr[0];
    }
    else
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        maxInterfaceNum = PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum;
        maxLedPorts     = PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedPorts;
        convertPtr = NULL;
    }
    else
    {
        maxInterfaceNum = PRV_CPSS_DXCH_BOBCAT2_LED_IF_NUN_CNS;
        maxLedPorts = PRV_CPSS_DXCH_BOBCAT2_LED_IF_PORTS_NUM_CNS;
        convertPtr = NULL;
    }

    if (ledInterfaceNum >= maxInterfaceNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "LED IF %d out of boundary [0-%d]",ledInterfaceNum, maxInterfaceNum-1);
    }

    if(convertPtr)
    {
        maxLedPorts = convertPtr[ledInterfaceNum];
    }

    if (ledPort >= maxLedPorts)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "LED port %d in stream %d out of boundary [0-%d]",ledPort,ledInterfaceNum, maxLedPorts-1);
    }

    return GT_OK;
}

/**
* @internal prvCpssLedStreamNumOfInterfacesInPortGroupGet function
* @endinternal
*
* @brief   Get the number of LED interfaces that the device's core supports
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                       Number of LED interfaces that the device's core supports
*
* @note Multi-core device will return information per core !!!
*
*/
GT_U32   prvCpssLedStreamNumOfInterfacesInPortGroupGet
(
    IN GT_U8 devNum
)
{
    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        /* lion2 */
        /* ports 0..11 --> PER CORE !!! */
        return 1;
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            /* Aldrin2 supports 4 interfaces */
            /* ports 0..11 , 11..23 , 24..47, 48..71,72 (CPU) */
            return PRV_CPSS_DXCH_ALDRIN2_LED_IF_NUM_CNS;
        }
        else
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        {
            /* bc3 supports 4 interfaces (2 per pipe) */
            /* ports 0..17 , 18..35 , 36..53, 54..71 */
            return PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS;
        }
        else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
        {
            /* bc2 supports 5 interfaces */
            /* ports 0..11 , 12..23 , 24..35 , 48..59 , (60..71) / (36..47) */
            return PRV_CPSS_DXCH_BOBCAT2_LED_IF_NUN_CNS;
        }
        else if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
                (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
        {
            /* aldrin supports 2 interfaces
             * led 0: ports 0-31
             * led 1: port 32 (DMA 62)*/
            return PRV_CPSS_DXCH_ALDRIN_LED_IF_NUM_CNS;
        }
        else if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
                (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ||
                (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ||
                (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E))
        {
            /* Falcon and AC5X supports single LED interface */
            return 1;
        }
        else
        {
            /* the device not supported .. yet */
            return 0;
        }
    }
    else
    {
        /* legacy devices :  xCat3; AC5
           number of led interfaces.
           LED intrerface 0 - Giga Ports 0..11, CPU port and XG ports 26, 27
           LED intrerface 1 - Giga Ports 12..23, and XG ports 24, 24.
        */

        return PRV_CPSS_DXCH_XCAT3_LED_IF_NUM_CNS;
    }
}

/**
* @internal prvCpssLedStreamNumInterfaceNumCheck function
* @endinternal
*
* @brief   Check LED interface number
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number.
* @param[in] ledInterfaceNum          - LED stream interface number.
*
* @retval GT_TRUE                  - ledInterfaceNum is OK
* @retval GT_FALSE                 - ledInterfaceNum is wrong
*/
static GT_BOOL prvCpssLedStreamNumInterfaceNumCheck
(
    IN GT_U8    devNum,
    IN GT_U32   ledInterfaceNum
)
{
    if(ledInterfaceNum >= prvCpssLedStreamNumOfInterfacesInPortGroupGet(devNum))
    {
        return GT_FALSE;
    }

    if((PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) == GT_TRUE) && (ledInterfaceNum < 3))
    {
        /* Cetus has LED interfaces 3..4 only */
        return GT_FALSE;
    }

    return GT_TRUE;
}

/**
* @internal prvCpssDxChPortLedInterfaceGet function
* @endinternal
*
* @brief   Get LED interface number by port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
*                                       RETURNS:
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - not supported device
*/
GT_STATUS prvCpssDxChPortLedInterfaceGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32              *interfaceNumberPtr
)
{
    GT_U32      portMacNum;             /* MAC number */
    GT_U32      ledsInterfaceMode = 0;  /* LED Interface Mode */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(interfaceNumberPtr);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        /*
        LED connectivity should be as follow
        * LED0 chain is connected to Ports 0-11 (25gig ports including cg)
        * LED1 chain is connected to Ports 12-23 (25gig ports including cg)
        * LED2 chain is connected to Ports 48-71 (10gig ports)
        * LED3 chain is connected to Ports 24-47 + CPU (10gig ports)
        */
        if(portMacNum < 12)
        {
            *interfaceNumberPtr = 0;
        }
        else
        if (portMacNum < 24)
        {
            *interfaceNumberPtr = 1;
        }
        else
        if((portMacNum < 48) ||
            (portMacNum == 72))
        {
            *interfaceNumberPtr = 3;
        }
        else
        if (portMacNum < 72)
        {
            *interfaceNumberPtr = 2;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
/*
        BC3 embed four LEDs chains (two per pipe):
            LED0 chain is connected to Ports 0-11 and to CPU0.
            LED1 chain is connected to Ports 12-35.
            LED2 chain is connected to Ports 36-47 and to CPU1.
            LED3 chain is connected to Ports 48-71.
*/
        if((portMacNum < 12) || (portMacNum == 72))
        {
            *interfaceNumberPtr = 0;
        }
        else if (portMacNum < 36)
        {
            *interfaceNumberPtr = 1;
        }
        else if((portMacNum < 48) || (portMacNum == 73))
        {
            *interfaceNumberPtr = 2;
        }
        else if (portMacNum < 72)
        {
            *interfaceNumberPtr = 3;
        }
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
        {
            /* bc2 supports 5 interfaces */
            /* ports 0..11 , 12..23 , 24..35 , 48..59 , (60..71) / (36..47) */
            *interfaceNumberPtr = (portMacNum < 36) ? (portMacNum / 12) :
                                  (portMacNum < 48) ? 4           :
                                  (portMacNum < 60) ? 3           : 4;
        }
        else if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
        {
            /* bobk-caelum supports 5 interfaces */
            /* ports 0..11 , 12..23 , 24..35 , 36..47 , 56..59 + 64..71*/

            if( prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,
                    PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl19,
                    0,
                    1,
                    &ledsInterfaceMode)!=GT_OK)
            {
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* Mode 0 ports 0..11 , 12..23 , 24..35 , 36..47 , 56..59 + 64..71*/
            *interfaceNumberPtr = (portMacNum < 48) ? (portMacNum / 12) : 4;

            /* Mode 1 ports 0..11 , 12..23 , 24..35 , 56..59  , 36..47+ 64..71*/
            if (ledsInterfaceMode == 1)
            {
                if ((portMacNum >= 35)&&(portMacNum <= 47))
                {
                    *interfaceNumberPtr = 4;
                }

                if ((portMacNum >= 56)&&(portMacNum <= 59))
                {
                    *interfaceNumberPtr = 3;
                }
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }
    else if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        /* Aldrin supports 2 LED interfaces: 0: Ports[31:0], 1: Ports[32] */
        *interfaceNumberPtr = (portMacNum < 32) ? 0 : 1;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        GT_U32  ravenNum;
        GT_U32  localRaven;
        GT_U32  tileNum;

        prvCpssDxChFalconPortMacDeviceMapGet(devNum, portMacNum, &ravenNum, &localRaven, &tileNum);
        *interfaceNumberPtr = LED_UNIT_NUMBER_GET(ravenNum);
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        *interfaceNumberPtr = 0; CPSS_TBD_BOOKMARK_AC5P
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        *interfaceNumberPtr = 0; CPSS_TBD_BOOKMARK_PHOENIX
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
    {
        *interfaceNumberPtr = 0; CPSS_TBD_BOOKMARK_HARRIER
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        *interfaceNumberPtr = 0; CPSS_TBD_BOOKMARK_IRONMAN
    }
    else
    {
        /* the device not supported .. yet */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLedInClockGet function
* @endinternal
*
* @brief   Get Input LED units clock.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[out] inClkInKhzPtr           - (pointer to) Input Clock of LED Units in KHz
*                                       RETURNS:
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChLedInClockGet
(
    IN GT_U8                devNum,
    OUT GT_U32             *inClkInKhzPtr
)
{
    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            *inClkInKhzPtr = 833333;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            /* both AC5P and Harrier have same LED Clock value */
            *inClkInKhzPtr = 843750;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            *inClkInKhzPtr = 650000;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChLedStreamClassAndGroupConfig function
* @endinternal
*
* @brief   This routine configures the control, the class manipulation and the
*         group configuration registers for both led interfaces in Dx.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledControl               - The user interface control register value.
* @param[in] ledClassConfPtr          (pointer to) a list of class manipulation registers
* @param[in] ledClassNum              - the class list length
* @param[in] ledGroupConfPtr          (pointer to) a list of group configuration registers
* @param[in] ledGroupNum              - the group list length
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - ledClassNum value or ledGroupNum value is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. See the Led Serial Interface in the device's data-sheet for full
*       detail of the leds configuration, classes and groups.
*
*/
static GT_STATUS internal_cpssDxChLedStreamClassAndGroupConfig
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ledControl,
    IN  CPSS_DXCH_LED_CLASS_CONF_STC        *ledClassConfPtr,
    IN  GT_U32                              ledClassNum,
    IN  CPSS_DXCH_LED_GROUP_CONF_STC        *ledGroupConfPtr,
    IN  GT_U32                              ledGroupNum
)
{
    GT_U32                  i,j;              /* loop iterators           */
    GT_U32                  fieldOffset;      /* written field offset     */
    GT_U32                  fieldLength;      /* written field length     */
    GT_U32                  classNum;         /* class indication number  */
    GT_U32                  groupNum;         /* group number             */
    GT_U32                  ledInterfacesNum; /* number of led interfaces */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(ledClassConfPtr);
    CPSS_NULL_PTR_CHECK_MAC(ledGroupConfPtr);

    /* validate class indication data */
    if (ledClassNum > CPSS_DXCH_MAX_LED_CLASS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* validate group data */
    if (ledGroupNum > CPSS_DXCH_MAX_LED_GROUP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* get the number of LED interfaces that the device's core supports */
    /* NOTE: multi-core device will return information per core !!! */
    ledInterfacesNum = prvCpssLedStreamNumOfInterfacesInPortGroupGet(devNum);

    if (ledInterfacesNum > LED_MAX_NUM_OF_INTERFACE)
    {
        /* should never happen */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* configure led interfaces */
    for (i = 0; i < ledInterfacesNum; i++)
    {
        /* set led control per interface */

        if (prvCpssDrvHwPpWriteRegister(devNum,
                                        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                        ledRegs.ledControl[i],
                                        ledControl) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* set class indication per interface */
        for (j = 0; j < ledClassNum ; j++)
        {
            classNum = ledClassConfPtr[j].classNum;

            switch (classNum)
            {
                case 0:
                case 2:
                    fieldOffset = 0;
                    fieldLength = 16;
                    break;

                case 1:
                case 3:
                    fieldOffset = 16;
                    fieldLength = 16;
                    break;

                case 4:
                case 5:
                    fieldOffset = 0;
                    fieldLength = 16;
                    break;

                case 6:
                    fieldOffset = 0;
                    fieldLength = 20;
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            if (prvCpssDrvHwPpSetRegField(devNum,
                                          PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                          ledRegs.ledClass[i][classNum],
                                          fieldOffset, fieldLength,
                                          ledClassConfPtr[j].classConf) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }

        /* set group logic per interface */
        for (j = 0; j < ledGroupNum; j++)
        {
            groupNum = ledGroupConfPtr[j].groupNum;

            switch (groupNum)
            {
                case 0:
                case 2:
                    fieldOffset = 0;
                    break;

                case 1:
                case 3:
                    fieldOffset = 16;
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            if (prvCpssDrvHwPpSetRegField(devNum,
                                          PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                          ledRegs.ledGroup[i][groupNum],
                                          fieldOffset, 16,
                                          ledGroupConfPtr[j].groupConf) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChLedStreamClassAndGroupConfig function
* @endinternal
*
* @brief   This routine configures the control, the class manipulation and the
*         group configuration registers for both led interfaces in Dx.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledControl               - The user interface control register value.
* @param[in] ledClassConfPtr          (pointer to) a list of class manipulation registers
* @param[in] ledClassNum              - the class list length
* @param[in] ledGroupConfPtr          (pointer to) a list of group configuration registers
* @param[in] ledGroupNum              - the group list length
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - ledClassNum value or ledGroupNum value is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. See the Led Serial Interface in the device's data-sheet for full
*       detail of the leds configuration, classes and groups.
*
*/
GT_STATUS cpssDxChLedStreamClassAndGroupConfig
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ledControl,
    IN  CPSS_DXCH_LED_CLASS_CONF_STC        *ledClassConfPtr,
    IN  GT_U32                              ledClassNum,
    IN  CPSS_DXCH_LED_GROUP_CONF_STC        *ledGroupConfPtr,
    IN  GT_U32                              ledGroupNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamClassAndGroupConfig);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledControl, ledClassConfPtr, ledClassNum, ledGroupConfPtr, ledGroupNum));

    rc = internal_cpssDxChLedStreamClassAndGroupConfig(devNum, ledControl, ledClassConfPtr, ledClassNum, ledGroupConfPtr, ledGroupNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledControl, ledClassConfPtr, ledClassNum, ledGroupConfPtr, ledGroupNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChLedStreamHyperGStackTxQStatusEnableSet function
* @endinternal
*
* @brief   Enables the display to HyperGStack Ports Transmit Queue Status via LED
*         Stream.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE - enable
*                                      The status of the HyperGStack Ports Transmit Queues is
*                                      conveyed via the LED stream. Port<i>TxqNotFull indication for each of
*                                      those ports is set to 0 when the number of buffers currently allocated in all
*                                      of this ports transmit queues exceeds the limit configured for this port.
*                                      GT_FALSE - disable
*                                      The status of the HyperGStack Ports Transmit Queues is
*                                      not conveyed via the LED stream.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChLedStreamHyperGStackTxQStatusEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    GT_U32      value;   /* register field value */
    GT_U32      regAddr; /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E
                                          | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);


    /* write enable value */
    value = BOOL2BIT_MAC(enable);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueMiscCtrl;

    return prvCpssDrvHwPpSetRegField(devNum, regAddr, 3, 1, value);
}

/**
* @internal cpssDxChLedStreamHyperGStackTxQStatusEnableSet function
* @endinternal
*
* @brief   Enables the display to HyperGStack Ports Transmit Queue Status via LED
*         Stream.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] enable                   - GT_TRUE - enable
*                                      The status of the HyperGStack Ports Transmit Queues is
*                                      conveyed via the LED stream. Port<i>TxqNotFull indication for each of
*                                      those ports is set to 0 when the number of buffers currently allocated in all
*                                      of this ports transmit queues exceeds the limit configured for this port.
*                                      GT_FALSE - disable
*                                      The status of the HyperGStack Ports Transmit Queues is
*                                      not conveyed via the LED stream.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChLedStreamHyperGStackTxQStatusEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamHyperGStackTxQStatusEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChLedStreamHyperGStackTxQStatusEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamHyperGStackTxQStatusEnableGet function
* @endinternal
*
* @brief   Gets HyperGStack Ports Transmit Queue Status via LED Stream.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - GT_TRUE - enable
*                                      The status of the HyperGStack Ports Transmit Queues is
*                                      conveyed via the LED stream. Port<i>TxqNotFull indication for each of
*                                      those ports is set to 0 when the number of buffers currently allocated in all
*                                      of this ports transmit queues exceeds the limit configured for this port.
*                                      GT_FALSE - disable
*                                      The status of the HyperGStack Ports Transmit Queues is
*                                      not conveyed via the LED stream.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChLedStreamHyperGStackTxQStatusEnableGet
(
    IN  GT_U8       devNum,
    OUT  GT_BOOL    *enablePtr
)
{
    GT_U32      value;   /* register field value */
    GT_U32      regAddr; /* register address */
    GT_STATUS   rc;      /* return status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E
                                          | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->egrTxQConf.txQueueMiscCtrl;

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 3, 1, &value);

    if(rc != GT_OK)
    {
        return rc;
    }

    /* write enable value */
    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChLedStreamHyperGStackTxQStatusEnableGet function
* @endinternal
*
* @brief   Gets HyperGStack Ports Transmit Queue Status via LED Stream.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - GT_TRUE - enable
*                                      The status of the HyperGStack Ports Transmit Queues is
*                                      conveyed via the LED stream. Port<i>TxqNotFull indication for each of
*                                      those ports is set to 0 when the number of buffers currently allocated in all
*                                      of this ports transmit queues exceeds the limit configured for this port.
*                                      GT_FALSE - disable
*                                      The status of the HyperGStack Ports Transmit Queues is
*                                      not conveyed via the LED stream.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChLedStreamHyperGStackTxQStatusEnableGet
(
    IN  GT_U8       devNum,
    OUT  GT_BOOL    *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamHyperGStackTxQStatusEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChLedStreamHyperGStackTxQStatusEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamCpuOrPort27ModeSet function
* @endinternal
*
* @brief   This routine configures the whether LED stream interface 0 contains CPU
*         or port 27 indications.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] indicatedPort            - the selected port indication: CPU or port 27.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamCpuOrPort27ModeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_LED_CPU_OR_PORT27_ENT indicatedPort
)
{
    GT_STATUS rc;           /* return status */
    GT_U32  regData;        /* register led control value */
    GT_U32  regAddr;        /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E
                                          | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);


    switch(indicatedPort)
    {
        case CPSS_DXCH_LED_CPU_E:       regData = 0 ;
                                        break;
        case CPSS_DXCH_LED_PORT27_E:    regData = 1 ;
                                        break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr  = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledXgClass04Manipulation[0];

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, regData);

    return rc;
}

/**
* @internal cpssDxChLedStreamCpuOrPort27ModeSet function
* @endinternal
*
* @brief   This routine configures the whether LED stream interface 0 contains CPU
*         or port 27 indications.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] indicatedPort            - the selected port indication: CPU or port 27.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamCpuOrPort27ModeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_LED_CPU_OR_PORT27_ENT indicatedPort
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamCpuOrPort27ModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, indicatedPort));

    rc = internal_cpssDxChLedStreamCpuOrPort27ModeSet(devNum, indicatedPort);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, indicatedPort));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamCpuOrPort27ModeGet function
* @endinternal
*
* @brief   This routine gets the whether LED stream interface 0 contains CPU
*         or port 27 indications.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] indicatedPortPtr         - (pointer to) the selected port indication:
*                                      CPU or port 27.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamCpuOrPort27ModeGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_LED_CPU_OR_PORT27_ENT *indicatedPortPtr
)
{
    GT_STATUS rc;           /* return status */
    GT_U32  regData;        /* register led control value */
    GT_U32  regAddr;        /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E
                                          | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(indicatedPortPtr);

    regAddr  = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledXgClass04Manipulation[0];

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 1, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    *indicatedPortPtr = (regData == 0) ? CPSS_DXCH_LED_CPU_E : CPSS_DXCH_LED_PORT27_E ;

    return GT_OK;
}

/**
* @internal cpssDxChLedStreamCpuOrPort27ModeGet function
* @endinternal
*
* @brief   This routine gets the whether LED stream interface 0 contains CPU
*         or port 27 indications.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @param[out] indicatedPortPtr         - (pointer to) the selected port indication:
*                                      CPU or port 27.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamCpuOrPort27ModeGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_LED_CPU_OR_PORT27_ENT *indicatedPortPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamCpuOrPort27ModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, indicatedPortPtr));

    rc = internal_cpssDxChLedStreamCpuOrPort27ModeGet(devNum, indicatedPortPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, indicatedPortPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChLedStreamClassManipXgAux function
* @endinternal
*
* @brief   Auxilary function to calculate register address and offsets for class
*         manipulation configuration for XG (Flex) ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
* @param[in] classNum                 - class number
*
* @param[out] regAddrPtr               - (pointer to) register address
* @param[out] invertEnableOffsetPtr    - (pointer to) invert enable offset in register.
* @param[out] blinkEnableOffsetPtr     - (pointer to) blink enable offset in register.
* @param[out] blinkSelectOffsetPtr     - (pointer to) blink select offset in register.
* @param[out] forceEnableOffsetPtr     - (pointer to) force enable offset in register.
* @param[out] forceDataOffsetPtr       - (pointer to) force data offset in register.
* @param[out] forceDataLengthPtr       - (pointer to) force data length in register.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChLedStreamClassManipXgAux
(
    IN  GT_U8   devNum,
    IN  GT_U32  ledInterfaceNum,
    IN  GT_U32  classNum,
    OUT GT_U32  *regAddrPtr,
    OUT GT_U32  *invertEnableOffsetPtr,
    OUT GT_U32  *blinkEnableOffsetPtr,
    OUT GT_U32  *blinkSelectOffsetPtr,
    OUT GT_U32  *forceEnableOffsetPtr,
    OUT GT_U32  *forceDataOffsetPtr,
    OUT GT_U32  *forceDataLengthPtr
)
{
    switch(classNum)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4: *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledXgClass04Manipulation[ledInterfaceNum];
                break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9: *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledXgClass59Manipulation[ledInterfaceNum];
                break;
        case 10:
        case 11: *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledXgClass1011Manipulation[ledInterfaceNum];
                 break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(classNum)
    {
        case 0:
        case 5: *invertEnableOffsetPtr  = 7 ;
                *blinkEnableOffsetPtr   = 5 ;
                *blinkSelectOffsetPtr   = 6 ;
                *forceEnableOffsetPtr   = 4 ;
                *forceDataOffsetPtr     = 2 ;
                break;
        case 1:
        case 6: *invertEnableOffsetPtr  = 13 ;
                *blinkEnableOffsetPtr   = 11 ;
                *blinkSelectOffsetPtr   = 12 ;
                *forceEnableOffsetPtr   = 10 ;
                *forceDataOffsetPtr     = 8  ;
                break;
        case 2:
        case 7: *invertEnableOffsetPtr  = 19 ;
                *blinkEnableOffsetPtr   = 17 ;
                *blinkSelectOffsetPtr   = 18 ;
                *forceEnableOffsetPtr   = 16 ;
                *forceDataOffsetPtr     = 14 ;
                break;
        case 3:
        case 8: *invertEnableOffsetPtr  = 25 ;
                *blinkEnableOffsetPtr   = 23 ;
                *blinkSelectOffsetPtr   = 24 ;
                *forceEnableOffsetPtr   = 22 ;
                *forceDataOffsetPtr     = 20 ;
                break;
        case 4:
        case 9: *invertEnableOffsetPtr  = 31 ;
                *blinkEnableOffsetPtr   = 29 ;
                *blinkSelectOffsetPtr   = 30 ;
                *forceEnableOffsetPtr   = 28 ;
                *forceDataOffsetPtr     = 26 ;
                break;

        case 10:*invertEnableOffsetPtr  = 25 ;
                *blinkEnableOffsetPtr   = 23 ;
                *blinkSelectOffsetPtr   = 24 ;
                *forceEnableOffsetPtr   = 22 ;
                *forceDataOffsetPtr     = 20 ;
                break;

        case 11:*invertEnableOffsetPtr  = 31 ;
                *blinkEnableOffsetPtr   = 29 ;
                *blinkSelectOffsetPtr   = 30 ;
                *forceEnableOffsetPtr   = 28 ;
                *forceDataOffsetPtr     = 26 ;
                break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *forceDataLengthPtr = 2;

    return GT_OK;
}

/**
* @internal internal_cpssDxChLedStreamClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] classNum                 - class number.
*                                      For tri-speed ports:
*                                      (APPLICABLE RANGES: xCat3, AC5, Lion2  0..6;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X 0..5)
*                                      For XG ports: (APPLICABLE RANGES: xCat3, AC5, Lion2 0..11;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..5)
* @param[in] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamClassManipulationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    return cpssDxChLedStreamPortGroupClassManipulationSet(devNum,
                                               CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                               ledInterfaceNum, portType,
                                               classNum, classParamsPtr);
}

/**
* @internal cpssDxChLedStreamClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] classNum                 - class number.
*                                      For tri-speed ports:
*                                      (APPLICABLE RANGES: xCat3, AC5, Lion2  0..6;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X 0..5)
*                                      For XG ports: (APPLICABLE RANGES: xCat3, AC5, Lion2 0..11;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..5)
* @param[in] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamClassManipulationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamClassManipulationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledInterfaceNum, portType, classNum, classParamsPtr));

    rc = internal_cpssDxChLedStreamClassManipulationSet(devNum, ledInterfaceNum, portType, classNum, classParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledInterfaceNum, portType, classNum, classParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] classNum                 - class number.
*                                      For tri-speed ports:
*                                      (APPLICABLE RANGES: xCat3, AC5, Lion2  0..6;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X 0..5)
*                                      For XG ports: (APPLICABLE RANGES: xCat3, AC5, Lion2 0..11;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..5)
*
* @param[out] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamClassManipulationGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    OUT CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    return cpssDxChLedStreamPortGroupClassManipulationGet(devNum,
                                               CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                               ledInterfaceNum, portType,
                                               classNum, classParamsPtr);
}

/**
* @internal cpssDxChLedStreamClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] classNum                 - class number.
*                                      For tri-speed ports:
*                                      (APPLICABLE RANGES: xCat3, AC5, Lion2  0..6;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X 0..5)
*                                      For XG ports: (APPLICABLE RANGES: xCat3, AC5, Lion2 0..11;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..5)
*
* @param[out] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamClassManipulationGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    OUT CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamClassManipulationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledInterfaceNum, portType, classNum, classParamsPtr));

    rc = internal_cpssDxChLedStreamClassManipulationGet(devNum, ledInterfaceNum, portType, classNum, classParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledInterfaceNum, portType, classNum, classParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamGroupConfigSet function
* @endinternal
*
* @brief   This routine configures a LED stream group.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] groupNum                 - group number
*                                      For tri-speed ports: (APPLICABLE RANGES: 0..3)
*                                      For XG ports: (APPLICABLE RANGES: 0..1)
* @param[in] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - on out of range class number
*                                       (allowable class range 0..15)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamGroupConfigSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    IN  CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    return cpssDxChLedStreamPortGroupGroupConfigSet(devNum,
                                               CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                               ledInterfaceNum, portType,
                                               groupNum, groupParamsPtr);
}

/**
* @internal cpssDxChLedStreamGroupConfigSet function
* @endinternal
*
* @brief   This routine configures a LED stream group.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] groupNum                 - group number
*                                      For tri-speed ports: (APPLICABLE RANGES: 0..3)
*                                      For XG ports: (APPLICABLE RANGES: 0..1)
* @param[in] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - on out of range class number
*                                       (allowable class range o..15)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamGroupConfigSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    IN  CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamGroupConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledInterfaceNum, portType, groupNum, groupParamsPtr));

    rc = internal_cpssDxChLedStreamGroupConfigSet(devNum, ledInterfaceNum, portType, groupNum, groupParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledInterfaceNum, portType, groupNum, groupParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChLedStreamGroupConfigGet function
* @endinternal
*
* @brief   This routine gets a LED stream group configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] groupNum                 - group number
*                                      For tri-speed ports: (APPLICABLE RANGES: 0..3)
*                                      For XG ports: (APPLICABLE RANGES: 0..1)
*
* @param[out] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamGroupConfigGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    OUT CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    return cpssDxChLedStreamPortGroupGroupConfigGet(devNum,
                                               CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                               ledInterfaceNum, portType,
                                               groupNum, groupParamsPtr);
}

/**
* @internal cpssDxChLedStreamGroupConfigGet function
* @endinternal
*
* @brief   This routine gets a LED stream group configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] groupNum                 - group number
*                                      For tri-speed ports: (APPLICABLE RANGES: 0..3)
*                                      For XG ports: (APPLICABLE RANGES: 0..1)
*
* @param[out] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamGroupConfigGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    OUT CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamGroupConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledInterfaceNum, portType, groupNum, groupParamsPtr));

    rc = internal_cpssDxChLedStreamGroupConfigGet(devNum, ledInterfaceNum, portType, groupNum, groupParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledInterfaceNum, portType, groupNum, groupParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANG           - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamConfigSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              ledInterfaceNum,
    IN  CPSS_LED_CONF_STC   *ledConfPtr
)
{
    return cpssDxChLedStreamPortGroupConfigSet(devNum,
                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                              ledInterfaceNum, ledConfPtr);
}

/**
* @internal cpssDxChLedStreamConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANG           - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamConfigSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              ledInterfaceNum,
    IN  CPSS_LED_CONF_STC   *ledConfPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledInterfaceNum, ledConfPtr));

    rc = internal_cpssDxChLedStreamConfigSet(devNum, ledInterfaceNum, ledConfPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledInterfaceNum, ledConfPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
*
* @param[out] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamConfigGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              ledInterfaceNum,
    OUT CPSS_LED_CONF_STC   *ledConfPtr
)
{
    return cpssDxChLedStreamPortGroupConfigGet(devNum,
                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                              ledInterfaceNum, ledConfPtr);
}

/**
* @internal cpssDxChLedStreamConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
*
* @param[out] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamConfigGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              ledInterfaceNum,
    OUT CPSS_LED_CONF_STC   *ledConfPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledInterfaceNum, ledConfPtr));

    rc = internal_cpssDxChLedStreamConfigGet(devNum, ledInterfaceNum, ledConfPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledInterfaceNum, ledConfPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamClassIndicationSet function
* @endinternal
*
* @brief   This routine configures classes 9-11 indication (debug) for flex ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number (APPLICABLE RANGES: 0..1)
* @param[in] classNum                 - class number (APPLICABLE RANGES: 9..11)
* @param[in] indication               - port (debug)  data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamClassIndicationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  GT_U32                          classNum,
    IN  CPSS_DXCH_LED_INDICATION_ENT    indication
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr;            /* register address */
    GT_U32  offset;             /* offset in register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    XCAT3_LED_INTERFACE_NUM_CHECK_MAC(ledInterfaceNum);

    switch(classNum)
    {
        case 9:  offset = 0;
                 break;
        case 10: offset = 4;
                 break;
        case 11: offset = 8;
                 break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledHyperGStackDebugSelect[ledInterfaceNum];

    switch(indication)
    {
        case CPSS_DXCH_LED_INDICATION_PRIMARY_E:                       regData = 0;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_LINK_E:                          regData = 1;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_RX_ACT_E:                        regData = 2;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_TX_ACT_E:                        regData = 3;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_LOCAL_FAULT_OR_PCS_LINK_E:       regData = 4;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_REMOTE_FAULT_OR_RESERVED_E:      regData = 5;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_UKN_SEQ_OR_DUPLEX_E:             regData = 6;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_P_REJ_E:                         regData = 7;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_RX_ERROR_E:                      regData = 8;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_JABBER_E:                        regData = 9;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_FRAGMENT_E:                      regData = 10;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_CRC_ERROR_E:                     regData = 11;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_FC_RX_E:                         regData = 12;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_TX_BAD_CRC_OR_LATE_COL_E:        regData = 13;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_RX_BUFFER_FULL_OR_BACK_PR_E:
            /* uses for not xcat3 a1 and above */
            if(PRV_CPSS_DXCH_XCAT3_A1_AND_ABOVE_CHECK_MAC(devNum) == GT_FALSE)
            {
                regData = 14;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_LED_INDICATION_WATCHDOG_EXP_SAMPLE_E:           regData = 15;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E:
            /* uses for xcat3 a1 and above */
            if(PRV_CPSS_DXCH_XCAT3_A1_AND_ABOVE_CHECK_MAC(devNum) == GT_TRUE)
            {
                regData = 14;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;

        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, offset, 4, regData);

    return rc;
}

/**
* @internal cpssDxChLedStreamClassIndicationSet function
* @endinternal
*
* @brief   This routine configures classes 9-11 indication (debug) for flex ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number (APPLICABLE RANGES: 0..1)
* @param[in] classNum                 - class number (APPLICABLE RANGES: 9..11)
* @param[in] indication               - port (debug)  data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamClassIndicationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  GT_U32                          classNum,
    IN  CPSS_DXCH_LED_INDICATION_ENT    indication
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamClassIndicationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledInterfaceNum, classNum, indication));

    rc = internal_cpssDxChLedStreamClassIndicationSet(devNum, ledInterfaceNum, classNum, indication);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledInterfaceNum, classNum, indication));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamClassIndicationGet function
* @endinternal
*
* @brief   This routine gets classes 9-11 indication (debug) for flex ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number (APPLICABLE RANGES: 0..1)
* @param[in] classNum                 - class number (APPLICABLE RANGES: 9..11)
*
* @param[out] indicationPtr            - (pointer to) port (debug) indication data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamClassIndicationGet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         ledInterfaceNum,
    IN  GT_U32                         classNum,
    OUT CPSS_DXCH_LED_INDICATION_ENT   *indicationPtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr;            /* register address */
    GT_U32  offset;             /* offset in register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(indicationPtr);

    XCAT3_LED_INTERFACE_NUM_CHECK_MAC(ledInterfaceNum);

    switch(classNum)
    {
        case 9:  offset = 0;
                 break;
        case 10: offset = 4;
                 break;
        case 11: offset = 8;
                 break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledHyperGStackDebugSelect[ledInterfaceNum];
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, offset, 4, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    switch(regData)
    {
        case 0:  *indicationPtr = CPSS_DXCH_LED_INDICATION_PRIMARY_E;
                 break;
        case 1:  *indicationPtr = CPSS_DXCH_LED_INDICATION_LINK_E;
                 break;
        case 2:  *indicationPtr = CPSS_DXCH_LED_INDICATION_RX_ACT_E;
                 break;
        case 3:  *indicationPtr = CPSS_DXCH_LED_INDICATION_TX_ACT_E;
                 break;
        case 4:  *indicationPtr = CPSS_DXCH_LED_INDICATION_LOCAL_FAULT_OR_PCS_LINK_E;
                 break;
        case 5:  *indicationPtr = CPSS_DXCH_LED_INDICATION_REMOTE_FAULT_OR_RESERVED_E;
                 break;
        case 6:  *indicationPtr = CPSS_DXCH_LED_INDICATION_UKN_SEQ_OR_DUPLEX_E;
                 break;
        case 7:  *indicationPtr = CPSS_DXCH_LED_INDICATION_P_REJ_E;
                 break;
        case 8:  *indicationPtr = CPSS_DXCH_LED_INDICATION_RX_ERROR_E;
                 break;
        case 9:  *indicationPtr = CPSS_DXCH_LED_INDICATION_JABBER_E;
                 break;
        case 10: *indicationPtr = CPSS_DXCH_LED_INDICATION_FRAGMENT_E;
                 break;
        case 11: *indicationPtr = CPSS_DXCH_LED_INDICATION_CRC_ERROR_E;
                 break;
        case 12: *indicationPtr = CPSS_DXCH_LED_INDICATION_FC_RX_E;
                 break;
        case 13: *indicationPtr = CPSS_DXCH_LED_INDICATION_TX_BAD_CRC_OR_LATE_COL_E;
                 break;
        case 14: *indicationPtr = CPSS_DXCH_LED_INDICATION_RX_BUFFER_FULL_OR_BACK_PR_E;
                 break;
        case 15: *indicationPtr = CPSS_DXCH_LED_INDICATION_WATCHDOG_EXP_SAMPLE_E;
                 break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChLedStreamClassIndicationGet function
* @endinternal
*
* @brief   This routine gets classes 9-11 indication (debug) for flex ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number (APPLICABLE RANGES: 0..1)
* @param[in] classNum                 - class number (APPLICABLE RANGES: 9..11)
*
* @param[out] indicationPtr            - (pointer to) port (debug) indication data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamClassIndicationGet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         ledInterfaceNum,
    IN  GT_U32                         classNum,
    OUT CPSS_DXCH_LED_INDICATION_ENT   *indicationPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamClassIndicationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledInterfaceNum, classNum, indicationPtr));

    rc = internal_cpssDxChLedStreamClassIndicationGet(devNum, ledInterfaceNum, classNum, indicationPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledInterfaceNum, classNum, indicationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamDirectModeEnableSet function
* @endinternal
*
* @brief   This routine enables\disables LED Direct working mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number (APPLICABLE RANGES: 0..1)
* @param[in] enable                   - GT_TRUE: direct mode
*                                      GT_FALSE: serial mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note In Direct mode LED pin indicates a different Stack port status.
*       The external pins are mapped as follows:
*       LEDDATA1 — indicates stack port 24
*       LEDSTB1 — indicates stack port 25
*       LEDDATA0 — indicates stack port 26
*       LEDSTB0 — indicates stack port 27
*       The indication for each port is driven from the LED group0
*       configurations.
*
*/
static GT_STATUS internal_cpssDxChLedStreamDirectModeEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  ledInterfaceNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr;            /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                           CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    XCAT3_LED_INTERFACE_NUM_CHECK_MAC(ledInterfaceNum);

    regData = BOOL2BIT_MAC(enable);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledHyperGStackDebugSelect1[ledInterfaceNum];

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, regData);

    return rc;
}

/**
* @internal cpssDxChLedStreamDirectModeEnableSet function
* @endinternal
*
* @brief   This routine enables\disables LED Direct working mode.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number (APPLICABLE RANGES: 0..1)
* @param[in] enable                   - GT_TRUE: direct mode
*                                      GT_FALSE: serial mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note In Direct mode LED pin indicates a different Stack port status.
*       The external pins are mapped as follows:
*       LEDDATA1 — indicates stack port 24
*       LEDSTB1 — indicates stack port 25
*       LEDDATA0 — indicates stack port 26
*       LEDSTB0 — indicates stack port 27
*       The indication for each port is driven from the LED group0
*       configurations.
*
*/
GT_STATUS cpssDxChLedStreamDirectModeEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  ledInterfaceNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamDirectModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledInterfaceNum, enable));

    rc = internal_cpssDxChLedStreamDirectModeEnableSet(devNum, ledInterfaceNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledInterfaceNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamDirectModeEnableGet function
* @endinternal
*
* @brief   This routine gets LED pin indication direct mode enabling status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number (APPLICABLE RANGES: 0..1)
*
* @param[out] enablePtr                - (pointer to) GT_TRUE: direct mode
*                                      GT_FALSE: serial mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note In Direct mode LED pin indicates a different Stack port status.
*       The external pins are mapped as follows:
*       LEDDATA1 - indicates stack port 24
*       LEDSTB1 - indicates stack port 25
*       LEDDATA0 - indicates stack port 26
*       LEDSTB0 - indicates stack port 27
*       The indication for each port is driven from the LED group0
*       configurations.
*
*/
static GT_STATUS internal_cpssDxChLedStreamDirectModeEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  ledInterfaceNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr;            /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E
                                          | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    XCAT3_LED_INTERFACE_NUM_CHECK_MAC(ledInterfaceNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledHyperGStackDebugSelect1[ledInterfaceNum];

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 1, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regData);

    return GT_OK;
}

/**
* @internal cpssDxChLedStreamDirectModeEnableGet function
* @endinternal
*
* @brief   This routine gets LED pin indication direct mode enabling status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number (APPLICABLE RANGES: 0..1)
*
* @param[out] enablePtr                - (pointer to) GT_TRUE: direct mode
*                                      GT_FALSE: serial mode
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note In Direct mode LED pin indicates a different Stack port status.
*       The external pins are mapped as follows:
*       LEDDATA1 - indicates stack port 24
*       LEDSTB1 - indicates stack port 25
*       LEDDATA0 - indicates stack port 26
*       LEDSTB0 - indicates stack port 27
*       The indication for each port is driven from the LED group0
*       configurations.
*
*/
GT_STATUS cpssDxChLedStreamDirectModeEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  ledInterfaceNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamDirectModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledInterfaceNum, enablePtr));

    rc = internal_cpssDxChLedStreamDirectModeEnableGet(devNum, ledInterfaceNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledInterfaceNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChLedStreamGroupConfTriSpeedAux function
* @endinternal
*
* @brief   Auxilary function to calculate register address and offset for group
*         configuration for tri-speed (network) ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] interfaceNum             - LED stream interface number.
* @param[in] groupNum                 - group number
*
* @param[out] regAddrPtr               - (pointer to) register address
* @param[out] groupDataOffsetPtr       - (pointer to) group data offset in register.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChLedStreamGroupConfTriSpeedAux
(
    IN  GT_U8   devNum,
    IN  GT_U32  interfaceNum,
    IN  GT_U32  groupNum,
    OUT GT_U32  *regAddrPtr,
    OUT GT_U32  *groupDataOffsetPtr
)
{
    if( groupNum >= CPSS_DXCH_MAX_LED_GROUP_CNS )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledGroup[interfaceNum][groupNum];

    *groupDataOffsetPtr = ( groupNum & 0x1 ) ? 16 : 0 ;

    return GT_OK;
}

/**
* @internal prvCpssDxChBobcat2LedStreamConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChBobcat2LedStreamConfigSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              ledInterfaceNum,
    IN  CPSS_LED_CONF_STC   *ledConfPtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr;            /* register address */
    GT_U32  regMask;            /* register data mask */
    GT_U32  data;               /* local data */
    GT_U32  blink;              /* blink index */
    CPSS_LED_BLINK_DUTY_CYCLE_ENT   blinkDutyCycle; /* duty cycle of the blink signal */
    CPSS_LED_BLINK_DURATION_ENT     blinkDuration; /* blink freqency divider */

    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* LED control */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].LEDControl;
    regData = 0;

    U32_SET_FIELD_MAC(regData, 8,  8, ledConfPtr->ledStart);
    U32_SET_FIELD_MAC(regData, 16, 8, ledConfPtr->ledEnd);
    switch(ledConfPtr->pulseStretch)
    {
        case CPSS_LED_PULSE_STRETCH_0_NO_E:
            data = 0;
            break;
        case CPSS_LED_PULSE_STRETCH_1_E:
            data = 1;
            break;
        case CPSS_LED_PULSE_STRETCH_2_E:
            data = 2;
            break;
        case CPSS_LED_PULSE_STRETCH_3_E:
            data = 3;
            break;
        case CPSS_LED_PULSE_STRETCH_4_E:
            data = 4;
            break;
        case CPSS_LED_PULSE_STRETCH_5_E:
            data = 5;
            break;
        case CPSS_LED_PULSE_STRETCH_6_E:
            data = 6;
            break;
        case CPSS_LED_PULSE_STRETCH_7_E:
            data = 7;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData, 5, 3, data);


    U32_SET_FIELD_MAC(regData, 4, 1, (ledConfPtr->invertEnable == GT_TRUE) ? 0 : 1);

    switch(ledConfPtr->ledClockFrequency)
    {
        case CPSS_LED_CLOCK_OUT_FREQUENCY_500_E:
            data = 0;
            break;
        case CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E:
            data = 1;
            break;
        case CPSS_LED_CLOCK_OUT_FREQUENCY_2000_E:
            data = 2;
            break;
        case CPSS_LED_CLOCK_OUT_FREQUENCY_3000_E:
            data = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData, 2, 2, data);

    switch(ledConfPtr->ledOrganize)
    {
        case CPSS_LED_ORDER_MODE_BY_PORT_E:
            data = 0;
            break;
        case CPSS_LED_ORDER_MODE_BY_CLASS_E:
            data = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData, 1, 1, data);

    regMask = 0;
    U32_SET_FIELD_MAC(regMask , 1, 23, ((1 << 23)-1));
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMask, regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Blink global control */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].blinkGlobalControl;
    regData = 0;

    for (blink = 0; blink < 2; blink++)
    {
        if (blink == 0)
        {
            blinkDuration = ledConfPtr->blink0Duration;
            blinkDutyCycle = ledConfPtr->blink0DutyCycle;
        }
        else
        {
            blinkDuration = ledConfPtr->blink1Duration;
            blinkDutyCycle = ledConfPtr->blink1DutyCycle;
        }
        switch (blinkDuration)
        {
            case CPSS_LED_BLINK_DURATION_0_E:
                data = 0;
                break;
            case CPSS_LED_BLINK_DURATION_1_E:
                data = 1;
                break;
            case CPSS_LED_BLINK_DURATION_2_E:
                data = 2;
                break;
            case CPSS_LED_BLINK_DURATION_3_E:
                data = 3;
                break;
            case CPSS_LED_BLINK_DURATION_4_E:
                data = 4;
                break;
            case CPSS_LED_BLINK_DURATION_5_E:
                data = 5;
                break;
            case CPSS_LED_BLINK_DURATION_6_E:
                data = 6;
                break;
            case CPSS_LED_BLINK_DURATION_7_E:
                data = 7;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        U32_SET_FIELD_MAC(regData , 0 + (5 * blink), 3, data);

        switch(blinkDutyCycle)
        {
            case CPSS_LED_BLINK_DUTY_CYCLE_0_E:
                data = 0;
                break;
            case CPSS_LED_BLINK_DUTY_CYCLE_1_E:
            case CPSS_LED_BLINK_DUTY_CYCLE_2_E:
                data = 1;
                break;
            case CPSS_LED_BLINK_DUTY_CYCLE_3_E:
                data = 2;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        U32_SET_FIELD_MAC(regData , 3 + (5 * blink), 2, data);
    }

    return prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 10, regData);
}

/* Falcon LED HW configuaration data */
typedef struct CPSS_FALCON_LED_CONF_HW_STCT
{
    GT_U32  ledOrganize;
    GT_U32  blink0DutyCycle;
    GT_U32  blink0DurationDivider;
    GT_U32  blink1DutyCycle;
    GT_U32  blink1DurationDivider;
    GT_U32  pulseStretchDivider;
    GT_U32  ledClockFrequencyDivider;
    GT_U32  ledStart[CPSS_CHIPLETS_MAX_NUM_CNS];
    GT_U32  ledEnd[CPSS_CHIPLETS_MAX_NUM_CNS];
    GT_U32  ledChainSize;
    GT_U32  ledChainBypass[CPSS_CHIPLETS_MAX_NUM_CNS];
    GT_U32  firstActiveLedUnitInChain;
    GT_U32  lastActiveLedUnitInChain;
} PRV_CPSS_FALCON_LED_CONF_HW_STC;

static GT_STATUS prvCpssDxChFalconLedStreamConfigGet
(
    IN  GT_U8               devNum,
    OUT CPSS_LED_CONF_STC   *ledConfPtr,
    OUT PRV_CPSS_FALCON_LED_CONF_HW_STC  *ledConfHwPtr
);

static GT_STATUS prvCpssDxChFalconLedStreamConfigParamToHwConvert
(
    IN  GT_U8                               devNum,
    IN  CPSS_LED_CONF_STC                   *ledConfPtr,
    OUT PRV_CPSS_FALCON_LED_CONF_HW_STC     *ledConfHwPtr
);

/**
* @internal prvCpssDxChFalconLedClockFrequencyConvert function
* @endinternal
*
* @brief   LED clock frequence validity check and convert
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[inout] ledConfPtr         - (pointer to) LED stream configuration structure
* @param[inout] ledConfHwPtr       - (pointer to) LED stream HW structure
* @param[in] hwConvert             - conversion direction (from/to SW/HW structure)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - not valid HW data
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
*/
static GT_STATUS prvCpssDxChFalconLedClockFrequencyConvert
(
    IN      GT_U8                               devNum,
    INOUT   CPSS_LED_CONF_STC                   *ledConfPtr,
    INOUT   PRV_CPSS_FALCON_LED_CONF_HW_STC     *ledConfHwPtr,
    IN      GT_BOOL                             hwConvert
)
{
    GT_U32      inLedClkInKhz; /* LED Clock in Khz */
    GT_STATUS   rc; /* return status */

    rc = prvCpssDxChLedInClockGet(devNum,&inLedClkInKhz);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (hwConvert == GT_TRUE)
    {
        if (ledConfPtr->sip6LedConfig.ledClockFrequency < FALCON_LED_RAVEN_CLK_MIN_CNS  ||
            ledConfPtr->sip6LedConfig.ledClockFrequency > FALCON_LED_RAVEN_CLK_MAX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* Ratio of clock divider relative to app_clk for generating the LED clock out */
        ledConfHwPtr->ledClockFrequencyDivider = (GT_U32)(inLedClkInKhz/ledConfPtr->sip6LedConfig.ledClockFrequency);
        /* This value must be even and the minimal value is 2 */
        if (ledConfHwPtr->ledClockFrequencyDivider < 2)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        if (ledConfHwPtr->ledClockFrequencyDivider % 2 != 0)
        {
            ledConfHwPtr->ledClockFrequencyDivider = ledConfHwPtr->ledClockFrequencyDivider & ~(0x1);
        }
    }
    else
    {
        if (ledConfHwPtr->ledClockFrequencyDivider == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        ledConfPtr->sip6LedConfig.ledClockFrequency = (GT_U32)(inLedClkInKhz/ledConfHwPtr->ledClockFrequencyDivider);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFalconLedBlinkDurationConvert function
* @endinternal
*
* @brief   LED Blink duration validity check and convert
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[inout] ledConfPtr         - (pointer to) LED stream configuration structure
* @param[inout] ledConfHwPtr       - (pointer to) LED stream HW structure
* @param[in] hwConvert             - conversion direction (from/to SW/HW structure)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - not valid HW data
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChFalconLedBlinkDurationConvert
(
    IN      GT_U8                               devNum,
    INOUT   CPSS_LED_CONF_STC                   *ledConfPtr,
    INOUT   PRV_CPSS_FALCON_LED_CONF_HW_STC     *ledConfHwPtr,
    IN      GT_BOOL                             hwConvert
)
{
    GT_U32      inLedClkInKhz; /* LED Clock in Khz */
    GT_STATUS   rc; /* return status */

    rc = prvCpssDxChLedInClockGet(devNum, &inLedClkInKhz);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (hwConvert == GT_TRUE)
    {
        switch (ledConfPtr->blink0Duration)
        {
            case CPSS_LED_BLINK_DURATION_0_E:
                ledConfHwPtr->blink0DurationDivider = 32; break;
            case CPSS_LED_BLINK_DURATION_1_E:
                ledConfHwPtr->blink0DurationDivider = 64; break;
            case CPSS_LED_BLINK_DURATION_2_E:
                ledConfHwPtr->blink0DurationDivider = 128; break;
            case CPSS_LED_BLINK_DURATION_3_E:
                ledConfHwPtr->blink0DurationDivider = 256; break;
            case CPSS_LED_BLINK_DURATION_4_E:
                ledConfHwPtr->blink0DurationDivider = 512; break;
            case CPSS_LED_BLINK_DURATION_5_E:
                ledConfHwPtr->blink0DurationDivider = 1024; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        ledConfHwPtr->blink0DurationDivider = ledConfHwPtr->blink0DurationDivider * inLedClkInKhz;
        if (ledConfHwPtr->blink0DurationDivider >= BIT_30)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        switch (ledConfPtr->blink1Duration)
        {
            case CPSS_LED_BLINK_DURATION_0_E:
                ledConfHwPtr->blink1DurationDivider = 32; break;
            case CPSS_LED_BLINK_DURATION_1_E:
                ledConfHwPtr->blink1DurationDivider = 64; break;
            case CPSS_LED_BLINK_DURATION_2_E:
                ledConfHwPtr->blink1DurationDivider = 128; break;
            case CPSS_LED_BLINK_DURATION_3_E:
                ledConfHwPtr->blink1DurationDivider = 256; break;
            case CPSS_LED_BLINK_DURATION_4_E:
                ledConfHwPtr->blink1DurationDivider = 512; break;
            case CPSS_LED_BLINK_DURATION_5_E:
                ledConfHwPtr->blink1DurationDivider = 1024; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        ledConfHwPtr->blink1DurationDivider = ledConfHwPtr->blink1DurationDivider * inLedClkInKhz;
        if (ledConfHwPtr->blink1DurationDivider >= BIT_30)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        ledConfHwPtr->blink0DurationDivider = ledConfHwPtr->blink0DurationDivider / inLedClkInKhz;
        switch (ledConfHwPtr->blink0DurationDivider)
        {
            case 32:
                ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_0_E; break;
            case 64:
                ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_1_E; break;
            case 128:
                ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_2_E; break;
            case 256:
                ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_3_E; break;
            case 512:
                ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_4_E; break;
            case 1024:
                ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_5_E; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        ledConfHwPtr->blink1DurationDivider = ledConfHwPtr->blink1DurationDivider / inLedClkInKhz;
        switch (ledConfHwPtr->blink1DurationDivider)
        {
            case 32:
                ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_0_E; break;
            case 64:
                ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_1_E; break;
            case 128:
                ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_2_E; break;
            case 256:
                ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_3_E; break;
            case 512:
                ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_4_E; break;
            case 1024:
                ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_5_E; break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFalconLedPulseStretchConvert function
* @endinternal
*
* @brief   LED pulse stretching divider validity check and convert
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[inout] ledConfPtr         - (pointer to) LED stream configuration structure
* @param[inout] ledConfHwPtr       - (pointer to) LED stream HW structure
* @param[in] hwConvert             - conversion direction (from/to SW/HW structure)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - not valid HW data
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChFalconLedPulseStretchConvert
(
    INOUT   CPSS_LED_CONF_STC                   *ledConfPtr,
    INOUT   PRV_CPSS_FALCON_LED_CONF_HW_STC     *ledConfHwPtr,
    IN      GT_BOOL                             hwConvert
)
{
    if (hwConvert == GT_TRUE)
    {
        switch (ledConfPtr->pulseStretch)
        {
            case CPSS_LED_PULSE_STRETCH_1_E:
                ledConfHwPtr->pulseStretchDivider = 10000; break;
            case CPSS_LED_PULSE_STRETCH_2_E:
                ledConfHwPtr->pulseStretchDivider = 500000; break;
            case CPSS_LED_PULSE_STRETCH_3_E:
                ledConfHwPtr->pulseStretchDivider = 1000000; break;
            case CPSS_LED_PULSE_STRETCH_4_E:
                ledConfHwPtr->pulseStretchDivider = 1500000; break;
            case CPSS_LED_PULSE_STRETCH_5_E:
                ledConfHwPtr->pulseStretchDivider = 2000000; break;
            case CPSS_LED_PULSE_STRETCH_6_E:
                ledConfHwPtr->pulseStretchDivider = 2500000; break;
            case CPSS_LED_PULSE_STRETCH_7_E:
                ledConfHwPtr->pulseStretchDivider = 3000000; break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (ledConfHwPtr->pulseStretchDivider >= BIT_30)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        switch (ledConfHwPtr->pulseStretchDivider)
        {
            case 10000:
                ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_1_E; break;
            case 500000:
                ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_2_E; break;
            case 1000000:
                ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_3_E; break;
            case 1500000:
                ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_4_E; break;
            case 2000000:
                ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_5_E; break;
            case 2500000:
                ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_6_E; break;
            case 3000000:
                ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_7_E; break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFalconLedBinkDutyCycleConvert function
* @endinternal
*
* @brief   LED duty cycle of the blink signal validity check and convert
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[inout] ledConfPtr         - (pointer to) LED stream configuration structure
* @param[inout] ledConfHwPtr       - (pointer to) LED stream HW structure
* @param[in] hwConvert             - conversion direction (from/to SW/HW structure)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - not valid HW data
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChFalconLedBinkDutyCycleConvert
(
    INOUT   CPSS_LED_CONF_STC                   *ledConfPtr,
    INOUT   PRV_CPSS_FALCON_LED_CONF_HW_STC     *ledConfHwPtr,
    IN      GT_BOOL                             hwConvert
)
{
    if (hwConvert == GT_TRUE)
    {
        switch (ledConfPtr->blink0DutyCycle)
        {
            case CPSS_LED_BLINK_DUTY_CYCLE_0_E:
                ledConfHwPtr->blink0DutyCycle = 0; break;
            case CPSS_LED_BLINK_DUTY_CYCLE_1_E:
            case CPSS_LED_BLINK_DUTY_CYCLE_2_E:
                ledConfHwPtr->blink0DutyCycle = 1; break;
            case CPSS_LED_BLINK_DUTY_CYCLE_3_E:
                ledConfHwPtr->blink0DutyCycle = 2; break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch (ledConfPtr->blink1DutyCycle)
        {
            case CPSS_LED_BLINK_DUTY_CYCLE_0_E:
                ledConfHwPtr->blink1DutyCycle = 0; break;
            case CPSS_LED_BLINK_DUTY_CYCLE_1_E:
            case CPSS_LED_BLINK_DUTY_CYCLE_2_E:
                ledConfHwPtr->blink1DutyCycle = 1; break;
            case CPSS_LED_BLINK_DUTY_CYCLE_3_E:
                ledConfHwPtr->blink1DutyCycle = 2; break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        switch (ledConfHwPtr->blink0DutyCycle)
        {
            case 0:
                ledConfPtr->blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_0_E; break;
            case 1:
                ledConfPtr->blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_1_E; break;
            case 2:
                ledConfPtr->blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_3_E; break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        switch (ledConfHwPtr->blink1DutyCycle)
        {
            case 0:
                ledConfPtr->blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_0_E; break;
            case 1:
                ledConfPtr->blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_1_E; break;
            case 2:
                ledConfPtr->blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_3_E; break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFalconLedOrganizeConvert function
* @endinternal
*
* @brief   LED stream organization mode validity check and convert
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[inout] ledConfPtr         - (pointer to) LED stream configuration structure
* @param[inout] ledConfHwPtr       - (pointer to) LED stream HW structure
* @param[in] hwConvert             - conversion direction (from/to SW/HW structure)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - not valid HW data
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChFalconLedOrganizeConvert
(
    INOUT   CPSS_LED_CONF_STC                   *ledConfPtr,
    INOUT   PRV_CPSS_FALCON_LED_CONF_HW_STC     *ledConfHwPtr,
    IN      GT_BOOL                             hwConvert
)
{
    if (hwConvert == GT_TRUE)
    {
        switch (ledConfPtr->ledOrganize)
        {
            case CPSS_LED_ORDER_MODE_BY_PORT_E:
                ledConfHwPtr->ledOrganize = 0;
                break;
            case CPSS_LED_ORDER_MODE_BY_CLASS_E:
                ledConfHwPtr->ledOrganize = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        switch(ledConfHwPtr->ledOrganize)
        {
            case 0:
                ledConfPtr->ledOrganize = CPSS_LED_ORDER_MODE_BY_PORT_E;
                break;
            case 1:
                ledConfPtr->ledOrganize = CPSS_LED_ORDER_MODE_BY_CLASS_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChSip6LedStreamFirstLedUnitGet function
* @endinternal
*
* @brief   Gets first LED unit index in LED stream chain.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[out] firstLedUnitPtr      - (pointer to) first LED unit in the stream chain
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - not valid HW data
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS prvCpssDxChSip6LedStreamFirstLedUnitGet
(
    IN  GT_U8                               devNum,
    OUT GT_U32                              *firstLedUnitPtr
)
{
    GT_STATUS rc;                                   /* return status */
    CPSS_LED_CONF_STC   ledConf;                    /* LED configuration parameters */
    PRV_CPSS_FALCON_LED_CONF_HW_STC   ledHwConf;    /* LED HW structure */

    ledHwConf.firstActiveLedUnitInChain = NOT_USED_CNS;
    rc = prvCpssDxChFalconLedStreamConfigGet(devNum, &ledConf, &ledHwConf);
    if(rc != GT_OK)
    {
        return rc;
    }
    if (ledHwConf.firstActiveLedUnitInChain != NOT_USED_CNS)
    {
        /* Get first not bypassed LED unit in stream chain */
        *firstLedUnitPtr = ledHwConf.firstActiveLedUnitInChain;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChSip6LedUnitInChainGet function
* @endinternal
*
* @brief   Gets location of the LED unit in chain by its number
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  ledUnitNumber             - LED unit number
* @param[in]  ledHwConfPtr              - (pointer to) LED unit HW configuration
* @param[out] ledInChainPtr             - (pointer to) LED unit location in chain
*
*/
static GT_VOID prvCpssDxChSip6LedUnitInChainGet
(
    IN  GT_U32                              ledUnitNumber,
    IN  PRV_CPSS_FALCON_LED_CONF_HW_STC     *ledHwConfPtr,
    OUT LED_CHAIN_ORDER                     *ledInChainPtr
)
{
    GT_U32  firstLedUnit;           /* Index of the first LED unit in chain */
    GT_U32  lastLedUnit;            /* Index of the last LED unit in chain */

    /* Get first not bypassed LED unit in stream chain */
    firstLedUnit = ledHwConfPtr->firstActiveLedUnitInChain;

    /* Get last not bypassed LED unit in stream chain -
       In Falcon device the order of LED units comes in opposite order. The last LED unit in chain is the first iteration index == 0.
            (E.g. for 4 tiles LED chain: GP12->GP13->GP14->GP15->GP11->GP10->GP9->GP8->GP4->GP5->GP6->GP7->GP3->GP2->GP1->GP0)
       In AC5P and above devices, the last LED unit is the last iteration index == num_of_led_units - 1 */
    lastLedUnit = ledHwConfPtr->lastActiveLedUnitInChain;

    if (ledUnitNumber == firstLedUnit && ledUnitNumber == lastLedUnit)
    {
        *ledInChainPtr = FIRST_LAST_LED_E;
    }
    else
    {
        if (ledUnitNumber == lastLedUnit)
        {
            *ledInChainPtr = LAST_LED_E;
        }
        else
        if (ledUnitNumber == firstLedUnit)
        {
            *ledInChainPtr = FIRST_LED_E;
        }
        else
        {
            *ledInChainPtr = OTHER_LED_E;
        }
    }
}

/**
* @internal prvCpssDxChSip6LedUnitConfigConvert function
* @endinternal
*
* @brief   LED stream SIP6 LED unit specific configurations validity check and convert
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[inout] ledConfPtr         - (pointer to) LED stream configuration structure
* @param[inout] ledConfHwPtr       - (pointer to) LED stream HW structure
* @param[in] ledUnitIndex          - LED unit index
* @param[in] hwConvert             - conversion direction:
*                                       GT_TRUE  - converts from SW to HW structure
*                                       GT_FALSE - converts from HW to SW structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - not valid HW data
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChSip6LedUnitConfigConvert
(
    IN      GT_U8                               devNum,
    INOUT   CPSS_LED_CONF_STC                  *ledConfPtr,
    INOUT   PRV_CPSS_FALCON_LED_CONF_HW_STC    *ledConfHwPtr,
    IN      GT_U32                              ledUnitIndex,
    IN      GT_BOOL                             hwConvert
)
{
    GT_U32  ledUnitNumber;              /* LED unit number */

    LED_UNIT_NUMBER_BY_INDEX_GET_MAC(devNum, ledUnitIndex, ledUnitNumber);

    if (hwConvert == GT_TRUE)
    {
        if (ledConfPtr->sip6LedConfig.ledStart[ledUnitIndex] > 255 ||
            ledConfPtr->sip6LedConfig.ledEnd[ledUnitIndex] > 255)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        ledConfHwPtr->ledStart[ledUnitIndex] = ledConfPtr->sip6LedConfig.ledStart[ledUnitIndex];
        ledConfHwPtr->ledEnd[ledUnitIndex] = ledConfPtr->sip6LedConfig.ledEnd[ledUnitIndex];
        ledConfHwPtr->ledChainBypass[ledUnitIndex] = BOOL2BIT_MAC(ledConfPtr->sip6LedConfig.ledChainBypass[ledUnitIndex]);

        /* Initialize the first and the last active LED units in the chain.
           (E.g. For Falcon devices starting from LAST  : led_index 0 = LAST, led_index 7 = FIRST
                 For AC5X device starting from FIRST    : led_index 0 = FIRST, led_index 4 = LAST) */
        if (ledConfHwPtr->lastActiveLedUnitInChain == NOT_USED_CNS)
        {
            /* The first NOT bypassed LED unit in the chain */
            if (ledConfHwPtr->ledChainBypass[ledUnitIndex] == GT_FALSE)
            {
                /* Set the last and the first LED units */
                ledConfHwPtr->lastActiveLedUnitInChain = ledUnitNumber;
                ledConfHwPtr->firstActiveLedUnitInChain = ledUnitNumber;
            }
        }
        else
        {
            /* The next NON bypassed unit */
            if (ledConfHwPtr->ledChainBypass[ledUnitIndex] == GT_FALSE)
            {
                switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
                {
                    case CPSS_PP_FAMILY_DXCH_FALCON_E:
                        /* Update the first NOT bypassed unit
                          (For Falcon devices the first LED unit is the highest DB index) */
                        ledConfHwPtr->firstActiveLedUnitInChain = ledUnitNumber;
                        break;
                    default:
                        /* Update the last NOT bypassed unit
                           (For AC5X and above devices the last LED unit is the highest DB index) */
                        ledConfHwPtr->lastActiveLedUnitInChain = ledUnitNumber;
                        break;
                }
            }
        }
    }
    else
    {
        ledConfPtr->sip6LedConfig.ledStart[ledUnitIndex] = ledConfHwPtr->ledStart[ledUnitIndex];
        ledConfPtr->sip6LedConfig.ledEnd[ledUnitIndex] = ledConfHwPtr->ledEnd[ledUnitIndex];
        ledConfPtr->sip6LedConfig.ledChainBypass[ledUnitIndex] = BIT2BOOL_MAC(ledConfHwPtr->ledChainBypass[ledUnitIndex]);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFalconLedStreamConfigParamToHwConvert function
* @endinternal
*
* @brief   Check LED configuration parameters and convert to HW data.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] ledConfPtr            - (pointer to) LED stream configuration to be converted
* @param[out] ledConfHwPtr         - (pointer to) LED stream HW converted structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChFalconLedStreamConfigParamToHwConvert
(
    IN  GT_U8                               devNum,
    IN  CPSS_LED_CONF_STC                   *ledConfPtr,
    OUT PRV_CPSS_FALCON_LED_CONF_HW_STC     *ledConfHwPtr
)
{
    GT_U32  ledUnitIndex;             /* LED unit index */
    GT_STATUS  rc;                    /* return status */
    GT_U32  ledUnitNumber;            /* LED unit number */

    CPSS_NULL_PTR_CHECK_MAC(ledConfPtr);
    CPSS_NULL_PTR_CHECK_MAC(ledConfHwPtr);

    cpssOsMemSet(ledConfHwPtr, 0, sizeof(PRV_CPSS_FALCON_LED_CONF_HW_STC));

    /* The function must be first in conversion sequence.
       The ratioo of clock divider will be converted first and then used in the subsequente conversions */
    rc = prvCpssDxChFalconLedClockFrequencyConvert(devNum, ledConfPtr, ledConfHwPtr, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChFalconLedPulseStretchConvert(ledConfPtr, ledConfHwPtr, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* The slowest blink duration the device can get is ~1200ms */
    rc = prvCpssDxChFalconLedBlinkDurationConvert(devNum, ledConfPtr, ledConfHwPtr, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChFalconLedBinkDutyCycleConvert(ledConfPtr, ledConfHwPtr, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChFalconLedOrganizeConvert(ledConfPtr, ledConfHwPtr, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    ledConfHwPtr->ledChainSize = 0;
    ledConfHwPtr->firstActiveLedUnitInChain = NOT_USED_CNS;
    ledConfHwPtr->lastActiveLedUnitInChain = ledConfHwPtr->firstActiveLedUnitInChain;

    for(ledUnitIndex = 0; ledUnitIndex < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledUnitIndex++)
    {
        rc = prvCpssDxChSip6LedUnitConfigConvert(devNum, ledConfPtr, ledConfHwPtr, ledUnitIndex, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Calculate LED chain size based on all not bypassed LEDs */
    for(ledUnitIndex = 0; ledUnitIndex < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledUnitIndex++)
    {
        prvCpssDxChSip6LedUnitNumberByIndexGet(devNum, ledUnitIndex, &ledUnitNumber);
        if (ledUnitNumber == NOT_USED_CNS)
        {
            continue;
        }

        /* LED unit is used in calculating of the LED chain size */
        if (ledConfPtr->sip6LedConfig.ledChainBypass[ledUnitIndex] == GT_FALSE)
        {
            if (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles < 4)
            {
                /* First LED unit generates the stream and should be skipped in calculating of the LED chain size */
                if (ledUnitNumber != ledConfHwPtr->firstActiveLedUnitInChain)
                {
                    ledConfHwPtr->ledChainSize += (ledConfHwPtr->ledEnd[ledUnitIndex] - ledConfHwPtr->ledStart[ledUnitIndex]) + 1;
                }
            }
            else
            {
                /* Falcon 12.8 - Calculate LED chain size based on all LED units */
                ledConfHwPtr->ledChainSize += (ledConfHwPtr->ledEnd[ledUnitIndex] - ledConfHwPtr->ledStart[ledUnitIndex]) + 1;
            }
        }
    }

    if (ledConfHwPtr->firstActiveLedUnitInChain == NOT_USED_CNS)
    {
        /* No active LED units in LED chain */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    return GT_OK;
}

/* Convert HW data to LED SW structure */
static GT_STATUS prvCpssDxChFalconLedStreamHwToSwConfigConvert
(
    IN  GT_U8                            devNum,
    IN  PRV_CPSS_FALCON_LED_CONF_HW_STC *ledConfHwPtr,
    OUT CPSS_LED_CONF_STC               *ledConfPtr
)
{
    GT_U32      ledIndex;               /* LED index */
    GT_STATUS   rc;                     /* return status */

    rc = prvCpssDxChFalconLedClockFrequencyConvert(devNum, ledConfPtr, ledConfHwPtr, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChFalconLedPulseStretchConvert(ledConfPtr, ledConfHwPtr, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChFalconLedBlinkDurationConvert(devNum, ledConfPtr, ledConfHwPtr, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChFalconLedBinkDutyCycleConvert(ledConfPtr, ledConfHwPtr, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChFalconLedOrganizeConvert(ledConfPtr, ledConfHwPtr, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    for(ledIndex = 0; ledIndex < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledIndex++)
    {
        rc = prvCpssDxChSip6LedUnitConfigConvert(devNum, ledConfPtr, ledConfHwPtr, ledIndex, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChSip6LedUnitRestore function
* @endinternal
*
* @brief   Restore LED stream SIP6 LED unit in the end of stretch configurations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     - device number
* @param[in] ledConfHwPtr               - (pointer to) LED stream HW structure
* @param[in] ledUnitIndex               - LED unit index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - not valid HW data
* @retval GT_FAIL                  - otherwise
*
* @note When modifying the stretch from the default value, a long stretch might occur.
*       The duration of the stretch depends on the LED clock (WA: Jira CPSS-11140)
*/
static GT_STATUS prvCpssDxChSip6LedUnitRestore
(
    IN  GT_U8                               devNum,
    IN  PRV_CPSS_FALCON_LED_CONF_HW_STC    *ledHwConfPtr,
    IN  GT_U32                              ledUnitIndex
)
{
    GT_STATUS   rc;                         /* return status */
    GT_U32      ledUnitNumber;              /* LED unit  */
    GT_U32      ledUnitRegDbIndex;          /* LED unit register DB index */
    GT_U32      regAddr;                    /* register address */
    LED_CHAIN_ORDER  ledInChain;            /* LED unit location in chain */

    LED_UNIT_NUMBER_BY_INDEX_GET_MAC(devNum, ledUnitIndex, ledUnitNumber);

    prvCpssDxChSip6LedUnitInChainGet(ledUnitNumber, ledHwConfPtr, &ledInChain);

    ledUnitRegDbIndex = LED_REG_DB_INDEX_BY_UNIT_ID(devNum, ledUnitNumber);

    /* Skip LED unit configurations except the first and the last LED unit in the chain */
    if (ledHwConfPtr->ledChainBypass[ledUnitIndex] && (ledInChain == OTHER_LED_E))
    {
        /* Skip bypassed LED unit */
        return GT_OK;
    }

    /* 5 - Configure the clock to original value */
    /* LED Chain Clock Control */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDChainClockControl;
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 11, 12, ledHwConfPtr->ledClockFrequencyDivider);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* 6 - Configure the LED unit to non-bypass */
    /* LED Chain Stream Control 0 */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDChainStreamControl0;

    return prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, 0);
}

/**
* @internal prvCpssDxChSip6LedUnitStretchSet function
* @endinternal
*
* @brief   Sets LED stream SIP6 LED unit stretch configurations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     - device number
* @param[in] ledConfHwPtr               - (pointer to) LED stream HW structure
* @param[in] ledUnitRegDbIndex          - LED unit register DB index
* @param[in] ledInChain                 - the LED place in chain
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - not valid HW data
* @retval GT_FAIL                  - otherwise
*
* @note When modifying the stretch from the default value, a long stretch might occur.
*       The duration of the stretch depends on the LED clock (WA: Jira CPSS-11140)
*/
static GT_STATUS prvCpssDxChSip6LedUnitStretchSet
(
    IN  GT_U8                               devNum,
    IN  PRV_CPSS_FALCON_LED_CONF_HW_STC    *ledHwConfPtr,
    IN  GT_U32                              ledUnitRegDbIndex,
    IN  GT_U32                              ledInChain
)
{
    GT_STATUS rc;                       /* return status */
    GT_U32  pulseStretchDivider;        /* pulse stretch HW value */
    GT_U32  ledClockFrequencyDivider;   /* LED clock frequency divider HW value */
    GT_U32  regAddr;                    /* register address */
    GT_U32  ledUnitIndex;               /* LED unit index */
    GT_BOOL restoreLedUnit;

    /* Pulse Stretch Division */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].pulseStretchDivision;
    /* Pulse stretch divider */
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 30, &pulseStretchDivider);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* The stretch value has not been set yet - no need workaround */
    if (pulseStretchDivider == 0)
    {
        /* Pulse Stretch Division */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].pulseStretchDivision;
        return prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 30, ledHwConfPtr->pulseStretchDivider);
    }

    if (ledHwConfPtr->pulseStretchDivider == pulseStretchDivider)
    {
        /* Nothing to update */
        return GT_OK;
    }

    /* Apply WA sequence */

    /*  1 - Configure the LED unit to bypass */
    /* LED Chain Stream Control 0 */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDChainStreamControl0;
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, 1);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* 2 - Increase the internal led clock to maximal value - sys_clk/2 */
    ledClockFrequencyDivider = 2;

    /* LED Chain Clock Control */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDChainClockControl;
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 11, 12, ledClockFrequencyDivider);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* 3 - Configure the stretch */
    /* Pulse Stretch Division */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].pulseStretchDivision;
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 30, ledHwConfPtr->pulseStretchDivider);
    if (rc != GT_OK)
    {
        return rc;
    }

    restoreLedUnit = GT_FALSE;
    if (PRV_CPSS_DXCH_PP_HW_INFO_RAVENS_MAC(devNum).numOfRavens)
    {
        /* Restore LED units configuration.
           In Falcon devices the first LED Unit is the last in the LED chain iteration loop */
        if (ledInChain == FIRST_LED_E || ledInChain == FIRST_LAST_LED_E)
        {
            /* 4 - Wait for 2.7 seconds */
            cpssOsTimerWkAfter(2700);

            restoreLedUnit = GT_TRUE;
        }
    }
    else
    {
        /* Restore LED units configuration.
           In NOT Falcon devices the last LED unit is the last in the LED chain iteration loop */
        if (ledInChain == LAST_LED_E || ledInChain == FIRST_LAST_LED_E)
        {
            /* 4 - Wait for 2.7 seconds */
            cpssOsTimerWkAfter(2700);

            restoreLedUnit = GT_TRUE;
        }
    }

    if (restoreLedUnit == GT_TRUE)
    {
        for(ledUnitIndex = 0; ledUnitIndex < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledUnitIndex++)
        {
            rc = prvCpssDxChSip6LedUnitRestore(devNum, ledHwConfPtr, ledUnitIndex);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChSip6LedUnitConfigSet function
* @endinternal
*
* @brief   Sets LED stream SIP6 LED unit specific configurations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] ledConfHwPtr          - (pointer to) LED stream HW structure
* @param[in] ledUnitIndex          - LED unit index
* @param[inout] resetFirstLedUnitPtr  - (pointer to) first LED unit reset status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - not valid HW data
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChSip6LedUnitConfigSet
(
    IN    GT_U8                                 devNum,
    IN    PRV_CPSS_FALCON_LED_CONF_HW_STC      *ledHwConfPtr,
    IN    GT_U32                                ledUnitIndex,
    INOUT GT_BOOL                              *resetFirstLedUnitPtr
)
{
    GT_STATUS rc;                   /* return status */
    GT_U32  regData;                /* register led control value */
    GT_U32  regAddr;                /* register address */
    GT_U32  ledUnitNumber;          /* LED unit number */
    GT_U32  ledUnitRegDbIndex;      /* LED unit register DB index */
    LED_CHAIN_ORDER  ledInChain;    /* LED unit location in chain */
    GT_U32  ledIndicationGapCompensation; /* Additional LED indication used to separate two LED streams in strobe */

    switch (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        case 4:
            ledIndicationGapCompensation = FALCON_12_8_LED_INDICATION_GAP_COMPENSATION_CNS;
            break;
        case 2:
            ledIndicationGapCompensation = FALCON_6_4_LED_INDICATION_GAP_COMPENSATION_CNS;
            break;
        case 1:
            ledIndicationGapCompensation = 0;
            break;
        default:
            /* AC5X and above */
            ledIndicationGapCompensation = AC5X_LED_INDICATION_GAP_COMPENSATION_CNS;
    }

    LED_UNIT_NUMBER_BY_INDEX_GET_MAC(devNum, ledUnitIndex, ledUnitNumber);

    ledUnitRegDbIndex = LED_REG_DB_INDEX_BY_UNIT_ID(devNum, ledUnitNumber);

    if (ledUnitRegDbIndex >= SIP_6_MAX_LED_UNIT_NUM_CNC)
    {
        /* should never happen */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    prvCpssDxChSip6LedUnitInChainGet(ledUnitNumber, ledHwConfPtr, &ledInChain);

    /* LED unit is bypassed - no need further configurations */
    if (ledHwConfPtr->ledChainBypass[ledUnitIndex])
    {
        /* LED Chain Stream Control 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDChainStreamControl0;
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if( rc != GT_OK )
        {
            return rc;
        }

        /* Bypass the LED unit */
        U32_SET_FIELD_MAC(regData, 0, 1, 1);

        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if( rc != GT_OK )
        {
            return rc;
        }

        *resetFirstLedUnitPtr = GT_TRUE;

        /* Skip LED unit configurations */
        return GT_OK;
    }

    /* LED Chain Clock Control */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDChainClockControl;
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 11, 12, ledHwConfPtr->ledClockFrequencyDivider);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Only in last Raven LED */
    if (ledInChain == LAST_LED_E || ledInChain == FIRST_LAST_LED_E)
    {
        /* Sample LED stream output on neg-edge of the LED Clock Out */
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 23, 1, 1);
        if( rc != GT_OK )
        {
            return rc;
        }
    }

    /* Pulse Stretch Division WA */
    rc = prvCpssDxChSip6LedUnitStretchSet(devNum, ledHwConfPtr, ledUnitRegDbIndex, ledInChain);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Blink Division 0 */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].blinkDivision0;
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 30, ledHwConfPtr->blink0DurationDivider);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Blink Division 1 */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].blinkDivision1;
    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 30, ledHwConfPtr->blink1DurationDivider);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Blink Global Control */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].blinkGlobalControl;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    U32_SET_FIELD_MAC(regData , 3, 2, ledHwConfPtr->blink0DutyCycle);
    U32_SET_FIELD_MAC(regData , 8, 2, ledHwConfPtr->blink1DutyCycle);

    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Configure LED chain size at first unit */
    if (ledInChain == FIRST_LED_E || ledInChain == FIRST_LAST_LED_E)
    {
        /* LED Chain Stream Control 1 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDChainStreamControl1;

        /* Number of app_clk cycles between 2 LED stream generated by the first LED unit in the chain */
        regData =  (ledHwConfPtr->ledChainSize + ledIndicationGapCompensation) * ledHwConfPtr->ledClockFrequencyDivider;

        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 28, regData);
        if( rc != GT_OK )
        {
            return rc;
        }

    }

    /* LED Chain Stream Control 0 */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDChainStreamControl0;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    switch (ledInChain)
    {
        case FIRST_LAST_LED_E:
            /* Enable first LED unit */
            U32_SET_FIELD_MAC(regData, 1, 1, 1);
            /* Only first bit in strobe */
            U32_SET_FIELD_MAC(regData, 13, 1, 1);
            break;
        case FIRST_LED_E:
            /* Enable first LED unit */
            U32_SET_FIELD_MAC(regData, 1, 1, 1);
            U32_SET_FIELD_MAC(regData, 13, 1, 0);
            break;
        case LAST_LED_E:
            U32_SET_FIELD_MAC(regData, 1, 1, 0);
            /* Only first bit in strobe */
            U32_SET_FIELD_MAC(regData, 13, 1, 1);
            break;
        default:
            U32_SET_FIELD_MAC(regData, 1, 1, 0);
            U32_SET_FIELD_MAC(regData, 13, 1, 0);
    }

    /* Set LED unit bypass bit */
    U32_SET_FIELD_MAC(regData, 0, 1, ledHwConfPtr->ledChainBypass[ledUnitIndex]);

    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* LED Control */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDControl;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* End of LED stream location */
    U32_SET_FIELD_MAC(regData, 13, 8, ledHwConfPtr->ledEnd[ledUnitIndex]);

    /* Start of LED stream location */
    U32_SET_FIELD_MAC(regData, 5, 8, ledHwConfPtr->ledStart[ledUnitIndex]);

    /* Organize Mode */
    U32_SET_FIELD_MAC(regData, 1, 1, ledHwConfPtr->ledOrganize);

    /* Reset LED */
    U32_SET_FIELD_MAC(regData, 0, 1, 0);

    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Normal LED operation */
    return prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, 1);
}

/**
* @internal prvCpssDxChFalconLedStreamConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] ledConfPtr            - (pointer to) LED stream configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChFalconLedStreamConfigSet
(
    IN  GT_U8               devNum,
    IN  CPSS_LED_CONF_STC   *ledConfPtr
)
{
    GT_STATUS rc;                   /* return status */
    GT_U32  regData;                /* register led control value */
    GT_U32  regAddr;                /* register address */
    GT_U32  ledIndex;               /* LED index */
    PRV_CPSS_FALCON_LED_CONF_HW_STC   ledHwConf;  /* LED HW structure */
    PRV_CPSS_FALCON_LED_CONF_HW_STC   *ledHwConfPtr = &ledHwConf;
    GT_U32  ledUnitRegDbIndex;      /* LED unit register DB index */
    GT_BOOL resetFirstLedUnit;

    rc = prvCpssDxChFalconLedStreamConfigParamToHwConvert(devNum, ledConfPtr, ledHwConfPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get first not bypassed LED unit in stream chain */
    ledUnitRegDbIndex = LED_REG_DB_INDEX_BY_UNIT_ID(devNum, ledHwConfPtr->firstActiveLedUnitInChain);

    /* Disable first LED unit */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDChainStreamControl0;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Non-fisrt - default */
    U32_SET_FIELD_MAC(regData,  1, 1, 0);
    /* Last in chain - default */
    U32_SET_FIELD_MAC(regData, 13, 1, 1);

    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    resetFirstLedUnit = GT_FALSE;

    for(ledIndex = 0; ledIndex < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledIndex++)
    {
        rc = prvCpssDxChSip6LedUnitConfigSet(devNum, ledHwConfPtr, ledIndex, &resetFirstLedUnit);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Reset the first LED unit */
    if (resetFirstLedUnit == GT_TRUE)
    {
        /* LED Control */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDControl;
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if( rc != GT_OK )
        {
            return rc;
        }

        /* Reset LED */
        U32_SET_FIELD_MAC(regData, 0, 1, 0);

        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if( rc != GT_OK )
        {
            return rc;
        }

        /* Normal LED operation */
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, 1);
        if( rc != GT_OK )
        {
            return rc;
        }
    }


    return GT_OK;
}

/**
* @internal internal_cpssDxChLedStreamPortGroupConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANG           - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamPortGroupConfigSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              ledInterfaceNum,
    IN  CPSS_LED_CONF_STC   *ledConfPtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr;            /* register address */
    GT_U32  data;               /* local data */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(ledConfPtr);
    LED_INTERFACE_NUM_CHECK_MAC(devNum,ledInterfaceNum);

    if( (ledConfPtr->ledStart > 255) || (ledConfPtr->ledEnd > 255) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* Falcon LED support */
            return prvCpssDxChFalconLedStreamConfigSet(devNum, ledConfPtr);
        }
        else
        {
            return prvCpssDxChBobcat2LedStreamConfigSet(devNum, ledInterfaceNum, ledConfPtr);
        }
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    /* registers arrays boundary additional check */
    XCAT3_LED_INTERFACE_NUM_CHECK_MAC(ledInterfaceNum);

    regAddr  = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledControl[ledInterfaceNum];

    regData = 0;

    U32_SET_FIELD_MAC(regData , 0, 8, ledConfPtr->ledStart);
    U32_SET_FIELD_MAC(regData , 8, 8, ledConfPtr->ledEnd);

    switch(ledConfPtr->pulseStretch)
    {
        case CPSS_LED_PULSE_STRETCH_0_NO_E: data = 0;
                                            break;
        case CPSS_LED_PULSE_STRETCH_1_E:    data = 1;
                                            break;
        case CPSS_LED_PULSE_STRETCH_2_E:    data = 2;
                                            break;
        case CPSS_LED_PULSE_STRETCH_3_E:    data = 3;
                                            break;
        case CPSS_LED_PULSE_STRETCH_4_E:    data = 4;
                                            break;
        case CPSS_LED_PULSE_STRETCH_5_E:    data = 5;
                                            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData , 16, 3, data);


    switch(ledConfPtr->blink0Duration)
    {
        case CPSS_LED_BLINK_DURATION_0_E:   data = 0;
                                            break;
        case CPSS_LED_BLINK_DURATION_1_E:   data = 1;
                                            break;
        case CPSS_LED_BLINK_DURATION_2_E:   data = 2;
                                            break;
        case CPSS_LED_BLINK_DURATION_3_E:   data = 3;
                                            break;
        case CPSS_LED_BLINK_DURATION_4_E:   data = 4;
                                            break;
        case CPSS_LED_BLINK_DURATION_5_E:   data = 5;
                                            break;
        case CPSS_LED_BLINK_DURATION_6_E:   data = 6;
                                            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData , 19, 3, data);

    switch(ledConfPtr->blink0DutyCycle)
    {
        case CPSS_LED_BLINK_DUTY_CYCLE_0_E: data = 0;
                                            break;
        case CPSS_LED_BLINK_DUTY_CYCLE_1_E: data = 1;
                                            break;
        case CPSS_LED_BLINK_DUTY_CYCLE_2_E: data = 2;
                                            break;
        case CPSS_LED_BLINK_DUTY_CYCLE_3_E: data = 3;
                                            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData , 22, 2, data);

    switch(ledConfPtr->blink1Duration)
    {
        case CPSS_LED_BLINK_DURATION_0_E:   data = 0;
                                            break;
        case CPSS_LED_BLINK_DURATION_1_E:   data = 1;
                                            break;
        case CPSS_LED_BLINK_DURATION_2_E:   data = 2;
                                            break;
        case CPSS_LED_BLINK_DURATION_3_E:   data = 3;
                                            break;
        case CPSS_LED_BLINK_DURATION_4_E:   data = 4;
                                            break;
        case CPSS_LED_BLINK_DURATION_5_E:   data = 5;
                                            break;
        case CPSS_LED_BLINK_DURATION_6_E:   data = 6;
                                            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData , 24, 3, data);

    switch(ledConfPtr->blink1DutyCycle)
    {
        case CPSS_LED_BLINK_DUTY_CYCLE_0_E: data = 0;
                                            break;
        case CPSS_LED_BLINK_DUTY_CYCLE_1_E: data = 1;
                                            break;
        case CPSS_LED_BLINK_DUTY_CYCLE_2_E: data = 2;
                                            break;
        case CPSS_LED_BLINK_DUTY_CYCLE_3_E: data = 3;
                                            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData , 27, 2, data);

    U32_SET_FIELD_MAC(regData , 29, 1, BOOL2BIT_MAC(ledConfPtr->disableOnLinkDown));

    switch(ledConfPtr->ledOrganize)
    {
        case CPSS_LED_ORDER_MODE_BY_PORT_E:     data = 0 ;
                                                break;
        case CPSS_LED_ORDER_MODE_BY_CLASS_E:    data = 1 ;
                                                break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData , 30, 1, data);

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 0, 31, regData);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    regAddr  = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledClass[ledInterfaceNum][6];

    regData = 0;

    U32_SET_FIELD_MAC(regData , 16, 1, ((ledConfPtr->clkInvert == GT_TRUE)?1:0));

    switch(ledConfPtr->class5select)
    {
        case CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E:     data = 0 ;
                                                        break;
        case CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E:   data = 1 ;
                                                        break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData , 18, 1, data);

    switch(ledConfPtr->class13select)
    {
        case CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E:      data = 0 ;
                                                        break;
        case CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E: data = 1 ;
                                                        break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData , 19, 1, data);

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum, portGroupId, regAddr, 0xD0000, regData);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChLedStreamPortGroupConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANG           - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortGroupConfigSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              ledInterfaceNum,
    IN  CPSS_LED_CONF_STC   *ledConfPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamPortGroupConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, ledInterfaceNum, ledConfPtr));

    rc = internal_cpssDxChLedStreamPortGroupConfigSet(devNum, portGroupsBmp, ledInterfaceNum, ledConfPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, ledInterfaceNum, ledConfPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChBobcat2LedStreamConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
*
* @param[out] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChBobcat2LedStreamConfigGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              ledInterfaceNum,
    OUT CPSS_LED_CONF_STC   *ledConfPtr
)
{
    GT_STATUS rc;           /* return status */
    GT_U32  regData;        /* register led control value */
    GT_U32  regAddr;        /* register address */
    GT_U32  blink;          /* blink index */
    CPSS_LED_BLINK_DUTY_CYCLE_ENT   *blinkDutyCyclePtr; /* (pointer to) duty cycle of the blink signal */
    CPSS_LED_BLINK_DURATION_ENT     *blinkDurationPtr; /* (pointer to) blink frequency divider */


    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(ledConfPtr, 0, sizeof(CPSS_LED_CONF_STC));

    /* LED control */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].LEDControl;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    ledConfPtr->ledStart = U32_GET_FIELD_MAC(regData, 8,  8);
    ledConfPtr->ledEnd = U32_GET_FIELD_MAC(regData, 16, 8);
    switch(U32_GET_FIELD_MAC(regData, 5, 3))
    {
        case 0:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_0_NO_E;
            break;
        case 1:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_1_E;
            break;
        case 2:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_2_E;
            break;
        case 3:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_3_E;
            break;
        case 4:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_4_E;
            break;
        case 5:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_5_E;
            break;
        case 6:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_6_E;
            break;
        case 7:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_7_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    ledConfPtr->invertEnable = (U32_GET_FIELD_MAC(regData, 4, 1) == 0) ? GT_TRUE : GT_FALSE;

    switch(U32_GET_FIELD_MAC(regData, 2, 2))
    {
        case 0:
            ledConfPtr->ledClockFrequency = CPSS_LED_CLOCK_OUT_FREQUENCY_500_E;
            break;
        case 1:
            ledConfPtr->ledClockFrequency = CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E;
            break;
        case 2:
            ledConfPtr->ledClockFrequency = CPSS_LED_CLOCK_OUT_FREQUENCY_2000_E;
            break;
        case 3:
            ledConfPtr->ledClockFrequency = CPSS_LED_CLOCK_OUT_FREQUENCY_3000_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    switch(U32_GET_FIELD_MAC(regData, 1, 1))
    {
        case 0:
            ledConfPtr->ledOrganize = CPSS_LED_ORDER_MODE_BY_PORT_E;
            break;
        case 1:
            ledConfPtr->ledOrganize = CPSS_LED_ORDER_MODE_BY_CLASS_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Blink global control */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].blinkGlobalControl;
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 10, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    for (blink = 0; blink < 2; blink++)
    {
        if (blink == 0)
        {
            blinkDurationPtr = &ledConfPtr->blink0Duration;
            blinkDutyCyclePtr = &ledConfPtr->blink0DutyCycle;
        }
        else
        {
            blinkDurationPtr = &ledConfPtr->blink1Duration;
            blinkDutyCyclePtr = &ledConfPtr->blink1DutyCycle;
        }
        switch (U32_GET_FIELD_MAC(regData , 0 + (5 * blink), 3))
        {
            case 0:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_0_E;
                break;
            case 1:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_1_E;
                break;
            case 2:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_2_E;
                break;
            case 3:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_3_E;
                break;
            case 4:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_4_E;
                break;
            case 5:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_5_E;
                break;
            case 6:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_6_E;
                break;
            case 7:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_7_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        switch(U32_GET_FIELD_MAC(regData , 3 + (5 * blink), 2))
        {
            case 0:
                *blinkDutyCyclePtr = CPSS_LED_BLINK_DUTY_CYCLE_0_E;
                break;
            case 1:
                *blinkDutyCyclePtr = CPSS_LED_BLINK_DUTY_CYCLE_1_E;
                break;
            case 2:
                *blinkDutyCyclePtr = CPSS_LED_BLINK_DUTY_CYCLE_3_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

    }

    return GT_OK;
}

/**
* @internal prvCpssDxChSip6LedUnitConfigGet function
* @endinternal
*
* @brief   Gets LED stream SIP6 LED unit specific configurations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] ledUnitIndex          - LED unit index
* @param[out] ledConfHwPtr         - (pointer to) LED stream HW structure
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChSip6LedUnitConfigGet
(
    IN    GT_U8                                 devNum,
    IN    GT_U32                                ledUnitIndex,
    OUT   PRV_CPSS_FALCON_LED_CONF_HW_STC      *ledHwConfPtr
)
{
    GT_STATUS rc;                   /* return status */
    GT_U32  regData;                /* register led control value */
    GT_U32  regAddr;                /* register address */
    GT_U32  fieldValue;             /* register field value */
    GT_U32  fieldValue1;            /* register field value */
    GT_U32  ledUnitNumber;          /* LED interface number */
    GT_U32  ledInChain;
    GT_U32  ledUnitRegDbIndex;          /* LED unit register DB index */

    LED_UNIT_NUMBER_BY_INDEX_GET_MAC(devNum, ledUnitIndex, ledUnitNumber);

    ledUnitRegDbIndex = LED_REG_DB_INDEX_BY_UNIT_ID(devNum, ledUnitNumber);

    /* LED Chain Stream Control 0 */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDChainStreamControl0;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    ledHwConfPtr->ledChainBypass[ledUnitIndex] = U32_GET_FIELD_MAC(regData, 0, 1);

    /* LED unit is bypassed  */
    if (ledHwConfPtr->ledChainBypass[ledUnitIndex])
    {
        /* Not configured LED unit */
        return GT_OK;
    }

    /* Identify the LED unit order in chain */
    fieldValue  = U32_GET_FIELD_MAC(regData,  1, 1);
    fieldValue1 = U32_GET_FIELD_MAC(regData, 13, 1);
    if (fieldValue && fieldValue1)
    {
        ledInChain = FIRST_LAST_LED_E;
        ledHwConfPtr->firstActiveLedUnitInChain = ledUnitNumber;
        ledHwConfPtr->lastActiveLedUnitInChain  = ledUnitNumber;
    }
    else
    {
        if (fieldValue)
        {
            ledInChain = FIRST_LED_E;
            ledHwConfPtr->firstActiveLedUnitInChain = ledUnitNumber;
        }
        else
        {
            fieldValue = U32_GET_FIELD_MAC(regData, 13, 1);
            if (fieldValue)
            {
                ledInChain = LAST_LED_E;
                ledHwConfPtr->lastActiveLedUnitInChain = ledUnitNumber;
            }
            else
            {
                ledInChain = OTHER_LED_E;
            }
        }
    }

    if (ledInChain == FIRST_LED_E || ledInChain == FIRST_LAST_LED_E)
    {
        /* Pulse Stretch Division */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].pulseStretchDivision;
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 30, &ledHwConfPtr->pulseStretchDivider);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Blink Division 0 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].blinkDivision0;
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 30, &ledHwConfPtr->blink0DurationDivider);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Blink Division 1 */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].blinkDivision1;
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 0, 30, &ledHwConfPtr->blink1DurationDivider);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Blink Global Control */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].blinkGlobalControl;
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }

        ledHwConfPtr->blink0DutyCycle = U32_GET_FIELD_MAC(regData, 3, 2);
        ledHwConfPtr->blink1DutyCycle = U32_GET_FIELD_MAC(regData, 8, 2);

        /* LED Chain Clock Control */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDChainClockControl;
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 11, 12, &ledHwConfPtr->ledClockFrequencyDivider);
        if( rc != GT_OK )
        {
            return rc;
        }
    }

    /* LED Control */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].LEDControl;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* End of LED stream location */
    ledHwConfPtr->ledEnd[ledUnitIndex] = U32_GET_FIELD_MAC(regData, 13, 8);
    /* Start of LED stream location */
    ledHwConfPtr->ledStart[ledUnitIndex] = U32_GET_FIELD_MAC(regData, 5, 8);

    /* Organize Mode */
    ledHwConfPtr->ledOrganize = U32_GET_FIELD_MAC(regData, 1, 1);

    return GT_OK;
}

/**
* @internal prvCpssDxChFalconLedStreamConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration in SW and HW format.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
*
* @param[out] ledConfPtr           - (pointer to) LED stream configuration parameters.
*
* @param[out] ledHwConfPtr         - (pointer to) LED stream HW structure.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChFalconLedStreamConfigGet
(
    IN  GT_U8               devNum,
    OUT CPSS_LED_CONF_STC   *ledConfPtr,
    OUT PRV_CPSS_FALCON_LED_CONF_HW_STC  *ledHwConfPtr
)
{
    GT_STATUS rc;                   /* return status */
    GT_U32  ledUnitIndex;           /* LED unit index */

    cpssOsMemSet(ledConfPtr, 0, sizeof(CPSS_LED_CONF_STC));

    for(ledUnitIndex = 0; ledUnitIndex < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledUnitIndex++)
    {
        rc = prvCpssDxChSip6LedUnitConfigGet(devNum, ledUnitIndex, ledHwConfPtr);
        if( rc != GT_OK )
        {
            return rc;
        }
    }

    return prvCpssDxChFalconLedStreamHwToSwConfigConvert(devNum, ledHwConfPtr, ledConfPtr);
}

/**
* @internal internal_cpssDxChLedStreamPortGroupConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
*
* @param[out] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamPortGroupConfigGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              ledInterfaceNum,
    OUT CPSS_LED_CONF_STC   *ledConfPtr
)
{
    GT_STATUS rc;           /* return status */
    GT_U32  regData;        /* register led control value */
    GT_U32  regAddr;        /* register address */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_FALCON_LED_CONF_HW_STC  ledConfHw;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(ledConfPtr);
    LED_INTERFACE_NUM_CHECK_MAC(devNum,ledInterfaceNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* Falcon LED support */
            return prvCpssDxChFalconLedStreamConfigGet(devNum, ledConfPtr, &ledConfHw);
        }
        else
        {

            return prvCpssDxChBobcat2LedStreamConfigGet(devNum,ledInterfaceNum,ledConfPtr);
        }
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    /* registers arrays boundary additional check */
    XCAT3_LED_INTERFACE_NUM_CHECK_MAC(ledInterfaceNum);

    regAddr  = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledControl[ledInterfaceNum];

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 0, 31, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    ledConfPtr->ledStart = U32_GET_FIELD_MAC(regData, 0, 8);
    ledConfPtr->ledEnd   = U32_GET_FIELD_MAC(regData, 8, 8);

    switch(U32_GET_FIELD_MAC(regData, 16, 3))
    {
        case 0: ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_0_NO_E;
                break;
        case 1: ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_1_E;
                break;
        case 2: ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_2_E;
                break;
        case 3: ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_3_E;
                break;
        case 4: ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_4_E;
                break;
        case 5: ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_5_E;
                break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch(U32_GET_FIELD_MAC(regData, 19, 3))
    {
        case 0: ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_0_E;
                break;
        case 1: ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_1_E;
                break;
        case 2: ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_2_E;
                break;
        case 3: ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_3_E;
                break;
        case 4: ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_4_E;
                break;
        case 5: ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_5_E;
                break;
        case 6: ledConfPtr->blink0Duration = CPSS_LED_BLINK_DURATION_6_E;
                break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch(U32_GET_FIELD_MAC(regData, 22, 2))
    {
        case 0: ledConfPtr->blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_0_E;
                break;
        case 1: ledConfPtr->blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_1_E;
                break;
        case 2: ledConfPtr->blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_2_E;
                break;
        case 3: ledConfPtr->blink0DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_3_E;
                break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch(U32_GET_FIELD_MAC(regData, 24, 3))
    {
        case 0: ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_0_E;
                break;
        case 1: ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_1_E;
                break;
        case 2: ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_2_E;
                break;
        case 3: ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_3_E;
                break;
        case 4: ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_4_E;
                break;
        case 5: ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_5_E;
                break;
        case 6: ledConfPtr->blink1Duration = CPSS_LED_BLINK_DURATION_6_E;
                break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch(U32_GET_FIELD_MAC(regData, 27, 2))
    {
        case 0: ledConfPtr->blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_0_E;
                break;
        case 1: ledConfPtr->blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_1_E;
                break;
        case 2: ledConfPtr->blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_2_E;
                break;
        case 3: ledConfPtr->blink1DutyCycle = CPSS_LED_BLINK_DUTY_CYCLE_3_E;
                break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    ledConfPtr->disableOnLinkDown = BIT2BOOL_MAC(U32_GET_FIELD_MAC(regData, 29, 1));

    ledConfPtr->ledOrganize = U32_GET_FIELD_MAC(regData, 30, 1) == 0 ?
                                            CPSS_LED_ORDER_MODE_BY_PORT_E :
                                            CPSS_LED_ORDER_MODE_BY_CLASS_E;

    regAddr  = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledClass[ledInterfaceNum][6];

    rc = prvCpssDrvHwPpPortGroupReadRegBitMask(devNum, portGroupId, regAddr, 0xD0000, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    ledConfPtr->clkInvert = U32_GET_FIELD_MAC(regData, 16, 1) == 1 ?
                                                                    GT_TRUE : GT_FALSE;

    ledConfPtr->class5select = U32_GET_FIELD_MAC(regData, 18, 1) == 0 ?
                                                    CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E:
                                                    CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E;

    ledConfPtr->class13select = U32_GET_FIELD_MAC(regData, 19, 1) == 0 ?
                                                    CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E:
                                                    CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E;

    return GT_OK;
}

/**
* @internal cpssDxChLedStreamPortGroupConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
*
* @param[out] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortGroupConfigGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              ledInterfaceNum,
    OUT CPSS_LED_CONF_STC   *ledConfPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamPortGroupConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, ledInterfaceNum, ledConfPtr));

    rc = internal_cpssDxChLedStreamPortGroupConfigGet(devNum, portGroupsBmp, ledInterfaceNum, ledConfPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, ledInterfaceNum, ledConfPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChLedStreamClassManipTriSpeedAux function
* @endinternal
*
* @brief   Auxilary function to calculate register address and offsets for class
*         manipulation configuration of tri-speed (network) ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledInterfaceNum          - LED stream interface number.
* @param[in] classNum                 - class number
*
* @param[out] regAddrPtr               - (pointer to) register address
* @param[out] invertEnableOffsetPtr    - (pointer to) invert enable offset in register.
* @param[out] blinkEnableOffsetPtr     - (pointer to) blink enable offset in register.
* @param[out] blinkSelectOffsetPtr     - (pointer to) blink select offset in register.
* @param[out] forceEnableOffsetPtr     - (pointer to) force enable offset in register.
* @param[out] forceDataOffsetPtr       - (pointer to) force data offset in register.
* @param[out] forceDataLengthPtr       - (pointer to) force data length in register.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChLedStreamClassManipTriSpeedAux
(
    IN  GT_U8   devNum,
    IN  GT_U32  ledInterfaceNum,
    IN  GT_U32  classNum,
    OUT GT_U32  *regAddrPtr,
    OUT GT_U32  *invertEnableOffsetPtr,
    OUT GT_U32  *blinkEnableOffsetPtr,
    OUT GT_U32  *blinkSelectOffsetPtr,
    OUT GT_U32  *forceEnableOffsetPtr,
    OUT GT_U32  *forceDataOffsetPtr,
    OUT GT_U32  *forceDataLengthPtr
)
{
    switch(classNum)
    {
        case 0:
        case 2:
        case 4:
        case 5:
        case 6: *invertEnableOffsetPtr  = 15 ;
                *blinkEnableOffsetPtr   = 13 ;
                *blinkSelectOffsetPtr   = 14 ;
                *forceEnableOffsetPtr   = 12 ;
                *forceDataOffsetPtr     = 0  ;
                break;
        case 1:
        case 3: *invertEnableOffsetPtr  = 31 ;
                *blinkEnableOffsetPtr   = 29 ;
                *blinkSelectOffsetPtr   = 30 ;
                *forceEnableOffsetPtr   = 28 ;
                *forceDataOffsetPtr     = 16 ;
                break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *forceDataLengthPtr = 12;

    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledClass[ledInterfaceNum][classNum];

    return GT_OK;
}

/**
* @internal prvCpssDxChSip6LedUnitClassManipulationSet function
* @endinternal
*
* @brief   Sets LED unit class manipulation parameters
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] ledUnitIndex          - LED unit index
* @param[in] classNum              - class number.
*                                    (APPLICABLE RANGE: 0..5)
* @param[in] classParamsPtr        - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
*/
static GT_STATUS prvCpssDxChSip6LedUnitClassManipulationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledUnitIndex,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;                   /* return status */
    GT_U32  regData;                /* register led control value */
    GT_U32  regAddr;                /* register address */
    GT_U32  regMask;                /* register led control mask */
    GT_U32  fieldOffset;            /* register field offset */
    GT_U32  ledUnitNumber;          /* LED unit number */

    if (classNum >= PRV_CPSS_DXCH_BOBCAT2_LED_INDICATION_CLASS_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    LED_UNIT_NUMBER_BY_INDEX_GET_MAC(devNum, ledUnitIndex, ledUnitNumber);

    /* Blink Global Control Register */
    regData = regMask = 0x0;
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitNumber].blinkGlobalControl;

    /* blinkGlobalControl.Blink En[18-25] position is defined by class Num */
    fieldOffset = 18 + classNum;
    U32_SET_FIELD_MAC(regData , fieldOffset, 1, (classParamsPtr->blinkEnable) ? 1 : 0 );
    U32_SET_FIELD_MAC(regMask , fieldOffset, 1, 1);

    /* blinkGlobalControl.Blink Sel[10-17] position is defined by class Num */
    fieldOffset = 10 + classNum;
    U32_SET_FIELD_MAC(regData , fieldOffset, 1, (classParamsPtr->blinkSelect == CPSS_LED_BLINK_SELECT_0_E) ? 0 : 1);
    U32_SET_FIELD_MAC(regMask , fieldOffset, 1, 1);
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMask, regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Classes and Group Configuration Register, Force En[24:29] position is defined by class Num */
    regData = regMask = 0x0;
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitNumber].classesAndGroupConfig;

    fieldOffset = 24 + classNum;
    U32_SET_FIELD_MAC(regData , fieldOffset, 1, (classParamsPtr->forceEnable) ? 1 : 0 );
    U32_SET_FIELD_MAC(regMask , fieldOffset, 1, 1);
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMask, regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Class forced data Register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitNumber].classForcedData[classNum];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, classParamsPtr->forceData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Class Gobal Control Register */
    regData = regMask = 0x0;
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitNumber].classGlobalControl;

    fieldOffset = 6 + classNum;
    U32_SET_FIELD_MAC(regData, fieldOffset, 1, (classParamsPtr->pulseStretchEnable) ? 1 : 0);
    U32_SET_FIELD_MAC(regMask, fieldOffset, 1, 1);

    fieldOffset = classNum;
    U32_SET_FIELD_MAC(regData , fieldOffset, 1, (classParamsPtr->disableOnLinkDown) ? 1 : 0 );
    U32_SET_FIELD_MAC(regMask , fieldOffset, 1, 1);

    return prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMask, regData);;
}

/**
* @internal prvCpssDxChFalconLedStreamClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGE: 0..5)
* @param[in] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/

static GT_STATUS prvCpssDxChFalconLedStreamClassManipulationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;    /* return status */
    GT_U32  ledUnitIndex;

    for(ledUnitIndex = 0; ledUnitIndex < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledUnitIndex++)
    {
        rc = prvCpssDxChSip6LedUnitClassManipulationSet(devNum, ledUnitIndex, classNum, classParamsPtr);
        if( rc != GT_OK )
        {
            return rc;
        }
    }

    return GT_OK;
}

static GT_STATUS internal_cpssDxChBobcat2LedStreamClassManipulationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;    /* return status */
    GT_U32  regData; /* register led control value */
    GT_U32  regMask; /* register led control mask */
    GT_U32  regAddr; /* register address */
    GT_U32  fieldOffset; /* register field offset */

    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (classNum >= PRV_CPSS_DXCH_BOBCAT2_LED_INDICATION_CLASS_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Blink Global Control Register */
    regData = regMask = 0x0;
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].blinkGlobalControl;

    /* blinkGlobalControl.Blink En[18-25] position is defined by class Num */
    fieldOffset = 18 + classNum;
    U32_SET_FIELD_MAC(regData , fieldOffset, 1, (classParamsPtr->blinkEnable) ? 1 : 0 );
    U32_SET_FIELD_MAC(regMask , fieldOffset, 1, 1);

    /* blinkGlobalControl.Blink Sel[10-17] position is defined by class Num */
    fieldOffset = 10 + classNum;
    U32_SET_FIELD_MAC(regData , fieldOffset, 1, (classParamsPtr->blinkSelect == CPSS_LED_BLINK_SELECT_0_E) ? 0 : 1);
    U32_SET_FIELD_MAC(regMask , fieldOffset, 1, 1);
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMask, regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Classes and Group Configuration Register, Force En[24:29] position is defined by class Num */
    regData = regMask = 0x0;
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].classesAndGroupConfig;

    fieldOffset = 24 + classNum;
    U32_SET_FIELD_MAC(regData , fieldOffset, 1, (classParamsPtr->forceEnable) ? 1 : 0 );
    U32_SET_FIELD_MAC(regMask , fieldOffset, 1, 1);
    rc = prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMask, regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Class forced data Register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].classForcedData[classNum];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, classParamsPtr->forceData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Class Gobal Control Register */
    regData = regMask = 0x0;
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].classGlobalControl;

    fieldOffset = 6 + classNum;
    U32_SET_FIELD_MAC(regData , fieldOffset, 1, (classParamsPtr->pulseStretchEnable) ? 1 : 0 );
    U32_SET_FIELD_MAC(regMask , fieldOffset, 1, 1);

    fieldOffset = classNum;
    U32_SET_FIELD_MAC(regData , fieldOffset, 1, (classParamsPtr->disableOnLinkDown) ? 1 : 0 );
    U32_SET_FIELD_MAC(regMask , fieldOffset, 1, 1);

    return prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMask, regData);
}

/**
* @internal internal_cpssDxChLedStreamPortGroupClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
*                                      (APPLICABLE DEVICES  xCat3; AC5; Lion2)
* @param[in] classNum                 - class number.
*                                      For tri-speed ports:
*                                      (APPLICABLE RANGES: xCat3, AC5, Lion2  0..6;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X 0..5)
*                                      For XG ports: (APPLICABLE RANGES: xCat3, AC5, Lion2 0..11;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..5)
* @param[in] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamPortGroupClassManipulationSet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;    /* return status */
    GT_U32  regData; /* register led control value */
    GT_U32  regMask; /* register led control mask */
    GT_U32  regAddr; /* register address */
    GT_U32  invertEnableOffset; /* field offset of invert enable */
    GT_U32  blinkEnableOffset;  /* field offset of blink enable */
    GT_U32  blinkSelectOffset;  /* field offset of blink select */
    GT_U32  forceEnableOffset;  /* field offset of force enable */
    GT_U32  forceDataOffset;    /* field offset of force data */
    GT_U32  forceDataLength;  /* field length of force data */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(classParamsPtr);
    LED_INTERFACE_NUM_CHECK_MAC(devNum,ledInterfaceNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* Falcon LED support */
            return prvCpssDxChFalconLedStreamClassManipulationSet(devNum, classNum, classParamsPtr);
        }
        else
        {
            return internal_cpssDxChBobcat2LedStreamClassManipulationSet(devNum, ledInterfaceNum, classNum, classParamsPtr);
        }
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);
    switch(portType)
    {
        case CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E:
            rc = prvCpssDxChLedStreamClassManipTriSpeedAux(devNum,
                                                           ledInterfaceNum,
                                                           classNum,
                                                           &regAddr,
                                                           &invertEnableOffset,
                                                           &blinkEnableOffset,
                                                           &blinkSelectOffset,
                                                           &forceEnableOffset,
                                                           &forceDataOffset,
                                                           &forceDataLength);
            break;
        case CPSS_DXCH_LED_PORT_TYPE_XG_E:
            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
               (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssDxChLedStreamClassManipXgAux(devNum,
                                                     ledInterfaceNum,
                                                     classNum,
                                                     &regAddr,
                                                     &invertEnableOffset,
                                                     &blinkEnableOffset,
                                                     &blinkSelectOffset,
                                                     &forceEnableOffset,
                                                     &forceDataOffset,
                                                     &forceDataLength);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if( rc != GT_OK )
    {
        return rc;
    }

    if( classParamsPtr->forceData >= (GT_U32)(1 << forceDataLength) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regData = 0x0;
    regMask = 0x0;

    U32_SET_FIELD_MAC(regData , forceDataOffset, forceDataLength, classParamsPtr->forceData);
    U32_SET_FIELD_MAC(regMask , forceDataOffset, forceDataLength, ((1 << forceDataLength)-1));

    U32_SET_FIELD_MAC(regData , forceEnableOffset, 1, ((classParamsPtr->forceEnable == GT_TRUE)?1:0));
    U32_SET_FIELD_MAC(regMask , forceEnableOffset, 1, 1);

    U32_SET_FIELD_MAC(regData , blinkEnableOffset, 1, ((classParamsPtr->blinkEnable == GT_TRUE)?1:0));
    U32_SET_FIELD_MAC(regMask , blinkEnableOffset, 1, 1);

    switch(classParamsPtr->blinkSelect)
    {
        case CPSS_LED_BLINK_SELECT_0_E: U32_SET_FIELD_MAC(regData , blinkSelectOffset, 1, 0);
                                        break;
        case CPSS_LED_BLINK_SELECT_1_E: U32_SET_FIELD_MAC(regData , blinkSelectOffset, 1, 1);
                                        break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regMask , blinkSelectOffset, 1, 1);

    U32_SET_FIELD_MAC(regData , invertEnableOffset, 1, ((classParamsPtr->invertEnable == GT_TRUE)?1:0));
    U32_SET_FIELD_MAC(regMask , invertEnableOffset, 1, 1);

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(devNum, portGroupId, regAddr, regMask, regData);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChLedStreamPortGroupClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
*                                      (APPLICABLE DEVICES  xCat3; AC5; Lion2)
* @param[in] classNum                 - class number.
*                                      For tri-speed ports:
*                                      (APPLICABLE RANGES: xCat3, AC5, Lion2  0..6;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X 0..5)
*                                      For XG ports: (APPLICABLE RANGES: xCat3, AC5, Lion2 0..11;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..5)
* @param[in] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortGroupClassManipulationSet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamPortGroupClassManipulationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, ledInterfaceNum, portType, classNum, classParamsPtr));

    rc = internal_cpssDxChLedStreamPortGroupClassManipulationSet(devNum, portGroupsBmp, ledInterfaceNum, portType, classNum, classParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, ledInterfaceNum, portType, classNum, classParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChFalconLedUnitClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] ledUnitNumber         - LED unit number
* @param[in] classNum              - class number
*                                      (APPLICABLE RANGES: 0..5)
*
* @param[out] classParamsPtr       - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChFalconLedUnitClassManipulationGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledUnitNumber,
    IN  GT_U32                          classNum,
    OUT  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;    /* return status */
    GT_U32  regData; /* register led control value */
    GT_U32  regAddr; /* register address */
    GT_U32  fieldOffset; /* register field offset */

    if ((classNum >= PRV_CPSS_DXCH_BOBCAT2_LED_INDICATION_CLASS_NUMBER_CNS) ||
        (ledUnitNumber >= SIP_6_MAX_LED_UNIT_NUM_CNC))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(classParamsPtr, 0, sizeof(CPSS_LED_CLASS_MANIPULATION_STC));

    /* Blink Global Control Register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitNumber].blinkGlobalControl;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    fieldOffset = 18 + classNum;
    classParamsPtr->blinkEnable =
        U32_GET_FIELD_MAC(regData , fieldOffset, 1) ? GT_TRUE : GT_FALSE;

    fieldOffset = 10 + classNum;
    classParamsPtr->blinkSelect =
        U32_GET_FIELD_MAC(regData , fieldOffset, 1) ? CPSS_LED_BLINK_SELECT_1_E : CPSS_LED_BLINK_SELECT_0_E;

    /* Classes and Group Configuration Register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitNumber].classesAndGroupConfig;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    fieldOffset = 24 + classNum;
    classParamsPtr->forceEnable =
        U32_GET_FIELD_MAC(regData , fieldOffset, 1) ? GT_TRUE : GT_FALSE;


    /* Class forced data Register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitNumber].classForcedData[classNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &classParamsPtr->forceData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Class Gobal Control Register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitNumber].classGlobalControl;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    fieldOffset = 6 + classNum;
    classParamsPtr->pulseStretchEnable =
        U32_GET_FIELD_MAC(regData, fieldOffset, 1) ? GT_TRUE : GT_FALSE;

    fieldOffset = classNum;
    classParamsPtr->disableOnLinkDown =
        U32_GET_FIELD_MAC(regData, fieldOffset, 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

static GT_STATUS internal_cpssDxChBobcat2LedStreamClassManipulationGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledInterfaceNum,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;    /* return status */
    GT_U32  regData; /* register led control value */
    GT_U32  regAddr; /* register address */
    GT_U32  fieldOffset; /* register field offset */

    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (classNum >= PRV_CPSS_DXCH_BOBCAT2_LED_INDICATION_CLASS_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(classParamsPtr, 0, sizeof(CPSS_LED_CLASS_MANIPULATION_STC));

    /* Blink Global Control Register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].blinkGlobalControl;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    fieldOffset = 18 + classNum;
    classParamsPtr->blinkEnable =
        U32_GET_FIELD_MAC(regData , fieldOffset, 1) ? GT_TRUE : GT_FALSE;

    fieldOffset = 10 + classNum;
    classParamsPtr->blinkSelect =
        U32_GET_FIELD_MAC(regData , fieldOffset, 1) ? CPSS_LED_BLINK_SELECT_1_E : CPSS_LED_BLINK_SELECT_0_E;

    /* Classes and Group Configuration Register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].classesAndGroupConfig;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    fieldOffset = 24 + classNum;
    classParamsPtr->forceEnable =
        U32_GET_FIELD_MAC(regData , fieldOffset, 1) ? GT_TRUE : GT_FALSE;


    /* Class forced data Register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].classForcedData[classNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &classParamsPtr->forceData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Class Gobal Control Register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].classGlobalControl;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    fieldOffset = 6 + classNum;
    classParamsPtr->pulseStretchEnable =
        U32_GET_FIELD_MAC(regData, fieldOffset, 1) ? GT_TRUE : GT_FALSE;

    fieldOffset = classNum;
    classParamsPtr->disableOnLinkDown =
        U32_GET_FIELD_MAC(regData, fieldOffset, 1) ? GT_TRUE : GT_FALSE;


    return GT_OK;
}

/**
* @internal internal_cpssDxChLedStreamPortGroupClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] classNum                 - class number.
*                                      For tri-speed ports:
*                                      (APPLICABLE RANGES: xCat3, AC5, Lion2  0..6;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X 0..5)
*                                      For XG ports: (APPLICABLE RANGES: xCat3, AC5, Lion2 0..11;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..5)
*
* @param[out] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamPortGroupClassManipulationGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    OUT CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;    /* return status */
    GT_U32  regData; /* register led control value */
    GT_U32  regAddr; /* register address */
    GT_U32  invertEnableOffset; /* field offset of invert enable */
    GT_U32  blinkEnableOffset;  /* field offset of blink enable */
    GT_U32  blinkSelectOffset;  /* field offset of blink select */
    GT_U32  forceEnableOffset;  /* field offset of force enable */
    GT_U32  forceDataOffset;    /* field offset of force data */
    GT_U32  forceDataLength;    /* field length of force data */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32  firstLedUnitNumber;   /* first LED unit in stream chain */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(classParamsPtr);
    LED_INTERFACE_NUM_CHECK_MAC(devNum,ledInterfaceNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* Get first LED unit in the stream chain */
            rc = prvCpssDxChSip6LedStreamFirstLedUnitGet(devNum, &firstLedUnitNumber);
            if( rc != GT_OK )
            {
                return rc;
            }

            /* Falcon LED support */
            return prvCpssDxChFalconLedUnitClassManipulationGet(devNum, firstLedUnitNumber, classNum, classParamsPtr);
        }
        else
        {
            return internal_cpssDxChBobcat2LedStreamClassManipulationGet(devNum, ledInterfaceNum, classNum, classParamsPtr);
        }
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    switch(portType)
    {
        case CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E:
            rc = prvCpssDxChLedStreamClassManipTriSpeedAux(devNum,
                                                           ledInterfaceNum,
                                                           classNum,
                                                           &regAddr,
                                                           &invertEnableOffset,
                                                           &blinkEnableOffset,
                                                           &blinkSelectOffset,
                                                           &forceEnableOffset,
                                                           &forceDataOffset,
                                                           &forceDataLength);
            break;
        case CPSS_DXCH_LED_PORT_TYPE_XG_E:
            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
               (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssDxChLedStreamClassManipXgAux(devNum,
                                                     ledInterfaceNum,
                                                     classNum,
                                                     &regAddr,
                                                     &invertEnableOffset,
                                                     &blinkEnableOffset,
                                                     &blinkSelectOffset,
                                                     &forceEnableOffset,
                                                     &forceDataOffset,
                                                     &forceDataLength);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if( rc != GT_OK )
    {
        return rc;
    }

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    classParamsPtr->forceData = U32_GET_FIELD_MAC(regData, forceDataOffset, forceDataLength);
    classParamsPtr->forceEnable = U32_GET_FIELD_MAC(regData, forceEnableOffset, 1) == 1 ?
                                    GT_TRUE : GT_FALSE;

    classParamsPtr->blinkEnable = U32_GET_FIELD_MAC(regData, blinkEnableOffset, 1) == 1 ?
                                    GT_TRUE : GT_FALSE;

    classParamsPtr->blinkSelect = U32_GET_FIELD_MAC(regData, blinkSelectOffset, 1) == 0 ?
                                    CPSS_LED_BLINK_SELECT_0_E : CPSS_LED_BLINK_SELECT_1_E;

    classParamsPtr->invertEnable = U32_GET_FIELD_MAC(regData, invertEnableOffset, 1) == 1 ?
                                    GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal cpssDxChLedStreamPortGroupClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] classNum                 - class number.
*                                      For tri-speed ports:
*                                      (APPLICABLE RANGES: xCat3, AC5, Lion2  0..6;
*                                      Bobcat2, Caelum, Bobcat3, Aldrin, AC3X 0..5)
*                                      For XG ports: (APPLICABLE RANGES: xCat3, AC5, Lion2 0..11;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..5)
*
* @param[out] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortGroupClassManipulationGet
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U32                          ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT     portType,
    IN  GT_U32                          classNum,
    OUT CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamPortGroupClassManipulationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, ledInterfaceNum, portType, classNum, classParamsPtr));

    rc = internal_cpssDxChLedStreamPortGroupClassManipulationGet(devNum, portGroupsBmp, ledInterfaceNum, portType, classNum, classParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, ledInterfaceNum, portType, classNum, classParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChLedStreamGroupConfXgAux function
* @endinternal
*
* @brief   Auxilary function to calculate register address and offset for group
*         configuration for XG (Flex) ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] interfaceNum             - LED stream interface number.
* @param[in] groupNum                 - group number
*
* @param[out] regAddrPtr               - (pointer to) register address
* @param[out] groupDataOffsetPtr       - (pointer to) group data offset in register.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChLedStreamGroupConfXgAux
(
    IN  GT_U8   devNum,
    IN  GT_U32  interfaceNum,
    IN  GT_U32  groupNum,
    OUT GT_U32  *regAddrPtr,
    OUT GT_U32  *groupDataOffsetPtr
)
{
    if( groupNum >= 2 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *regAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ledRegs.ledXgGroup01Configuration[interfaceNum];

    *groupDataOffsetPtr = ( groupNum & 0x1 ) ? 16 : 0 ;

    return GT_OK;
}

/**
* @internal prvCpssDxChSip6LedUnitStreamGroupConfigSet function
* @endinternal
*
* @brief   Sets LED unit stream group parameters
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] classNum              - class number.
*                                    (APPLICABLE RANGE: 0..5)
* @param[in] classParamsPtr        - (pointer to) class manipulation parameters
* @param[in] ledUnitIndex          - LED unit index
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChSip6LedUnitStreamGroupConfigSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      groupNum,
    IN  CPSS_LED_GROUP_CONF_STC    *groupParamsPtr,
    IN  GT_U32                      ledUnitIndex
)
{
    GT_U32  regData;                /* register led control value */
    GT_U32  regAddr;                /* register address */
    GT_U32  ledUnitNumber;          /* LED interface number */
    GT_U32  ledUnitRegDbIndex;      /* LED unit register DB index */
    GT_U32  groupDataOffset;        /* offset in register for group data */

    LED_UNIT_NUMBER_BY_INDEX_GET_MAC(devNum, ledUnitIndex, ledUnitNumber);

    ledUnitRegDbIndex = LED_REG_DB_INDEX_BY_UNIT_ID(devNum, ledUnitNumber);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].classesAndGroupConfig;
    regData = 0;

    groupDataOffset = groupNum * 12;

    U32_SET_FIELD_MAC(regData, 0, 3, groupParamsPtr->classA);
    U32_SET_FIELD_MAC(regData, 3, 3, groupParamsPtr->classB);
    U32_SET_FIELD_MAC(regData, 6, 3, groupParamsPtr->classC);
    U32_SET_FIELD_MAC(regData, 9, 3, groupParamsPtr->classD);

    return prvCpssDrvHwPpSetRegField(devNum, regAddr, groupDataOffset, 12, regData);
}

/**
* @internal prvCpssDxChFalconLedStreamGroupConfigSet function
* @endinternal
*
* @brief   This routine configures a LED stream group.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] groupNum                 - group number
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - on out of range class number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChFalconLedStreamGroupConfigSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      groupNum,
    IN  CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_U32  ledIndex;               /* LED index */
    GT_STATUS rc;

    if (groupNum >= PRV_CPSS_DXCH_BOBCAT2_LED_GROUP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (groupParamsPtr->classA >= BIT_3 ||
        groupParamsPtr->classB >= BIT_3 ||
        groupParamsPtr->classC >= BIT_3 ||
        groupParamsPtr->classD >= BIT_3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    for(ledIndex = 0; ledIndex < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledIndex++)
    {
        rc = prvCpssDxChSip6LedUnitStreamGroupConfigSet(devNum, groupNum, groupParamsPtr, ledIndex);
        if( rc != GT_OK )
        {
            return rc;
        }
    }

    return GT_OK;
}

static GT_STATUS internal_cpssDxChBobcat2LedStreamGroupConfigSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      ledInterfaceNum,
    IN  GT_U32                      groupNum,
    IN  CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_U32  regData;            /* register value */
    GT_U32  regAddr;            /* register address */
    GT_U32  groupDataOffset;    /* offset in register for group data */

    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (groupNum >= PRV_CPSS_DXCH_BOBCAT2_LED_GROUP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (groupParamsPtr->classA >= BIT_3 ||
        groupParamsPtr->classB >= BIT_3 ||
        groupParamsPtr->classC >= BIT_3 ||
        groupParamsPtr->classD >= BIT_3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].classesAndGroupConfig;
    regData = 0;

    groupDataOffset = groupNum * 12;

    U32_SET_FIELD_MAC(regData, 0, 3, groupParamsPtr->classA);
    U32_SET_FIELD_MAC(regData, 3, 3, groupParamsPtr->classB);
    U32_SET_FIELD_MAC(regData, 6, 3, groupParamsPtr->classC);
    U32_SET_FIELD_MAC(regData, 9, 3, groupParamsPtr->classD);

    return prvCpssDrvHwPpSetRegField(devNum, regAddr, groupDataOffset, 12, regData);
}

/**
* @internal internal_cpssDxChLedStreamPortGroupGroupConfigSet function
* @endinternal
*
* @brief   This routine configures a LED stream group.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] groupNum                 - group number
*                                      For tri-speed ports: (APPLICABLE RANGES: 0..3)
*                                      For XG ports: (APPLICABLE RANGES: 0..1)
* @param[in] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - on out of range class number
*                                       (allowable class range o..15)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamPortGroupGroupConfigSet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    IN  CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr = 0;            /* register address */
    GT_U32  groupDataOffset = 0;    /* offset in register for group data */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(groupParamsPtr);
    LED_INTERFACE_NUM_CHECK_MAC(devNum,ledInterfaceNum);

    if( (groupParamsPtr->classA > 0xF) ||
        (groupParamsPtr->classB > 0xF) ||
        (groupParamsPtr->classC > 0xF) ||
        (groupParamsPtr->classD > 0xF) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* Falcon LED support */
            return prvCpssDxChFalconLedStreamGroupConfigSet(devNum, groupNum, groupParamsPtr);
        }
        else
        {
            return internal_cpssDxChBobcat2LedStreamGroupConfigSet(devNum, ledInterfaceNum, groupNum, groupParamsPtr);
        }
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);
    switch(portType)
    {
        case CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E:
            rc = prvCpssDxChLedStreamGroupConfTriSpeedAux(devNum,
                                                          ledInterfaceNum,
                                                          groupNum,
                                                          &regAddr,
                                                          &groupDataOffset);
            break;
        case CPSS_DXCH_LED_PORT_TYPE_XG_E:
            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
               (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssDxChLedStreamGroupConfXgAux(devNum,
                                                    ledInterfaceNum,
                                                    groupNum,
                                                    &regAddr,
                                                    &groupDataOffset);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if( rc != GT_OK )
    {
        return rc;
    }

    regData = 0;
    U32_SET_FIELD_MAC(regData , 0, 4, groupParamsPtr->classA);
    U32_SET_FIELD_MAC(regData , 4, 4, groupParamsPtr->classB);
    U32_SET_FIELD_MAC(regData , 8, 4, groupParamsPtr->classC);
    U32_SET_FIELD_MAC(regData , 12, 4, groupParamsPtr->classD);

    /* loop on all active port groups in the bmp */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                groupDataOffset, 16, regData);
        if( rc != GT_OK )
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    return GT_OK;
}

/**
* @internal cpssDxChLedStreamPortGroupGroupConfigSet function
* @endinternal
*
* @brief   This routine configures a LED stream group.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] groupNum                 - group number
*                                      For tri-speed ports: (APPLICABLE RANGES: 0..3)
*                                      For XG ports: (APPLICABLE RANGES: 0..1)
* @param[in] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - on out of range class number
*                                       (allowable class range o..15)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortGroupGroupConfigSet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    IN  CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamPortGroupGroupConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, ledInterfaceNum, portType, groupNum, groupParamsPtr));

    rc = internal_cpssDxChLedStreamPortGroupGroupConfigSet(devNum, portGroupsBmp, ledInterfaceNum, portType, groupNum, groupParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, ledInterfaceNum, portType, groupNum, groupParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChFalconLedStreamGroupConfigGet function
* @endinternal
*
* @brief   This routine gets a LED stream group configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] groupNum                 - group number
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChFalconLedStreamGroupConfigGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      groupNum,
    OUT CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr;            /* register address */
    GT_U32  groupDataOffset;    /* offset in register for group data */
    GT_U32  firstLedUnit;       /* first non bypassed LED unit in stream chain */
    GT_U32  ledUnitRegDbIndex;  /* LED unit register DB index */

    if (groupNum >= PRV_CPSS_DXCH_BOBCAT2_LED_GROUP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get first LED unit in stream chain */
    rc = prvCpssDxChSip6LedStreamFirstLedUnitGet(devNum, &firstLedUnit);
    if( rc != GT_OK )
    {
        return rc;
    }

    ledUnitRegDbIndex = LED_REG_DB_INDEX_BY_UNIT_ID(devNum, firstLedUnit);

    groupDataOffset = groupNum * 12;
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledUnitRegDbIndex].classesAndGroupConfig;
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, groupDataOffset, 12, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    groupParamsPtr->classA = U32_GET_FIELD_MAC(regData, 0, 3);
    groupParamsPtr->classB = U32_GET_FIELD_MAC(regData, 3, 3);
    groupParamsPtr->classC = U32_GET_FIELD_MAC(regData, 6, 3);
    groupParamsPtr->classD = U32_GET_FIELD_MAC(regData, 9, 3);

    return GT_OK;
}

static GT_STATUS internal_cpssDxChBobcat2LedStreamGroupConfigGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      ledInterfaceNum,
    IN  GT_U32                      groupNum,
    OUT CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr;            /* register address */
    GT_U32  groupDataOffset;    /* offset in register for group data */

    if (PRV_CPSS_SIP_5_10_CHECK_MAC(devNum) == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (groupNum >= PRV_CPSS_DXCH_BOBCAT2_LED_GROUP_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    groupDataOffset = groupNum * 12;
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].classesAndGroupConfig;
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, groupDataOffset, 12, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    groupParamsPtr->classA = U32_GET_FIELD_MAC(regData, 0, 3);
    groupParamsPtr->classB = U32_GET_FIELD_MAC(regData, 3, 3);
    groupParamsPtr->classC = U32_GET_FIELD_MAC(regData, 6, 3);
    groupParamsPtr->classD = U32_GET_FIELD_MAC(regData, 9, 3);


    return GT_OK;
}

/**
* @internal internal_cpssDxChLedStreamPortGroupGroupConfigGet function
* @endinternal
*
* @brief   This routine gets a LED stream group configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] groupNum                 - group number
*                                      For tri-speed ports: (APPLICABLE RANGES: 0..3)
*                                      For XG ports: (APPLICABLE RANGES: 0..1)
*
* @param[out] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamPortGroupGroupConfigGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    OUT CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr = 0;            /* register address */
    GT_U32  groupDataOffset = 0;    /* offset in register for group data */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(groupParamsPtr);
    LED_INTERFACE_NUM_CHECK_MAC(devNum,ledInterfaceNum);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* Falcon LED support */
            return prvCpssDxChFalconLedStreamGroupConfigGet(devNum, groupNum, groupParamsPtr);
        }
        else
        {
            return internal_cpssDxChBobcat2LedStreamGroupConfigGet(devNum, ledInterfaceNum, groupNum, groupParamsPtr);
        }
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
        devNum, portGroupsBmp, portGroupId);

    switch(portType)
    {
        case CPSS_DXCH_LED_PORT_TYPE_TRI_SPEED_E:
            rc = prvCpssDxChLedStreamGroupConfTriSpeedAux(devNum,
                                                          ledInterfaceNum,
                                                          groupNum,
                                                          &regAddr,
                                                          &groupDataOffset);
            break;
        case CPSS_DXCH_LED_PORT_TYPE_XG_E:
            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
               (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssDxChLedStreamGroupConfXgAux(devNum,
                                                    ledInterfaceNum,
                                                    groupNum,
                                                    &regAddr,
                                                    &groupDataOffset);
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if( rc != GT_OK )
    {
        return rc;
    }

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                            groupDataOffset, 16, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    groupParamsPtr->classA = U32_GET_FIELD_MAC(regData, 0, 4);
    groupParamsPtr->classB = U32_GET_FIELD_MAC(regData, 4, 4);
    groupParamsPtr->classC = U32_GET_FIELD_MAC(regData, 8, 4);
    groupParamsPtr->classD = U32_GET_FIELD_MAC(regData, 12, 4);

    return GT_OK;
}

/**
* @internal cpssDxChLedStreamPortGroupGroupConfigGet function
* @endinternal
*
* @brief   This routine gets a LED stream group configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
* @param[in] ledInterfaceNum          - LED stream interface number.
*                                      (APPLICABLE RANGES: xCat3, AC5 0..1;
*                                      Lion2, Falcon 0..0;
*                                      Bobcat2, Caelum, Bobcat3; Aldrin2 0..4;
*                                      Aldrin 0..1)
* @param[in] portType                 - tri-speed or XG port type.
* @param[in] groupNum                 - group number
*                                      For tri-speed ports: (APPLICABLE RANGES: 0..3)
*                                      For XG ports: (APPLICABLE RANGES: 0..1)
*
* @param[out] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortGroupGroupConfigGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      ledInterfaceNum,
    IN  CPSS_DXCH_LED_PORT_TYPE_ENT portType,
    IN  GT_U32                      groupNum,
    OUT CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamPortGroupGroupConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, ledInterfaceNum, portType, groupNum, groupParamsPtr));

    rc = internal_cpssDxChLedStreamPortGroupGroupConfigGet(devNum, portGroupsBmp, ledInterfaceNum, portType, groupNum, groupParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, ledInterfaceNum, portType, groupNum, groupParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWSet function
* @endinternal
*
* @brief   Sets number of the port in the LED's chain for specific MAC
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portMacType              - macType
* @param[in] position                 - port  index in LED stream
*                                      (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  PRV_CPSS_PORT_TYPE_ENT  portMacType,
    IN  GT_U32                  position
)
{
    GT_U32      portMacNum;             /* MAC number */
    GT_U32      regAddr;                /* register address */
    GT_U32      fieldOffset;            /* register field offset */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (!PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    switch (portMacType)
    {
        case PRV_CPSS_PORT_GE_E:
            PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
            fieldOffset = 10;
            break;
        case PRV_CPSS_PORT_XLG_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
            fieldOffset = 5;
            break;
        case PRV_CPSS_PORT_CG_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.CG[portMacNum].CG_CONVERTERS.CGMAControl1;
            fieldOffset = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "not supported MAC type %d, see PRV_CPSS_PORT_TYPE_ENT",portMacType);
    }
    if (prvCpssDrvHwPpSetRegField(devNum, regAddr, fieldOffset, 6, position) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet function
* @endinternal
*
* @brief   Gets number of the port in the LED's chain for specific MAC
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portMacType              - macType
*
* @param[out] positionPtr              - port position index in LED stream
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  PRV_CPSS_PORT_TYPE_ENT  portMacType,
    IN  GT_U32                 *positionPtr
)
{
    GT_U32      portMacNum;             /* MAC number */
    GT_U32      regAddr;                /* register address */
    GT_U32      fieldOffset;            /* register field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (!PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    switch (portMacType)
    {
        case PRV_CPSS_PORT_GE_E:
            PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
            fieldOffset = 10;
            break;
        case PRV_CPSS_PORT_XLG_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
            fieldOffset = 5;
            break;
        case PRV_CPSS_PORT_CG_E:
            if (0 != (portMacNum % 4)  &&  portMacNum >= 72)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "mac CG does exits for mac %d",portMacType);
            }
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.CG[portMacNum].CG_CONVERTERS.CGMAControl1;
            fieldOffset = 0;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "not supported MAC type %d, see PRV_CPSS_PORT_TYPE_ENT",portMacType);
    }
    if (prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, 6, /*OUT*/positionPtr) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}


/*----------------------------------------------------------------------------------
 *   Errata FE-3763116  scenarios:
 *   ------------------------------
 *
 *   1.  HW-Init()
 *          1.  mac type : undefinied. (possible values GE,XLG,CG, undefined)
 *          2.  led position : undefined (possible range 0-15/23 , undefined(0x3F)
 *
 *   2.  Port configuration (Power Up)
  *       a.
 *           - portModeSpeedSet()
 *                led position is not defined yet.
 *                save to DB configured MAC (either GE,XLG or CG)
 *                don't configure led position.
 *           - PortLedPositionSet(port, ledPosition)
 *                confiure mac specified by  DB to given led position (either GE,XLG or CG)
  *       b.
 *           - PortLedPositionSet()
 *                port is not configured yet. we don't know what mac type shall be configured.
 *                save led position to DB.
 *           - portModeSpeedSet()
 *                save to DB configured MAC (either GE,XLG or CG)
 *                configure led position for specified MAC type.
 *
 *   2.  Port configuration (Power Down)
 *           LED position for MAC type from DB(namely GE, XLG, CG) shall be set to 0x3F.
 *
 *
 *
 *   Design:
 *   -------
 *       HWInit
 *             1. errataPortLedInit
 *
 *       portModeSpeedSet(port, PowerUp)
 *             prvCpssDxChLedPortTypeConfig(port, PowerUp)
 *                  errataPortLedMacTypeSet(port,macType)
 *
 *       portModeSpeedSet(port, PowerDown)
 *             1. errataPortLedMacTypeSet(port,undefined)
 *
 *
 *       portLedPositionSet
 *             1. errataPortLedPositionSet(port,ledPosition)
 *
 *
 *              HWInit         portModeSpeedSet             portLedPositionSet
 *                |                  |                         |
 *          +--------------------------------------------------------+
 *          |   Led Errata handler                                   |
 *          |      errataPortLedInit();                              |
 *          |      errataPortLedMacTypeSet(port,macType)             |
 *          |      errataPortLedPositionSet(port,ledPosition)        |
 *          +--------------------------------------------------------+
 *          |          erratePortLedDB[]                             |
 *          |             - for each port(mac )                      |
 *          |                led position                            |
 *          |                mac type                                |
 *          +--------------------------------------------------------+
 *
 *       errataPortLedInit()
 *          1. for each mac
 *                 set LED position in all MAC types (namely GE, XLG, CG) to 0x3F.
 *           2. DB per mac :
 *                  1.  mac type : undefinied. (possible values GE,XLG,CG, undefined)
 *                  2.  led position : undefined (possible range 0-15, undefined(0x3F ????)
 *
 *       errataPortLedMacTypeSet(port,macType)
 *           if (macType == undefined ) // power down
 *              if (DB[port].macType != undefined) // last state was active mac
 *                 - configure  led of mac type DB[port].macType to 0x3f
 *                 - set DB[port].macType = undefined
 *           else
 *               1. set DB[port].macType = macType.
 *               2. if led position DB[port].ledPosition is undefined
 *                     do nothing return GT_OK;
 *                  else
 *                     configure  led of mac type DB[port].macType to led position from DB
 *
 *       errataPortLedPositionSet(port,ledPosition)
 *           DB[port].ledPosition = ledPosition.
 *           if DB[port].macType == undefined
 *              do nothing.
 *           else
 *              configure  led of mac type DB[port].macType to led position from DB.
 **----------------------------------------------------------------------------------*/
/*
   DB
   port led postion / MAC type
*/

/**
* @internal prvCpssDxChLedErrataPortLedInit function
* @endinternal
*
* @brief   Init FE-3763116 Port Led related Data Base
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChLedErrataPortLedInit
(
    IN  GT_U8                   devNum
)
{
    GT_U32 portMacNum;
    GT_U32 maxMacNum;

    if (GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT3_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    maxMacNum = PRV_CPSS_PP_MAC(devNum)->numOfPorts;
    for (portMacNum = 0 ; portMacNum < maxMacNum; portMacNum++)
    {
        PRV_CPSS_DXCH_PORT_LED_POSITION(devNum,portMacNum) = PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS;  /* init DB for all macs to invalide position */
        PRV_CPSS_DXCH_PORT_LED_MAC_TYPE(devNum,portMacNum) = PRV_CPSS_PORT_NOT_EXISTS_E;           /* init DB for all macs to invalide mode     */
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLedErrataPortLedMacTypeSet function
* @endinternal
*
* @brief   Init FE-3763116 Port Led Mac type set and if postion is configured
*         writes position to approptiate mac
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - portNum
* @param[in] macType                  -  , ( PRV_CPSS_PORT_NOT_EXISTS_E, if MAC is deacivated)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChLedErrataPortLedMacTypeSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  PRV_CPSS_PORT_TYPE_ENT  macType
)
{
    GT_STATUS                  rc;
    GT_U32                     portMacNum;             /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT     oldMMacType;
    GT_U32                     position;

    if (GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT3_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);


    oldMMacType = PRV_CPSS_DXCH_PORT_LED_MAC_TYPE(devNum,portMacNum);
    position    = PRV_CPSS_DXCH_PORT_LED_POSITION(devNum,portMacNum);

    if (oldMMacType != PRV_CPSS_PORT_NOT_EXISTS_E) /* designate position in old mac as void  in any case */
    {
        rc = prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWSet(devNum,portNum,oldMMacType,PRV_CPSS_DXCH_PORT_LED_POSITION_VOID_CNS);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }

    if (macType != PRV_CPSS_PORT_NOT_EXISTS_E) /* configure new mac to position from DB */
    {
        rc = prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWSet(devNum,portNum,macType,position);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    /* set new mac status */
    PRV_CPSS_DXCH_PORT_LED_MAC_TYPE(devNum,portMacNum) = macType;
    return GT_OK;
}

/**
* @internal prvCpssDxChLedErrataPortLedPositionSet function
* @endinternal
*
* @brief   Init FE-3763116 Port Led Position set and MAC type specified
*         writes position to approptiate mac
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - portNum
* @param[in] position                 - led  on LED interface corresponding to the port (MAC)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChLedErrataPortLedPositionSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  position
)
{
    GT_STATUS                  rc;
    GT_U32                     portMacNum;             /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT     macType;

    if (GT_FALSE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT3_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    macType = PRV_CPSS_DXCH_PORT_LED_MAC_TYPE(devNum,portMacNum);
    if (macType != PRV_CPSS_PORT_NOT_EXISTS_E) /* configure new mac to position from DB */
    {
        rc = prvCpssDxChBobcat2LedStreamPortPositionByMacTypeHWSet(devNum,portNum,macType,position);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }

    /* store new postion */
    PRV_CPSS_DXCH_PORT_LED_POSITION(devNum,portMacNum) = position;
    return GT_OK;
}

/**
* @internal prvCpssHawkLedControlInfoGet function
* @endinternal
*
* @brief  Get LED port control register address per specific MAC/Type.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portMacNum               - portNum MAC
* @param[in] portMacType              - portNum MAC type
* @param[out] regAddrPtr              - (pointer to) register's address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssHawkLedControlInfoGet
(
    IN GT_U8    devNum,
    IN GT_U32   portMacNum,
    IN PRV_CPSS_PORT_TYPE_ENT  portMacType,
    OUT GT_U32  *regAddrPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_REG_DB_INFO_STC        regDbInfo;

    switch (portMacType)
    {
        case PRV_CPSS_PORT_MTI_100_E:
            rc = prvCpssSip6RegDbInfoGet(devNum,portMacNum, PRV_CPSS_REG_DB_TYPE_MTI_MAC_EXT_E, &regDbInfo);
            if(rc != GT_OK)
            {
                *regAddrPtr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            else
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, regDbInfo.regDbIndex).portControl;
            }
            break;
        case PRV_CPSS_PORT_MTI_400_E:
            rc = prvCpssSip6RegDbInfoGet(devNum,portMacNum, PRV_CPSS_REG_DB_TYPE_MTI_MAC_EXT_E, &regDbInfo);
            if(rc != GT_OK)
            {
                *regAddrPtr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            else
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_GOP_MTI_EXT_MAC(devNum, regDbInfo.regDbIndex).segPortControl;
            }
            break;
        case PRV_CPSS_PORT_MTI_CPU_E:
            rc = prvCpssSip6RegDbInfoGet(devNum,portMacNum, PRV_CPSS_REG_DB_TYPE_MTI_CPU_EXT_E, &regDbInfo);
            if(rc != GT_OK)
            {
                *regAddrPtr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            else
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_GOP_MTI_CPU_EXT_MAC(devNum, regDbInfo.regDbIndex).portControl;
            }
            break;
        case PRV_CPSS_PORT_MTI_USX_E:
            rc = prvCpssSip6RegDbInfoGet(devNum,portMacNum, PRV_CPSS_REG_DB_TYPE_MTI_USX_EXT_E, &regDbInfo);
            if(rc != GT_OK)
            {
                *regAddrPtr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
            }
            else
            {
                *regAddrPtr = PRV_DXCH_REG1_UNIT_GOP_MTI_USX_EXT_MAC(devNum, portMacNum).portControl;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal falconRegInfoGet_LEDControl function
* @endinternal
*
* @brief  Get LED port control register address per specific MAC/Type
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portMacNum               - portNum MAC
* @param[in] portMacType              - portNum MAC type
* @param[out] regAddrPtr              - (pointer to) register's address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS falconRegInfoGet_LEDControl(
    IN GT_U8    devNum,
    IN GT_U32   portMacNum,
    IN PRV_CPSS_PORT_TYPE_ENT  portMacType,
    OUT GT_U32  *regAddrPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_REG_DB_INFO_STC        regDbInfo;

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        return prvCpssHawkLedControlInfoGet(devNum, portMacNum, portMacType, regAddrPtr);
    }

    rc = prvCpssSip6RegDbInfoGet(devNum, portMacNum, PRV_CPSS_REG_DB_TYPE_MTI_MPFS_E, &regDbInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch (portMacType)
    {
        case PRV_CPSS_PORT_MTI_100_E:
        case PRV_CPSS_PORT_MTI_400_E:
            *regAddrPtr = PRV_DXCH_REG1_UNIT_MPFS_MAC(devNum, regDbInfo.regDbIndex).LEDControl;
            break;
        case PRV_CPSS_PORT_MTI_CPU_E:
            *regAddrPtr = PRV_DXCH_REG1_UNIT_MPFS_CPU_MAC(devNum, regDbInfo.regDbIndex).LEDControl;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(*regAddrPtr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

#define NOT_USED    0xFFFF
/* LED port fields positions in  port control register */
typedef struct {
    GT_U32  ledPortNumberOffset;
    GT_U32  ledPortEnableOffset;
} LED_PORT_FIELD_POSITIONS_STC;

/**
* @internal falconLedControlFiledsOffsetGet function
* @endinternal
*
* @brief  Get LED port relevant fields offstets from LED port control register per specific MAC/Type
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portMacType              - portNum MAC type
* @param[out] ledPortFieldsPositionsPtr  - (pointer to) LED port fileds offsets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS falconLedControlFiledsOffsetGet(
    IN GT_U8    devNum,
    IN PRV_CPSS_PORT_TYPE_ENT  portMacType,
    OUT LED_PORT_FIELD_POSITIONS_STC  *ledPortFieldsPositionsPtr
)
{
    LED_PORT_FIELD_POSITIONS_STC  *ledPortFieldsPositionsCurrentPtr;

    LED_PORT_FIELD_POSITIONS_STC falconPortsFieldPositionArr[4] =
    {
        {1, 0}, /* MTI_100_E */
        {1, 0}, /* MTI_400_E */
        {1, 0}, /* MTI_CPU_E */
        {1, 0}  /* MTI_USX_E */
    };
    LED_PORT_FIELD_POSITIONS_STC ac5PPortsFieldPositionArr[4] =
    {
        { 9, 15}, /* MTI_100_E */
        {11, 17}, /* MTI_400_E */
        {10, 16}, /* MTI_CPU_E */
        {10, 16}  /* MTI_USX_E */
    };
    LED_PORT_FIELD_POSITIONS_STC ac5xPortsFieldPositionArr[4] =
    {
        {10, 16}, /* MTI_100_E */
        {NOT_USED, NOT_USED}, /* MTI_400_E */
        {10, 16}, /* MTI_CPU_E */
        {10, 16}  /* MTI_USX_E */
    };
    LED_PORT_FIELD_POSITIONS_STC harrierPortsFieldPositionArr[4] =
    {
        { 9,  15}, /* MTI_100_E */
        {11,  17}, /* MTI_400_E */
        {NOT_USED, NOT_USED}, /* MTI_CPU_E */
        {NOT_USED, NOT_USED}  /* MTI_USX_E */
    };

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            ledPortFieldsPositionsCurrentPtr = falconPortsFieldPositionArr;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            ledPortFieldsPositionsCurrentPtr = ac5PPortsFieldPositionArr;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            ledPortFieldsPositionsCurrentPtr = ac5xPortsFieldPositionArr;
            break;
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            ledPortFieldsPositionsCurrentPtr = harrierPortsFieldPositionArr;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (portMacType)
    {
        case PRV_CPSS_PORT_MTI_100_E:
            *ledPortFieldsPositionsPtr = ledPortFieldsPositionsCurrentPtr[0];
            break;
        case PRV_CPSS_PORT_MTI_400_E:
            *ledPortFieldsPositionsPtr = ledPortFieldsPositionsCurrentPtr[1];
            break;
        case PRV_CPSS_PORT_MTI_CPU_E:
            *ledPortFieldsPositionsPtr = ledPortFieldsPositionsCurrentPtr[2];
            break;
        case PRV_CPSS_PORT_MTI_USX_E:
            *ledPortFieldsPositionsPtr = ledPortFieldsPositionsCurrentPtr[3];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFalconLedStreamPortPositionSet function
* @endinternal
*
* @brief   Sets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman;
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] position                 - port  index in LED stream
*                                      (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssDxChFalconLedStreamPortPositionSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  position
)
{
    GT_STATUS               rc;
    GT_U32                  portMacNum;
    CPSS_PORT_MAC_TYPE_ENT  portMacType;
    GT_U32                  regAddr;
    GT_U32                  portEnable;
    LED_PORT_FIELD_POSITIONS_STC  ledPortFieldsPositions;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        /* the GM not support it */
        return GT_OK;
    }

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        return GT_OK;   CPSS_TBD_BOOKMARK_IRONMAN
    }

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum);

    rc = falconRegInfoGet_LEDControl(devNum, portMacNum, portMacType, &regAddr);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = falconLedControlFiledsOffsetGet(devNum, portMacType, &ledPortFieldsPositions);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (ledPortFieldsPositions.ledPortNumberOffset != NOT_USED)
    {
        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr,
                                       ledPortFieldsPositions.ledPortNumberOffset, 6, position);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        portEnable = (position != 0x3F) ? 1 : 0;

        if (ledPortFieldsPositions.ledPortEnableOffset != NOT_USED)
        {
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr,
                                           ledPortFieldsPositions.ledPortEnableOffset, 1, portEnable);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChBobcat2LedStreamPortPositionSet function
* @endinternal
*
* @brief   Sets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:   Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] position                 - port  index in LED stream
*                                      (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChBobcat2LedStreamPortPositionSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  position
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;                /* register address */
    GT_U32      fieldOffset;            /* register field offset */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* port MAC type */
    GT_U32      portMacNum;             /* MAC number */

    if (!PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) &&
        !PRV_CPSS_DXCH_XCAT3_A1_AND_ABOVE_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_ILKN_E)
    {/* ILKN supported for B0 and above */
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.ILKN_WRP.sdFcLedConvertersControl0;
            return prvCpssDrvHwPpSetRegField(devNum, regAddr, 10, 6, position);
        }
        else
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_BOBCAT3_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        rc = prvCpssDxChLedErrataPortLedPositionSet(devNum,portNum,position);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    }
    else
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            rc = prvCpssDxChFalconLedStreamPortPositionSet(devNum, portNum, position);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* Set LED port position in DB */
            PRV_CPSS_DXCH_PORT_LED_POSITION(devNum, portMacNum) = position;

            return GT_OK;
        }

        for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)
            {
                if(portNum == CPSS_CPU_PORT_NUM_CNS)
                    continue;
            }

            switch (portMacType)
            {
                case PRV_CPSS_PORT_GE_E:
                    PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
                    fieldOffset = 10;
                    break;
                case PRV_CPSS_PORT_XG_E:
                    /* some ports have only GE MAC */
                    if (PRV_CPSS_GE_PORT_GE_ONLY_E ==
                                    PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, portMacNum))
                        continue;

                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
                    fieldOffset = 5;
                    break;
                case PRV_CPSS_PORT_CG_E:
                    if (PRV_CPSS_XG_PORT_CG_SGMII_E != PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, portMacNum))
                    {
                        continue;
                    }
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.CG[portMacNum].CG_CONVERTERS.CGMAControl1;
                    fieldOffset = 0;
                    break;
                default:
                    continue;
            }

            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                if (prvCpssDrvHwPpSetRegField(devNum, regAddr, fieldOffset, 6, position) != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                continue;
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChLedStreamPortPositionSet function
* @endinternal
*
* @brief   Sets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] position                 - port  index in LED stream
*                                      (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamPortPositionSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  position
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      portMacNum; /* MAC number */
    GT_U32      fieldLength; /* register field length */
    GT_U32      fieldOffset; /* register field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(position >= BIT_6)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
       || (PRV_CPSS_DXCH_XCAT3_A1_AND_ABOVE_CHECK_MAC(devNum) == GT_TRUE))
    {
        return prvCpssDxChBobcat2LedStreamPortPositionSet(devNum, portNum, position);
    }

    fieldLength = 6;

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
        fieldOffset = 5;
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,
                    PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum),&regAddr);
        fieldOffset = 10;
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, position);
    return rc;
}

/**
* @internal cpssDxChLedStreamPortPositionSet function
* @endinternal
*
* @brief   Sets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] position                 - port  index in LED stream
*                                      (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortPositionSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  position
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamPortPositionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, position));

    rc = internal_cpssDxChLedStreamPortPositionSet(devNum, portNum, position);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, position));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChBobcat2LedStreamPortPositionGet function
* @endinternal
*
* @brief   Gets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssDxChBobcat2LedStreamPortPositionGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                 *positionPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;                /* register address */
    GT_U32      fieldOffset;            /* register field offset */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* port MAC type */
    GT_U32      portMacNum;             /* MAC number */
    LED_PORT_FIELD_POSITIONS_STC  ledPortFieldsPositions;

    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT3_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        *positionPtr = PRV_CPSS_DXCH_PORT_LED_POSITION(devNum,portMacNum);
        return GT_OK;
    }


    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = falconRegInfoGet_LEDControl(devNum, portMacNum, portMacType, &regAddr);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = falconLedControlFiledsOffsetGet(devNum, portMacType, &ledPortFieldsPositions);
        if(rc != GT_OK)
        {
            return rc;
        }
        fieldOffset = ledPortFieldsPositions.ledPortNumberOffset;
    }
    else
    {
        switch (portMacType)
        {
            case PRV_CPSS_PORT_NOT_EXISTS_E:
            case PRV_CPSS_PORT_FE_E:
            case PRV_CPSS_PORT_GE_E:
                PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
                fieldOffset = 10;
                break;
            case PRV_CPSS_PORT_CG_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.CG[portMacNum].CG_CONVERTERS.CGMAControl1;
                fieldOffset = 0;
                break;
            case PRV_CPSS_PORT_XG_E:
            case PRV_CPSS_PORT_XLG_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
                fieldOffset = 5;
                break;
            case PRV_CPSS_PORT_ILKN_E:
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.ILKN_WRP.sdFcLedConvertersControl0;
                fieldOffset = 10;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    if (fieldOffset != NOT_USED)
    {
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr,
                                       fieldOffset, 6, positionPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChLedStreamPortPositionGet function
* @endinternal
*
* @brief   Gets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamPortPositionGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *positionPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      portMacNum; /* MAC number */
    GT_U32      fieldLength; /* register field length */
    GT_U32      fieldOffset; /* register field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(positionPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChBobcat2LedStreamPortPositionGet(devNum, portNum, positionPtr);
    }

    fieldLength = 6;

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].xgMibCountersCtrl;
        fieldOffset = 5;
    }
    else
    {
        PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,
                    PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum),&regAddr);
        fieldOffset = 10;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, positionPtr);
    return rc;
}

/**
* @internal cpssDxChLedStreamPortPositionGet function
* @endinternal
*
* @brief   Gets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamPortPositionGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *positionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamPortPositionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, positionPtr));

    rc = internal_cpssDxChLedStreamPortPositionGet(devNum, portNum, positionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, positionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamPortClassIndicationSet function
* @endinternal
*
* @brief   This routine set the port class indications
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGES: 0..5)
* @param[in] indication               - value of port class indication
*                                      (APPLICABLE VALUES:
*                                       CPSS_DXCH_LED_INDICATION_PORT_ENABLED_E
*                                       CPSS_DXCH_LED_INDICATION_LINK_E
*                                       CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E
*                                       CPSS_DXCH_LED_INDICATION_RX_ERROR_E
*                                       CPSS_DXCH_LED_INDICATION_FC_TX_E
*                                       CPSS_DXCH_LED_INDICATION_FC_RX_E
*                                       CPSS_DXCH_LED_INDICATION_FULL_DUPLEX_E
*                                       CPSS_DXCH_LED_INDICATION_GMII_SPEED_E
*                                       CPSS_DXCH_LED_INDICATION_MII_SPEED_E)
*
* @retval GT_OK                       - on success
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_FAIL                     - otherwise
* @retval GT_NOT_INITIALIZED          - port's LED indication is not set yet by cpssDxChLedStreamPortClassIndicationSet
*/
static GT_STATUS internal_cpssDxChLedStreamPortClassIndicationSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  CPSS_DXCH_LED_INDICATION_ENT    indication
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regAddr;            /* register address */
    GT_U32  bitOffset;          /* register field offset */
    GT_U32  ledInterfaceNum;    /* LED interface number */
    GT_U32  ledPort;            /* LED port number */
    GT_U32  portMacNum;         /* MAC number */
    GT_U32  regData;            /* register led control value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (classNum >= PRV_CPSS_DXCH_BOBCAT2_LED_INDICATION_CLASS_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortLedInterfaceGet(devNum, portNum, &ledInterfaceNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = cpssDxChLedStreamPortPositionGet(devNum,portNum,&ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChDevLedInterfacePortCheck(devNum, ledInterfaceNum,ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Port indication polarity */
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == 0)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].portIndicationSelect[ledPort];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledInterfaceNum].portIndicationSelect[ledPort];
    }

    switch(indication)
    {
        case CPSS_DXCH_LED_INDICATION_PORT_ENABLED_E:                  regData = 0;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_LINK_E:                          regData = 1;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E:                     regData = 2;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_RX_ERROR_E:                      regData = 3;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_FC_TX_E:                         regData = 4;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_FC_RX_E:                         regData = 5;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_FULL_DUPLEX_E:                   regData = 28;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_GMII_SPEED_E:                    regData = 29;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_MII_SPEED_E:                     regData = 30;
                                                                       break;
        case CPSS_DXCH_LED_INDICATION_COLLISION_E:                     regData = 6;
                                                                       break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Port class indication*/
    bitOffset = (classNum * 5);
    return prvCpssDrvHwPpSetRegField(devNum, regAddr, bitOffset, 5, regData);
}

/**
* @internal cpssDxChLedStreamPortClassIndicationSet function
* @endinternal
*
* @brief   This routine set the port class indications
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGES: 0..5)
* @param[in] indication               - value of port class indication
*                                      (APPLICABLE VALUES:
*                                       CPSS_DXCH_LED_INDICATION_PORT_ENABLED_E
*                                       CPSS_DXCH_LED_INDICATION_LINK_E
*                                       CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E
*                                       CPSS_DXCH_LED_INDICATION_RX_ERROR_E
*                                       CPSS_DXCH_LED_INDICATION_FC_TX_E
*                                       CPSS_DXCH_LED_INDICATION_FC_RX_E
*                                       CPSS_DXCH_LED_INDICATION_FULL_DUPLEX_E
*                                       CPSS_DXCH_LED_INDICATION_GMII_SPEED_E
*                                       CPSS_DXCH_LED_INDICATION_MII_SPEED_E)
*
* @retval GT_OK                       - on success
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_FAIL                     - otherwise
* @retval GT_NOT_INITIALIZED          - port's LED indication is not set yet by cpssDxChLedStreamPortClassIndicationSet
*/
GT_STATUS cpssDxChLedStreamPortClassIndicationSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  CPSS_DXCH_LED_INDICATION_ENT    indication
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamPortClassIndicationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, classNum, indication));

    rc = internal_cpssDxChLedStreamPortClassIndicationSet(devNum, portNum, classNum, indication);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, classNum, indication));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamPortClassIndicationGet function
* @endinternal
*
* @brief   This routine gets the port class indications
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGES: 0..5)
* @param[out] indicationPtr           - (pointer to) value of the port class indication.
*
* @retval GT_OK                       - on success
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM                - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_FAIL                     - otherwise
* @retval GT_NOT_INITIALIZED          - port's LED indication is not set yet by cpssDxChLedStreamPortClassIndicationSet
*/
static GT_STATUS internal_cpssDxChLedStreamPortClassIndicationGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    OUT CPSS_DXCH_LED_INDICATION_ENT    *indicationPtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regAddr;            /* register address */
    GT_U32  bitOffset;          /* register field offset */
    GT_U32  ledInterfaceNum;    /* LED interface number */
    GT_U32  ledPort;            /* LED port number */
    GT_U32  portMacNum;         /* MAC number */
    GT_U32  regData;            /* register led control value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);
    CPSS_NULL_PTR_CHECK_MAC(indicationPtr);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (classNum >= PRV_CPSS_DXCH_BOBCAT2_LED_INDICATION_CLASS_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortLedInterfaceGet(devNum, portNum, &ledInterfaceNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = cpssDxChLedStreamPortPositionGet(devNum,portNum,&ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChDevLedInterfacePortCheck(devNum, ledInterfaceNum, ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Port indication polarity */
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == 0)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].portIndicationSelect[ledPort];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledInterfaceNum].portIndicationSelect[ledPort];
    }
    bitOffset = classNum * 5;

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, bitOffset, 5, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    switch(regData)
    {
        case 0:  *indicationPtr = CPSS_DXCH_LED_INDICATION_PORT_ENABLED_E;
                 break;
        case 1:  *indicationPtr = CPSS_DXCH_LED_INDICATION_LINK_E;
                 break;
        case 2:  *indicationPtr = CPSS_DXCH_LED_INDICATION_RX_TX_ACT_E;
                 break;
        case 3:  *indicationPtr = CPSS_DXCH_LED_INDICATION_RX_ERROR_E;
                 break;
        case 4: *indicationPtr = CPSS_DXCH_LED_INDICATION_FC_TX_E;
                 break;
        case 5: *indicationPtr = CPSS_DXCH_LED_INDICATION_FC_RX_E;
                 break;
        case 28: *indicationPtr = CPSS_DXCH_LED_INDICATION_FULL_DUPLEX_E;
                 break;
        case 29: *indicationPtr = CPSS_DXCH_LED_INDICATION_GMII_SPEED_E;
                 break;
        case 30: *indicationPtr = CPSS_DXCH_LED_INDICATION_MII_SPEED_E;
                 break;
        case 6: *indicationPtr = CPSS_DXCH_LED_INDICATION_COLLISION_E;
                 break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal cpssDxChLedStreamPortClassIndicationGet function
* @endinternal
*
* @brief   This routine gets the port class indications
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGES: 0..5)
* @param[out] indicationPtr           - (pointer to) value of the port class indication.
*
* @retval GT_OK                       - on success
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM                - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_FAIL                     - otherwise
* @retval GT_NOT_INITIALIZED          - port's LED indication is not set yet by cpssDxChLedStreamPortClassIndicationSet
*/
GT_STATUS cpssDxChLedStreamPortClassIndicationGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    OUT CPSS_DXCH_LED_INDICATION_ENT    *indicationPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamPortClassIndicationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, classNum, indicationPtr));

    rc = internal_cpssDxChLedStreamPortClassIndicationGet(devNum, portNum, classNum, indicationPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, classNum, indicationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamPortClassPolarityInvertEnableSet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGES: 0..5)
* @param[in] invertEnable             - status of the selected polarity indication.
*                                      GT_TRUE  - polarity inverted
*                                      GT_FALSE - polarity not inverted
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_INITIALIZED       - port's LED position is not set yet by cpssDxChLedStreamPortPositionSet
*/
static GT_STATUS internal_cpssDxChLedStreamPortClassPolarityInvertEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  GT_BOOL                         invertEnable
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regAddr;            /* register address */
    GT_U32  bitOffset;          /* register field offset */
    GT_U32  ledInterfaceNum;    /* LED interface number */
    GT_U32  ledPort;            /* LED port number */
    GT_U32  portMacNum;         /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (classNum >= PRV_CPSS_DXCH_BOBCAT2_LED_INDICATION_CLASS_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortLedInterfaceGet(devNum, portNum, &ledInterfaceNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChLedStreamPortPositionGet(devNum,portNum,&ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChDevLedInterfacePortCheck(devNum,ledInterfaceNum,ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Port indication polarity */
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == 0)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].portIndicationPolarity[ledPort/4];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledInterfaceNum].portIndicationPolarity[ledPort/4];
    }
    bitOffset = (ledPort % 4) * 6 + classNum;

    return prvCpssDrvHwPpSetRegField(devNum, regAddr, bitOffset, 1, ((invertEnable == GT_TRUE) ? 0 : 1));
}

/**
* @internal cpssDxChLedStreamPortClassPolarityInvertEnableSet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGES: 0..5)
* @param[in] invertEnable             - status of the selected polarity indication.
*                                      GT_TRUE  - polarity inverted
*                                      GT_FALSE - polarity not inverted
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_INITIALIZED       - port's LED position is not set yet by cpssDxChLedStreamPortPositionSet
*/
GT_STATUS cpssDxChLedStreamPortClassPolarityInvertEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  GT_BOOL                         invertEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamPortClassPolarityInvertEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, classNum, invertEnable));

    rc = internal_cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum, portNum, classNum, invertEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, classNum, invertEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamPortClassPolarityInvertEnableGet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGES: 0..5)
*
* @param[out] invertEnablePtr          - (pointer to) status of the selected polarity indication.
*                                      GT_TRUE  - polarity inverted
*                                      GT_FALSE - polarity not inverted
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_INITIALIZED       - port's LED position is not set yet by cpssDxChLedStreamPortPositionSet
*/
static GT_STATUS internal_cpssDxChLedStreamPortClassPolarityInvertEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  GT_BOOL                         *invertEnablePtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regAddr;            /* register address */
    GT_U32  bitOffset;          /* register field offset */
    GT_U32  fieldValue;         /* register field value */
    GT_U32  ledInterfaceNum;    /* LED interface number */
    GT_U32  ledPort;            /* LED port number */
    GT_U32  portMacNum;         /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (classNum >= PRV_CPSS_DXCH_BOBCAT2_LED_INDICATION_CLASS_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortLedInterfaceGet(devNum, portNum, &ledInterfaceNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = cpssDxChLedStreamPortPositionGet(devNum,portNum,&ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortLedInterfaceGet(devNum, portNum, &ledInterfaceNum);
    if( rc != GT_OK )
    {
        return rc;
    }


    rc = prvCpssDxChDevLedInterfacePortCheck(devNum, ledInterfaceNum,ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* Port indication polarity */
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == 0)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].portIndicationPolarity[ledPort/4];
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->sip6_LED[ledInterfaceNum].portIndicationPolarity[ledPort/4];
    }
    bitOffset = (ledPort % 4) * 6 + classNum;

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, bitOffset, 1, &fieldValue);
    if( rc != GT_OK )
    {
        return rc;
    }

    *invertEnablePtr =  (fieldValue == 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChLedStreamPortClassPolarityInvertEnableGet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number.
*                                      (APPLICABLE RANGES: 0..5)
*
* @param[out] invertEnablePtr          - (pointer to) status of the selected polarity indication.
*                                      GT_TRUE  - polarity inverted
*                                      GT_FALSE - polarity not inverted
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_INITIALIZED       - port's LED position is not set yet by cpssDxChLedStreamPortPositionSet
*/
GT_STATUS cpssDxChLedStreamPortClassPolarityInvertEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    OUT GT_BOOL                         *invertEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamPortClassPolarityInvertEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, classNum, invertEnablePtr));

    rc = internal_cpssDxChLedStreamPortClassPolarityInvertEnableGet(devNum, portNum, classNum, invertEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, classNum, invertEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedPhyControlGlobalSet function
* @endinternal
*
* @brief   Set global (per-PHY) LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports.
* @param[in] ledGlobalConfigPtr       - (pointer to) global configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
static GT_STATUS internal_cpssDxChLedPhyControlGlobalSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  *ledGlobalConfigPtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacLedGlobalConfigSetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacLedGlobalConfigSetFunc(devNum,portNum,
                                  ledGlobalConfigPtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChLedPhyControlGlobalSet function
* @endinternal
*
* @brief   Set global (per-PHY) LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports.
* @param[in] ledGlobalConfigPtr       - (pointer to) global configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChLedPhyControlGlobalSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  *ledGlobalConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedPhyControlGlobalSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ledGlobalConfigPtr));

    rc = internal_cpssDxChLedPhyControlGlobalSet(devNum, portNum, ledGlobalConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ledGlobalConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedPhyControlGlobalGet function
* @endinternal
*
* @brief   Get global (per-PHY) LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports.
*
* @param[out] ledGlobalConfigPtr       - (pointer to) global configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
static GT_STATUS internal_cpssDxChLedPhyControlGlobalGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  *ledGlobalConfigPtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacLedGlobalConfigGetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacLedGlobalConfigGetFunc(devNum,portNum,
                                  ledGlobalConfigPtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChLedPhyControlGlobalGet function
* @endinternal
*
* @brief   Get global (per-PHY) LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - connecting physical port number connected to remote ports.
*
* @param[out] ledGlobalConfigPtr       - (pointer to) global configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChLedPhyControlGlobalGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC  *ledGlobalConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedPhyControlGlobalGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ledGlobalConfigPtr));

    rc = internal_cpssDxChLedPhyControlGlobalGet(devNum, portNum, ledGlobalConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ledGlobalConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedPhyControlPerPortSet function
* @endinternal
*
* @brief   Set per-port LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number of remote port.
* @param[in] ledPerPortConfigPtr      - (pointer to) per-port configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
static GT_STATUS internal_cpssDxChLedPhyControlPerPortSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_LED_PHY_PORT_CONF_STC    *ledPerPortConfigPtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacLedPerPortConfigSetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacLedPerPortConfigSetFunc(devNum,portNum,
                                  ledPerPortConfigPtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChLedPhyControlPerPortSet function
* @endinternal
*
* @brief   Set per-port LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number of remote port.
* @param[in] ledPerPortConfigPtr      - (pointer to) per-port configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChLedPhyControlPerPortSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_LED_PHY_PORT_CONF_STC    *ledPerPortConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedPhyControlPerPortSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ledPerPortConfigPtr));

    rc = internal_cpssDxChLedPhyControlPerPortSet(devNum, portNum, ledPerPortConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ledPerPortConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedPhyControlPerPortGet function
* @endinternal
*
* @brief   Get per-port LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number of remote port.
*
* @param[out] ledPerPortConfigPtr      - (pointer to) per-port configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
static GT_STATUS internal_cpssDxChLedPhyControlPerPortGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_DXCH_LED_PHY_PORT_CONF_STC    *ledPerPortConfigPtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(devNum,portNum,portMacNum);

    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback */
    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacLedPerPortConfigGetFunc)
    {
        GT_BOOL doPpMacConfig = GT_TRUE;

        rc = portMacObjPtr->macDrvMacLedPerPortConfigGetFunc(devNum,portNum,
                                  ledPerPortConfigPtr,CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChLedPhyControlPerPortGet function
* @endinternal
*
* @brief   Get per-port LED configuration.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number of remote port.
*
* @param[out] ledPerPortConfigPtr      - (pointer to) per-port configuration
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note Applicable only to remote ports on AC3X.
*
*/
GT_STATUS cpssDxChLedPhyControlPerPortGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_DXCH_LED_PHY_PORT_CONF_STC    *ledPerPortConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedPhyControlPerPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ledPerPortConfigPtr));

    rc = internal_cpssDxChLedPhyControlPerPortGet(devNum, portNum, ledPerPortConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ledPerPortConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChLedStreamTwoClassModeSet function
* @endinternal
*
* @brief   This routine configures LED stream two-class mode configuration parameters.
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - physical device number
* @param[in] ledTwoClassModeConfPtr - (pointer to) LED stream two-class mode specific configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters value is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamTwoClassModeSet
(
    IN  GT_U8                        devNum,
    IN  CPSS_LED_TWO_CLASS_MODE_CONF_STC *ledTwoClassModeConfPtr
)
{
    GT_STATUS rc;
    GT_STATUS regAddr;
    GT_U32 regData;
    GT_U32 fieldValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AC5_E);
    CPSS_NULL_PTR_CHECK_MAC(ledTwoClassModeConfPtr);

    if (ledTwoClassModeConfPtr->ledClassSelectChain0 >= PRV_CPSS_DXCH_AC5_MAX_LED_CLASS_CNS ||
        ledTwoClassModeConfPtr->ledClassSelectChain1 >= PRV_CPSS_DXCH_AC5_MAX_LED_CLASS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                  DFXServerUnitsDeviceSpecificRegs.deviceCtrl27;
    rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    fieldValue = BOOL2BIT_MAC(ledTwoClassModeConfPtr->ledTwoClassModeEnable);
    U32_SET_FIELD_MAC(regData,  6, 1, fieldValue);
    U32_SET_FIELD_MAC(regData,  7, 5, ledTwoClassModeConfPtr->ledClassSelectChain0);
    U32_SET_FIELD_MAC(regData, 12, 5, ledTwoClassModeConfPtr->ledClassSelectChain1);

    return prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum, regAddr, regData);
}

/**
* @internal cpssDxChLedStreamTwoClassModeSet function
* @endinternal
*
* @brief   This routine configures LED stream two-class mode configuration parameters.
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - physical device number
* @param[in] ledTwoClassModeConfPtr - (pointer to) LED stream two-class mode specific configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters value is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChLedStreamTwoClassModeSet
(
    IN  GT_U8                        devNum,
    IN  CPSS_LED_TWO_CLASS_MODE_CONF_STC *ledTwoClassModeConfPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamTwoClassModeSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledTwoClassModeConfPtr));

    rc = internal_cpssDxChLedStreamTwoClassModeSet(devNum, ledTwoClassModeConfPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledTwoClassModeConfPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedStreamTwoClassModeGet function
* @endinternal
*
* @brief   Get LED stream two-class mode configuration parameters.
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - physical device number
* @param[out] ledTwoClassModeConfPtr   - (pointer to) LED stream two-class mode specific configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedStreamTwoClassModeGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_LED_TWO_CLASS_MODE_CONF_STC *ledTwoClassModeConfPtr
)
{
    GT_STATUS rc;
    GT_STATUS regAddr;
    GT_U32 regData;
    GT_U32 fieldValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AC5_E);
    CPSS_NULL_PTR_CHECK_MAC(ledTwoClassModeConfPtr);

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                  DFXServerUnitsDeviceSpecificRegs.deviceCtrl27;
    rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    fieldValue = U32_GET_FIELD_MAC(regData, 6,  1);
    ledTwoClassModeConfPtr->ledTwoClassModeEnable = BIT2BOOL_MAC(fieldValue);
    ledTwoClassModeConfPtr->ledClassSelectChain0 = U32_GET_FIELD_MAC(regData,  7, 5);
    ledTwoClassModeConfPtr->ledClassSelectChain1 = U32_GET_FIELD_MAC(regData, 12, 5);

    return GT_OK;
}
/**
* @internal cpssDxChLedStreamTwoClassModeGet function
* @endinternal
*
* @brief   Get LED stream two-class mode configuration parameters.
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - physical device number
* @param[out] ledTwoClassModeConfPtr   - (pointer to) LED stream two-class mode specific configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChLedStreamTwoClassModeGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_LED_TWO_CLASS_MODE_CONF_STC *ledTwoClassModeConfPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamTwoClassModeGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledTwoClassModeConfPtr));

    rc = internal_cpssDxChLedStreamTwoClassModeGet(devNum, ledTwoClassModeConfPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledTwoClassModeConfPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedUnitClassManipulationSet function
* @endinternal
*
* @brief   Sets LED Unit class manipulation parameters
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] ledUnitIndex          - LED Unit index
*                                   (APPLICABLE RANGES:
*                                    see description of LED unit index in description of CPSS_LED_SIP6_CONF_STC)
* @param[in] classNum              - class number.
*                                    (APPLICABLE RANGE: 0..5)
* @param[in] classParamsPtr        - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedUnitClassManipulationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledUnitIndex,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E
                                            | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(classParamsPtr);

    if (ledUnitIndex >= PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(classParamsPtr->blinkSelect)
    {
        case CPSS_LED_BLINK_SELECT_0_E:
        case CPSS_LED_BLINK_SELECT_1_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChSip6LedUnitClassManipulationSet(devNum, ledUnitIndex, classNum, classParamsPtr);
}

/**
* @internal cpssDxChLedStreamUnitClassManipulationSet function
* @endinternal
*
* @brief   Sets LED Unit class manipulation parameters
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] ledUnitIndex          - LED Unit index
*                                   (APPLICABLE RANGES:
*                                    see description of LED unit index in description of CPSS_LED_SIP6_CONF_STC)
* @param[in] classNum              - class number.
*                                    (APPLICABLE RANGE: 0..5)
* @param[in] classParamsPtr        - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamUnitClassManipulationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledUnitIndex,
    IN  GT_U32                          classNum,
    IN  CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamUnitClassManipulationSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, ledUnitIndex, classNum, classParamsPtr));

    rc = internal_cpssDxChLedUnitClassManipulationSet(devNum, ledUnitIndex, classNum, classParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, ledUnitIndex, classNum, classParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChLedUnitClassManipulationGet function
* @endinternal
*
* @brief  Gets LED Unit class manipulation parameters
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] ledUnitIndex          - LED Unit index
*                                   (APPLICABLE RANGES:
*                                    see description of LED unit index in description of CPSS_LED_SIP6_CONF_STC)
* @param[in] classNum              - class number
*                                    (APPLICABLE RANGES: 0..5)
*
* @param[out] classParamsPtr       - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssDxChLedUnitClassManipulationGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledUnitIndex,
    IN  GT_U32                          classNum,
    OUT CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_U32 ledUnitNumber;           /* LED unit number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                            | CPSS_ALDRIN_E | CPSS_AC3X_E
                                            | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(classParamsPtr);

    if (ledUnitIndex >= PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    LED_UNIT_NUMBER_BY_INDEX_GET_MAC(devNum, ledUnitIndex, ledUnitNumber);

    return prvCpssDxChFalconLedUnitClassManipulationGet(devNum, ledUnitNumber, classNum, classParamsPtr);
}

/**
* @internal cpssDxChLedStreamUnitClassManipulationGet function
* @endinternal
*
* @brief  Gets LED Unit class manipulation parameters
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] ledUnitIndex          - LED Unit index
*                                   (APPLICABLE RANGES:
*                                    see description of LED unit index in description of CPSS_LED_SIP6_CONF_STC)
* @param[in] classNum              - class number
*                                    (APPLICABLE RANGES: 0..5)
*
* @param[out] classParamsPtr       - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssDxChLedStreamUnitClassManipulationGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ledUnitIndex,
    IN  GT_U32                          classNum,
    OUT CPSS_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChLedStreamUnitClassManipulationGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledUnitIndex, classNum, classParamsPtr));

    rc = internal_cpssDxChLedUnitClassManipulationGet(devNum, ledUnitIndex, classNum, classParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledUnitIndex, classNum, classParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

