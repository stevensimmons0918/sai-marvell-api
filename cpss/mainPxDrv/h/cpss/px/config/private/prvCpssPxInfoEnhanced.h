/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file prvCpssPxInfoEnhanced.h
*
* @brief This file includes the declaration of the structure to hold the
* enhanced parameters that "fine tuning" the PP's capabilities in CPSS
* point of view.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPxInfoEnhanced_h
#define __prvCpssPxInfoEnhanced_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxHwTables.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>


/**
* @struct PRV_CPSS_PX_PP_CONFIG_FINE_TUNING_STC
 *
 * @brief A Structure including PP's tables sizes and parameters that
 * define the "fine tuning" , of the specific device.
 * Notes:
 * the initialization of those fields should be done during "init phase".
*/
typedef struct{
    struct{
        /* CNC section */
        GT_U32 cncBlocks;
        GT_U32 cncBlockNumEntries;

        /* Transmit Descriptors */
        GT_U32 transmitDescr;

        /* Buffer Memory */
        GT_U32 bufferMemory;

        /* pipe BW capacity for Rx-DMA Tx-DMA TxQ Tx-FIFO Eth-TxFIFO units */
        GT_U32 pipeBWCapacityInGbps;

        /****************************************************/
        /* add here other table sizes that are "parametric" */
        /****************************************************/
    }tableSize;

}PRV_CPSS_PX_PP_CONFIG_FINE_TUNING_STC;

/**
* @enum PRV_CPSS_PX_PORT_TYPE_ENT
 *
 * @brief port type used in mapping functions
*/
typedef enum{

    /** invalide source */
    PRV_CPSS_PX_PORT_TYPE_INVALID_E = -1,

    /** physical port */
    PRV_CPSS_PX_PORT_TYPE_PHYS_E = 0,

    /** rxDma port */
    PRV_CPSS_PX_PORT_TYPE_RxDMA_E,

    /** txDma port */
    PRV_CPSS_PX_PORT_TYPE_TxDMA_E,

    /** mac number */
    PRV_CPSS_PX_PORT_TYPE_MAC_E,

    /** txq port */
    PRV_CPSS_PX_PORT_TYPE_TXQ_E,

    /** max port , used in array declaration */
    PRV_CPSS_PX_PORT_TYPE_MAX_E

} PRV_CPSS_PX_PORT_TYPE_ENT;

/**
* @struct PRV_CPSS_PX_PP_HW_INFO_STC
 *
 * @brief A Structure including PP's HW major differences between devices.
 * Notes:
 * the initialization of those fields should be done during "phase 1".
*/
typedef struct{
    struct{
        GT_U32 cncUnits;
        GT_U32 cncBlocks;
        GT_U32 cncBlockNumEntries;
        GT_BOOL cncUploadUseDfxUnit;
    }cnc;

    struct {
        GT_U32  numberOfSmiIf; /* mu,ber of smi interfaces */
        GT_U32  activeSMIList[12];
        GT_BOOL activeSMIBmp [12];
        GT_U32  smiAutoPollingList[12]; /* whether there any out-of-band out polling ports , ex: BC2 ports 0-47 1G */
    }smi_support;

    struct{
        GT_U32  numTxqDq;/* Pipe hold 2 DQs ,
                            but single : rxdma,txdma,txfifo */
        GT_U32  txqDqNumPortsPerDp;/* per DP : number of 'TXQ_ports' that TXQ_DQ supports */
        GT_U32  mapPhyPortToTxqDqUnitIndex[PRV_CPSS_PX_MAX_DQ_PORTS_CNS];/* index to the array is 'Physical port number'
                                    the value is 'TQX-DQ' unit index */
    }multiDataPath;

}PRV_CPSS_PX_PP_HW_INFO_STC;

/* macro to get pointer to the hwInfo.cnc in PRV_CPSS_PX_PP_HW_INFO_STC */
#define PRV_CPSS_PX_PP_HW_INFO_CNC_MAC(_devNum)    \
    PRV_CPSS_PX_PP_MAC(_devNum)->hwInfo.cnc

/* private MACRO -- NOT to be used directly by CPSS code.
   only from the below macros */
#define __PX_PORT_NUM_CHECK_AND_CONVERT_MAC(_devNum,_portNum,_convertedPort,_convertType)  \
    {                                                                                   \
        GT_STATUS _rc;                                                                  \
        _rc = prvCpssPxPortPhysicalPortMapCheckAndConvert(_devNum, _portNum, _convertType, &_convertedPort);\
        if(GT_OK != _rc)                                                                \
        {                                                                               \
            return _rc;                                                                 \
        }                                                                               \
    }

