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
* @file snetFalconEgress.h
*
* @brief Falcon Egress processing
*
* @version   1
********************************************************************************
*/
#ifndef __snetFalconEgressh
#define __snetFalconEgressh

#include <asicSimulation/SKernel/smain/smain.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal snetFalconPreqTablesFormatInit function
* @endinternal
*
* @brief   init the format of preq tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetFalconPreqTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
);



/**
* @internal snetFalconEgressProcess function
* @endinternal
*
* @brief   Falcon : do Egress processing .
*         in the Control pipe : QAG --> HA --> EPCL --> PRE-Q --> EOAM --> EPLR --> ERMRK --> PHA --> EREP --> DP(RxDma).
*         in the Data Path  : --> RxDma --> TXQ-PDX --> (egress tile) TX-SDQ --> (TXFIFO,TXDMA) --> MAC.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - <localDevTargetPort> the egress physical port.
*                                       indication that packet was send to the DMA of the port
*                                       COMMENTS :
*/
GT_BOOL snetFalconEgressProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
);

/**
* @internal snetPreqProcess function
* @endinternal
*
* @brief   PREQ - supported on SIP6 devices
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - local egress port (not global).
*/
void snetPreqProcess
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
  IN GT_U32                          egressPort
);

/**
* @internal sip6MacD2dEagleIsPortConfig function
* @endinternal
*
* @brief   Falcon : validate port configuration on eagle D2D.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txFifoPort                - global dma port number.
* @param[in] isIngress                - Egress or Ingress
*
* @retval is port config               - GT_TRUE : port have at
*                                       least one slices.
*                                       GT_FALSE : port don't
*                                       have slices
*/
GT_BOOL sip6MacD2dEagleIsPortConfig
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                txFifoPort,
    IN GT_BOOL               isIngress
);


/**
* @internal sip6MacD2dRavenIsPortConfig function
* @endinternal
*
* @brief   Falcon : validate port configuration on raven D2D.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txFifoPort                - global dma port number.
* @param[in] isIngress                - Egress or Ingress
*
* @retval is port config               - GT_TRUE : port have at
*                                       least one slices.
*                                       GT_FALSE : port don't
*                                       have slices
*/
GT_BOOL sip6MacD2dRavenIsPortConfig
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                txFifoPort,
    IN GT_BOOL               isIngress
);


/**
* @internal snetFalconPhaProcess function
* @endinternal
*
* @brief   Falcon : do PHA processing .
*         in the Control pipe : QAG --> HA --> EPCL --> PRE-Q --> EOAM --> EPLR --> ERMRK --> PHA --> EREP --> DP(RxDma).
*         in the Data Path  : --> RxDma --> TXQ-PDX --> (egress tile) TX-SDQ --> (TXFIFO,TXDMA) --> MAC.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - <localDevTargetPort> the egress physical port.
*                                       indication that packet was send to the DMA of the port
* COMMENTS :
*/
void snetFalconPhaProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
);

/**
* @internal snetFalconPhaCalculateTargetPort function
* @endinternal
*
* @brief   Falcon : Get the target port .
*          For Egress mirrored packets, the original trg port is used for PHA target table lookup
*          For ingress mirror and non mirror packets, no change in egress port
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - <localDevTargetPort> the egress physical port.
*                                       indication that packet was send to the DMA of the port
* COMMENTS :
*/
GT_U32 snetFalconPhaCalculateTargetPort
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetFalconEgressh */



