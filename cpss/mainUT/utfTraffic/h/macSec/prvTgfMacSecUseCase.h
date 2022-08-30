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
* @file prvTgfMacSecUseCase.h
*
* @brief MACSec testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfMacSecUseCaseh
#define __prvTgfMacSecUseCaseh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <Kit/SABuilder_MACsec/incl/sa_builder_macsec.h>
#include <Kit/SABuilder_MACsec/Example_AES/incl/aes.h>

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }


/* Parameters that specify SA creation. */
typedef struct
{
    SABuilder_Direction_t direction;
    GT_U8 AN;
    GT_U8 *Key_p;
    GT_U32 KeyByteCount;
    GT_U8 *SCI_p;
    GT_U8 *SSCI_p;
    GT_U8 *Salt_p;
    GT_U32 SeqNumLo;
    GT_U32 SeqNumHi;
    GT_U32 SeqMask;
} da_sa_params_t;


void Log_HexDump32
(
    const GT_CHAR * szPrefix_p,
    const GT_U32 PrintOffset,
    const GT_U32 * Buffer_p,
    const GT_U32 Word32Count
);

/*
 * @enum PRV_TGF_MACSEC_CONFIG_PARAM_ENT
 *
 * @brief Config MACsec params as per test requirement.
 */
typedef enum{

    /** @brief Sequence number low(LSB 32 bits) value.
    */
    PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_LOW_E,

    /** @brief Sequence number threshold value.
    */
    PRV_TGF_MACSEC_CONFIG_PARAM_SEQ_NUM_THR_E,

    /** @brief Sequence number threshold value.
    */
    PRV_TGF_MACSEC_CONFIG_PARAM_TEST_PACKET_COUNT_E

} PRV_TGF_MACSEC_CONFIG_PARAM_ENT;


/*----------------------------------------------------------------------------
 * da_macsec_build_sa
 *
 * Create SA record using MACsec SA Builder.
 *
 * params (input)
 *     Parameters that specify MACsec SA.
 *
 * SAWordCount_p (output)
 * Return: pointer to buffer allocated by da_macsec malloc(),
 *         containing SA record. Caller must free the buffer.
 *         NULL in case of error.
 */
GT_U32 * da_macsec_build_sa
(
    const da_sa_params_t * const params,
    GT_U32 * const SAWordCount_p
);



/**
* internal prvTgfMacSecEgressConfigSet function
* @endinternal
*
* @brief   Enhanced UT MACSec Egress use case configurations
*
* @param[in] encryptionProtect - enable/disable Encryption
*/
GT_VOID prvTgfMacSecEgressConfigSet
(
    IN GT_BOOL     encryptionProtect
);

/**
* internal prvTgfMacSecEgressIngressConfigSet function
* @endinternal
*
* @brief   Enhanced UT MACSec Egress & Ingress use case configurations
*
*/
GT_VOID prvTgfMacSecEgressIngressConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfMacSecEgressTest function
* @endinternal
*
* @brief   Enhanced UT MACSec Egress use case test
*/
GT_VOID prvTgfMacSecEgressTest
(
    GT_VOID
);

/**
* @internal prvTgfMacSecEgressIngressTest function
* @endinternal
*
* @brief   Enhanced UT MACSec Egress & INgress use case test
*/
GT_VOID prvTgfMacSecEgressIngressTest
(
    GT_VOID
);

/**
* @internal prvTgfMacSecEgressVerification function
* @endinternal
*
* @brief   Enhanced UT MACSec Egress use case verification
*/
GT_VOID prvTgfMacSecEgressVerification
(
    GT_VOID
);

/**
* @internal prvTgfMacSecEgressIngress Verification function
* @endinternal
*
* @brief   Enhanced UT MACSec Egress & Ingress use case verification
*/
GT_VOID prvTgfMacSecEgressIngressVerification
(
    GT_VOID
);

/**
* @internal prvTgfMacSecEgressConfigRestore function
* @endinternal
*
* @brief   Enhanced UT MACSec configurations restore.
*/
GT_VOID prvTgfMacSecEgressConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfMacSecEgressIngressConfigRestore function
* @endinternal
*
* @brief   Enhanced UT MACSec Egress & Ingress configurations restore.
*/
GT_VOID prvTgfMacSecEgressIngressConfigRestore
(
    GT_VOID
);