/* check and get MAC number for given physical port */
#define PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(_devNum,_portNum,_portMacNum)  \
    __PX_PORT_NUM_CHECK_AND_CONVERT_MAC(_devNum,_portNum,_portMacNum,PRV_CPSS_PX_PORT_TYPE_MAC_E)

/* check and get TXQ number for given physical port */
#define PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(_devNum,_portNum,_portTxqNum) \
    __PX_PORT_NUM_CHECK_AND_CONVERT_MAC(_devNum,_portNum,_portTxqNum,PRV_CPSS_PX_PORT_TYPE_TXQ_E)

/* check and get RX_DMA number for given physical port */
#define PRV_CPSS_PX_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(_devNum,_portNum,_portRxdmaNum) \
    __PX_PORT_NUM_CHECK_AND_CONVERT_MAC(_devNum,_portNum,_portRxdmaNum,PRV_CPSS_PX_PORT_TYPE_RxDMA_E)

/* check and get TX_DMA number for given physical port */
#define PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXDMA_NUM_GET_MAC(_devNum,_portNum,_portTxdmaNum) \
    __PX_PORT_NUM_CHECK_AND_CONVERT_MAC(_devNum,_portNum,_portTxdmaNum,PRV_CPSS_PX_PORT_TYPE_TxDMA_E)

/* Pipe convert global txq-dq port number to DQ unit index */
#define PRV_CPSS_PX_GLOBAL_TXQ_DQ_PORT_TO_DQ_UNIT_INDEX_MAC(dev,globalTxqDqPortIndex) \
    ((globalTxqDqPortIndex) / PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp)

/* Pipe convert local txq-dq port number in specific DQ unit to global txq-dq port number */
#define PRV_CPSS_PX_LOCAL_TXQ_DQ_PORT_TO_GLOBAL_TXQ_DQ_PORT_MAC(dev,localTxqDqPortIndex,dqIndex) \
    ((localTxqDqPortIndex) + ((dqIndex) * PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp))

/* Pipe convert global txq-dq port number to local txq-dq port number */
#define PRV_CPSS_PX_GLOBAL_TXQ_DQ_PORT_TO_LOCAL_TXQ_DQ_PORT_MAC(dev,globalTxqDqPortIndex) \
    ((globalTxqDqPortIndex) % PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp)

/* Pipe access to the DB of 'per port' the txq-Dq unit index */
#define PRV_CPSS_PX_TXQ_DQ_UNIT_INDEX_OF_PORT_MAC(dev,_physPort)   \
    PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.mapPhyPortToTxqDqUnitIndex[_physPort]

/* Pipe the max number of global txq-Dq ports */
#define PRV_CPSS_PX_TXQ_DQ_MAX_PORTS_MAC(dev) \
    (PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.txqDqNumPortsPerDp * \
     PRV_CPSS_PX_PP_MAC(dev)->hwInfo.multiDataPath.numTxqDq)

/* check that port mapping access check is used */
/*#define PRV_CPSS_PX_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(_devNum) \
    (PRV_CPSS_PX_PP_MAC(_devNum)->hwInfo.portMapping.isUsedPortMappingCheck)*/

/**
* @internal prvCpssPxPortPhysicalPortMapCheckAndConvert function
* @endinternal
*
* @brief   Function checks and gets valid mapping from shadow DB.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] physicalPortNum          - physical port to convert
* @param[in] convertedPortType        - converted port type
*
* @param[out] convertedPortNumPtr      - (pointer to) converted port, according to port type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortPhysicalPortMapCheckAndConvert
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            physicalPortNum,
    IN  PRV_CPSS_PX_PORT_TYPE_ENT     convertedPortType,
    OUT GT_U32                          *convertedPortNumPtr
);


/**
* @internal prvCpssPxPortPhysicalPortMapReverseMappingGet function
* @endinternal
*
* @brief   Function checks and returns physical port number that mapped to given
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] origPortType             - original port type
* @param[in] origPortNum              - MAC/TXQ/DMA port number
*
* @param[out] physicalPortNumPtr       - (pointer to) physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, origPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortPhysicalPortMapReverseMappingGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  PRV_CPSS_PX_PORT_TYPE_ENT     origPortType,
    IN  GT_U32                          origPortNum,
    OUT GT_PHYSICAL_PORT_NUM            *physicalPortNumPtr
);

/**
* @internal prvCpssPxPortEventPortMapConvert function
* @endinternal
*
* @brief   Function checks and gets valid mapping for port per event type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] evConvertType            - event convert type
* @param[in] portNumFrom              - port to convert
*
* @param[out] portNumToPtr             - (pointer to) converted port, according to convert event type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNumFrom
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortEventPortMapConvert
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  PRV_CPSS_EV_CONVERT_DIRECTION_ENT    evConvertType,
    IN  GT_U32                          portNumFrom,
    OUT GT_U32                          *portNumToPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxInfoEnhanced_h */


