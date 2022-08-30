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
* @file cpssPxPortMapping.h
*
* @brief CPSS physical port mapping
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxPortMapping_h
#define __cpssPxPortMapping_h

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GT_PX_NA                                 ((GT_U32)~0)  /* used in mapping */
#define CPSS_PX_PORT_MAPPING_INVALID_PORT_CNS   GT_PX_NA

/**
* @enum CPSS_PX_PORT_MAPPING_TYPE_ENT
 *
 * @brief interface types of physical ports.
*/
typedef enum{

    /** Ethernet MAC */
    CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E = 0

    /** CPU SDMA */
    ,CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E

    /** @brief max definition
     *  CPSS_PX_PORT_MAPPING_TYPE_INVALID_E    invalid value
     */
    ,CPSS_PX_PORT_MAPPING_TYPE_MAX_E

    ,CPSS_PX_PORT_MAPPING_TYPE_INVALID_E = ~0

} CPSS_PX_PORT_MAPPING_TYPE_ENT;


#define PRV_CPSS_PX_PORT_NUM_CPU_DMA_CNS        16

/**
* @struct CPSS_PX_PORT_MAP_STC
 *
 * @brief General physical port map structure
*/
typedef struct{

    /** @brief physical port number
     *  (APPLICABLE RANGES: 0..16)
     */
    GT_PHYSICAL_PORT_NUM physicalPortNumber;

    /** @brief mapping type
     *  interfaceNumber  - interface number.
     *  (APPLICABLE RANGES: 0..15)
     *  IGNORED when mappingType == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E
     *  Comments:
     *  In the PIPE : The <txqPortNumber> is equal to <physicalPortNumber>
     */
    CPSS_PX_PORT_MAPPING_TYPE_ENT mappingType;

    GT_U32 interfaceNum;

} CPSS_PX_PORT_MAP_STC;



/**
* @struct CPSS_PX_SHADOW_PORT_MAP_STC
 *
 * @brief A structure to hold detailed information about
 * physical port mapping.
*/
typedef struct{

    /** mapping type */
    CPSS_PX_PORT_MAPPING_TYPE_ENT mappingType;

    /** @brief MAC Number for ETHERNET map type
     *  Parameter is ignored for other mapping types.
     *  (APPLICABLE RANGES: 0..15)
     */
    GT_U32 macNum;

    /** RXDMA/TXDMA/TxFIFO port number (APPLICABLE RANGES: 0..16) */
    GT_U32 dmaNum;

    /** @brief TXQ port number (APPLICABLE RANGES: 0..19)
     *  NOTE: ALWAYS equal to 'physicalPortNumber' (HARD WIRED in HW)
     *  so actual (APPLICABLE RANGES: 0..16)
     */
    GT_U32 txqNum;

} CPSS_PX_SHADOW_PORT_MAP_STC;


/**
* @struct CPSS_PX_DETAILED_PORT_MAP_STC
 *
 * @brief A structure to hold shadow of physical port mapping.
*/
typedef struct{

    /** @brief valid mapping
     *  GT_TRUE - valid mapping, was checked and set by cpssPxPortPhysicalPortMapSet
     *  GT_FALSE - not valid mapping
     */
    GT_BOOL valid;

    /** structure that hold detailed port mapping */
    CPSS_PX_SHADOW_PORT_MAP_STC portMap;

} CPSS_PX_DETAILED_PORT_MAP_STC;

/**
* @internal cpssPxPortPhysicalPortMapSet function
* @endinternal
*
* @brief   configure ports mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portMapArraySize         - Number of ports to map, array size
* @param[in] portMapArrayPtr          - pointer to array of mappings
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. All usable ports shall be configured at ONCE. Any port, that was mapped previously and is
*       not included into current mapping, losses its mapping.
*       2. Changing mapping under traffic may cause to traffic interruption.
*       3. The function checks whether all mapping are consistent.
*       In case of contradiction between mappings
*       (e,g. single TxQ port is mapped to several Tx DMA ports) the function returns with BAD_PARAM.
*       5. Other CPSS APIs that receive GT_PHYSICAL_PORT_NUM as input parameter will use this mapping
*       to convert the physical port to mac/dma/txq to configure them
*       6. Usage example
*       CPSS_PX_PORT_MAP_STC exampleMap[] =
*       (
*       -- the list for 'network ports' : 10G each
*       ( 0, _INTRFACE_TYPE_ETHERNET_MAC_E,   0, 0),
*       ( 1, _INTRFACE_TYPE_ETHERNET_MAC_E,   1, 1),
*       ...
*       ( 11, _INTRFACE_TYPE_ETHERNET_MAC_E,   11, 11),
*       -- the 'uplink port' : 100G
*       ( 12, _INTRFACE_TYPE_ETHERNET_MAC_E,   12, 12),
*       -- the SDMA CPU
*       ( 16, _INTRFACE_TYPE_CPU_SDMA_E,   GT_NA, 16),
*       );
*       rc = cpssPxCfgPhysicalPortMapSet(devNum, sizeof(exampleMap)/sizeof(exampleMap[0]), &exampleMap[0]);
*       if(rc != GT_OK)
*       (
*       fatal error.
*       )
*
*/
GT_STATUS cpssPxPortPhysicalPortMapSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_PX_PORT_MAP_STC       *portMapArrayPtr
);

/**
* @internal cpssPxPortPhysicalPortMapGet function
* @endinternal
*
* @brief   get ports mapping
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] firstPhysicalPortNumber  - first physical port to bring
* @param[in] portMapArraySize         - Mapping array size
*
* @param[out] portMapArrayPtr          - pointer to ports mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_FAIL                  - when cannot deliver requested number of items
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPhysicalPortMapGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN GT_PHYSICAL_PORT_NUM        firstPhysicalPortNumber,
    IN GT_U32                      portMapArraySize,
    OUT CPSS_PX_PORT_MAP_STC      *portMapArrayPtr
);


/**
* @internal cpssPxPortPhysicalPortMapIsValidGet function
* @endinternal
*
* @brief   Check if physical port was mapped
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isValidPtr               - (pointer to) is valid mapping
*                                      GT_TRUE - given port was mapped
*                                      GT_FALSE - there is no valid mapping for given port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPhysicalPortMapIsValidGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isValidPtr
);

/**
* @internal cpssPxPortPhysicalPortMapReverseMappingGet function
* @endinternal
*
* @brief   Function checks and returns physical port number that mapped to given
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Pipe
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
* @retval GT_NOT_FOUND             - not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPhysicalPortMapReverseMappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_PX_PORT_MAPPING_TYPE_ENT origPortType,
    IN  GT_U32                          origPortNum,
    OUT GT_PHYSICAL_PORT_NUM            *physicalPortNumPtr
);

/**
* @internal cpssPxPortPhysicalPortDetailedMapGet function
* @endinternal
*
* @brief   Function returns physical port detailed mapping
*         MAC/TXQ/DMA port number.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portMapShadowPtr         - (pointer to) detailed mapping
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPhysicalPortDetailedMapGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN   GT_PHYSICAL_PORT_NUM    portNum,
    OUT  CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr
);


/**
* @internal prvCpssPxPpResourcesTxqGlobal2LocalConvert function
* @endinternal
*
* @brief   Function converts global TXQ to dq idx and local txq-dq port
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] txqNum                   - global TXQ port number
*
* @param[out] dqIdxPtr                 - (pointer to) dq idx
* @param[out] localTxqNumPtr           - (pointer to) local txq number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPpResourcesTxqGlobal2LocalConvert
(

    IN  GT_SW_DEV_NUM devNum,
    IN  GT_U32 txqNum,
    OUT GT_U32 *dqIdxPtr,
    OUT GT_U32 *localTxqNumPtr
);



/**
* @internal cpssPxPortPhysicalPortMapIsCpuGet function
* @endinternal
*
* @brief   Check if physical port was mapped as CPU port
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isCpuPtr                 - (pointer to) is mapped as CPU port
*                                      GT_TRUE - given port was mapped as CPU port
*                                      GT_FALSE - there is no valid mapping for given port,
*                                      or it is not mapped as CPU port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPhysicalPortMapIsCpuGet
(
    IN    GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isCpuPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*__cpssPxPortMapping_h*/