/**
* @internal prvCpssDxChMacSecEgressConfigSet function
* @endinternal
*
* @brief   MACSec Egress configurations
*
* @param[in] devNum            - the device number
* @param[in] portNum           - port number
* @param[in] encryptionProtect - enable/disable Encryption
*
*  */
GT_VOID prvCpssDxChMacSecEgressConfigSet
(
    IN GT_U8       devNum,
    IN GT_PORT_NUM portNum,
    IN GT_BOOL     encryptionProtect
);

/**
* @internal prvCpssDxChMacSecEgressIngressConfigSet function
* @endinternal
*
* @brief   MACSec Egress & Ingress configurations
*
* @param[in] devNum            - the device number
* @param[in] portNum           - port number
* @param[in] encryptionProtect - enable/disable Encryption
*
*  */
GT_VOID prvCpssDxChMacSecEgressIngressConfigSet
(
    IN GT_U8       devNum,
    IN GT_PORT_NUM portNum,
    IN GT_BOOL     encryptionProtect
);



/**
* @internal prvCpssDxChMacSecEgressExit function
* @endinternal
*
* @brief   MACSec Egress & Ingress configurations
*
* @param[in] devNum - the device number
*/
GT_VOID prvCpssDxChMacSecEgressExit
(
    IN GT_U8       devNum
);


/**
* @internal prvCpssDxChMacSecEgressIngressExit function
* @endinternal
*
* @brief   MACSec Egress & Ingress configurations
*
* @param[in] devNum - the device number
*/
GT_VOID prvCpssDxChMacSecEgressIngressExit
(
    IN GT_U8       devNum
);

/**
* @internal prvTgfMacSecEventsVerification function
* @endinternal
*
* @brief   Enhanced UT MACSec events verification
*
* @param[in] testNo - test case number
*/
GT_VOID prvTgfMacSecEventsVerification
(
    IN GT_U8 testNo
);

/**
* @internal prvTgfMacSecEvents function
* @endinternal
*
* @brief   Enhanced UT MACSec events test
*/
GT_VOID prvTgfMacSecEvents
(
    GT_VOID
);

/**
 * @internal prvTgfMacSecConfigParamSet function
 * @endinternal
 *
 * @brief To set the test parameter value.
 *
 * @param[in] paramId                    - paramter id to be changed
 * @param[in] paramValue                 - parameter value to be used
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong parameters
 */
GT_STATUS prvTgfMacSecConfigParamSet
(
    IN  PRV_TGF_MACSEC_CONFIG_PARAM_ENT paramId,
    IN  GT_U32                          paramValue
);

/**
 * @internal prvTgfMacSecConfigParamGet function
 * @endinternal
 *
 * @brief To get the test parameter value.
 *
 * @param[in] paramId                    - paramter id to be changed
 * @param[in] paramValuePtr              - (pointer to) retrieve the test parameter value
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong parameters
 */
GT_STATUS prvTgfMacSecConfigParamGet
(
    IN  PRV_TGF_MACSEC_CONFIG_PARAM_ENT paramId,
    OUT GT_U32                          *paramValuePtr
);

/**
 * @internal prvTgfMacSecEventConfigSet function
 * @endinternal
 *
 * @brief Set the test configuration parameters for MACSec event use case.
 *
 * @param[in] seqNumLo                    - sequence number lower 32 bits
 * @param[in] seqNumThr                   - sequence number threshold
 * @param[in] pktCnt                      - packet count for test
 *
 */
GT_VOID prvTgfMacSecEventConfigSet
(
    IN GT_U32  seqNumLo,
    IN GT_U32  seqNumThr,
    IN GT_U32  pktCnt
);

/**
 * @internal prvTgfMacSecEventConfigStore function
 * @endinternal
 *
 * @brief Store the test configuration parameters.
 *
 */
GT_VOID prvTgfMacSecEventConfigStore
(
    GT_VOID
);

/**
 * @internal prvTgfMacSecEventConfigRestore function
 * @endinternal
 *
 * @brief Restore the test configuration parameters.
 *
 */
GT_VOID prvTgfMacSecEventConfigRestore
(
    GT_VOID
);

/**
* internal prvTgfMacSecDiEventsTest function
* @endinternal
*
* @brief   Enhanced UT MacSec DI events use case configurations
*
*/
GT_VOID prvTgfMacSecDiEventsTest
(
    GT_VOID
);

/**
* internal prvTgfMacSecDiEventsRestore function
* @endinternal
*
* @brief   Enhanced UT MacSec DI events restore configurations
*
*/
GT_VOID prvTgfMacSecDiEventsRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfMacSecEgressUseCaseh */




