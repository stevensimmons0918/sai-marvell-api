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
* @file prvCpssGenNetworkIfDsaTag.c
*
* @brief Implement CPSS build and parsing of DSA tag
*
*
* @version   3
********************************************************************************
*/

/*******************************************************************************
* External usage environment parameters
*******************************************************************************/
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfDsaTag.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal prvCpssNetDsaTagBuild function
* @endinternal
*
* @brief   Builds the DSA Tag .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] dsaTagType               - the type of DSA tag to build (regular/extended)
* @param[in] dsaParamsPtr             - (pointer to) dsa parameters.
*
* @param[out] dsaTagPtr                - (pointer to) the DSA tag.
*
* @retval GT_OK                    - if successful, or
* @retval GT_NOT_IMPLEMENTED       - the request is not implemented yet.
* @retval GT_BAD_PARAM             - one of the parameters is not valid
* @retval GT_BAD_PTR               - one of the pointers is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssNetDsaTagBuild
(
    IN PRV_CPSS_NET_DSA_TYPE_ENT    dsaTagType,
    IN PRV_CPSS_NET_DSA_PARAMS_STC  *dsaParamsPtr,
    OUT GT_U32                      *dsaTagPtr
)
{
    if(dsaTagType != PRV_CPSS_NET_DSA_TYPE_REGULAR_E)
    {
        /* not implemented yet */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    /* start with word 0 of the DSA tag */
    dsaTagPtr[0] = 0;

    if(dsaTagType != PRV_CPSS_NET_DSA_TYPE_REGULAR_E)
    {
        /* set in the "extended" bit -- bit 12 */
        U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],12,1,1);
    }

    /* Regular packets are subject to L2 bridging/egress-filtering */
    U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],30,2,dsaParamsPtr->dsaCmd);

    /* update fields according to packet encapsulation */
    switch(dsaParamsPtr->dsaCmd)
    {
        case PRV_CPSS_NET_DSA_CMD_FORWARD_E:
            if(dsaParamsPtr->dsaFormat.forward.srcTagged == GT_TRUE)
            {
                U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],29, 1, 1);
            }

            U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],24,5,dsaParamsPtr->dsaFormat.forward.srcDevNum);

            if(dsaParamsPtr->dsaFormat.forward.srcIsTrunk == GT_FALSE)
            {
                U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],19,5,dsaParamsPtr->dsaFormat.forward.source.srcPortNum);
            }
            else
            {
                U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],19,5,dsaParamsPtr->dsaFormat.forward.source.srcTrunkId);
                U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],18,1,1);/* is trunk bit */
            }
            break;
        case PRV_CPSS_NET_DSA_CMD_FROM_CPU_E:
            /* Control packets are NOT subject to L2 bridging/egress-filtering */
            if (dsaParamsPtr->dsaFormat.fromCpu.useVidx == GT_TRUE)
            {
                /* set use vidx bit */
                U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],18,1,1);
                /* update VIDX data*/
                if(dsaTagType == PRV_CPSS_NET_DSA_TYPE_REGULAR_E)
                {
                     /* 9 bits from bit 20 */
                    U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0], 20, 9,
                        dsaParamsPtr->dsaFormat.fromCpu.dest.vidx);
                }
                else
                {
                    /* 10 bits from bit 19 */
                    U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0], 19, 10,
                        dsaParamsPtr->dsaFormat.fromCpu.dest.vidx);
                }
            }
            else
            {
                /* update target port & dev */
                U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],24,5,
                        dsaParamsPtr->dsaFormat.fromCpu.dest.devPort.tgtDev);

                U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],19,5,
                        dsaParamsPtr->dsaFormat.fromCpu.dest.devPort.tgtPort);

                if (dsaParamsPtr->dsaFormat.fromCpu.dstTagged == GT_TRUE)
                {
                    U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],29,1,1);
                }
            }

            if(dsaTagType == PRV_CPSS_NET_DSA_TYPE_REGULAR_E)
            {
                /* NOTE: only 2 bits of TC are used */
                U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],16,2, dsaParamsPtr->dsaFormat.fromCpu.tc);
            }
            else
            {
                /* NOTE: only 1 bits of TC is used here :
                   set TC[0] in bit 17 word 0 */
                U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],17,1,dsaParamsPtr->dsaFormat.fromCpu.tc);
            }
            break;

        default:
            /* not implemented yet */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    if(dsaTagType != PRV_CPSS_NET_DSA_TYPE_REGULAR_E)
    {
        /* set CFI in bit 16 word 0 */
        U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],16,1,dsaParamsPtr->cfi);
    }

    /* update VID/VPT fields */
    U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],0,12,dsaParamsPtr->vid);
    U32_SET_FIELD_MASKED_MAC(dsaTagPtr[0],13,3,dsaParamsPtr->vpt);

    return GT_OK;
}



