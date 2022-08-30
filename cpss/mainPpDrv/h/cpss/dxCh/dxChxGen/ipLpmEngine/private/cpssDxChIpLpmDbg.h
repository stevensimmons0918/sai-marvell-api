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
* @file cpssDxChIpLpmDbg.h
*
* @brief IP LPM debug functions
*
* @version   12
********************************************************************************
*/

#ifndef __cpssDxChIpLpmDbgh
#define __cpssDxChIpLpmDbgh

#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal cpssDxChIpLpmDbgRouteTcamDump function
* @endinternal
*
* @brief   This func makes physical router tcam scanning and prints its contents.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] dump                     -  parameter for debugging purposes
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on failure.
*/
GT_STATUS cpssDxChIpLpmDbgRouteTcamDump
(
    IN GT_BOOL dump
);

/**
* @internal cpssDxChIpLpmDbgPatTrieValidityCheck function
* @endinternal
*
* @brief   This function checks Patricia trie validity.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDBId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_STATE             - on bad state in patricia trie
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgPatTrieValidityCheck
(
  IN GT_U32                     lpmDBId,
  IN GT_U32                     vrId,
  IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
  IN CPSS_UNICAST_MULTICAST_ENT prefixType
);

/**
* @internal cpssDxChIpLpmDbgPatTriePrint function
* @endinternal
*
* @brief   This function prints Patricia trie contents.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbId                  - The LPM DB id.
* @param[in] vrId                     - The virtual router identifier.
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if lpmDbId or vrId is not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgPatTriePrint
(
    IN GT_U32                     lpmDbId,
    IN GT_U32                     vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType
);

/**
* @internal cpssDxChIpLpmDbgHwValidation function
* @endinternal
*
* @brief   Validate the LPM in HW
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The device number
* @param[in] vrId                     - The virtual router id
* @param[in] protocol                 - The IP protocol
* @param[in] prefixType               - Unicast or multicast
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgHwValidation
(
    IN GT_U8                            devNum,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType
);

/**
* @internal cpssDxChIpLpmDbgShadowValidityCheck function
* @endinternal
*
* @brief   Validate the LPM in Shadow
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] protocol                 - The IP protocol
* @param[in] prefixType               - Unicast or multicast
* @param[in] returnOnFailure          - whether to return on first failure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgShadowValidityCheck
(
    IN GT_U32                           lpmDbId,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType,
    IN GT_BOOL                          returnOnFailure
);

/**
* @internal cpssDxChIpLpmDbgHwShadowSyncValidityCheck function
* @endinternal
*
* @brief   This function validates synchronization between the SW and HW of the LPM
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The device number
* @param[in] lpmDbId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] protocol                 - The IP protocol
* @param[in] prefixType               - Unicast or multicast
* @param[in] returnOnFailure          - whether to return on first failure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgHwShadowSyncValidityCheck
(
    IN GT_U8                            devNum,
    IN GT_U32                           lpmDbId,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType,
    IN GT_BOOL                          returnOnFailure
);

/**
* @internal cpssDxChIpLpmDbgHwShadowValidityAndSyncCheck function
* @endinternal
*
* @brief   This function Validate the LPM in Shadow, Validate the LPM in HW and
*         check synchronization between the SW and HW of the LPM
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The device number
* @param[in] lpmDbId                  - The LPM DB id
* @param[in] vrId                     - The virtual router id
* @param[in] protocol                 - The IP protocol
* @param[in] prefixType               - Unicast or multicast
* @param[in] returnOnFailure          - whether to return on first failure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgHwShadowValidityAndSyncCheck
(
    IN GT_U8                            devNum,
    IN GT_U32                           lpmDbId,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType,
    IN GT_BOOL                          returnOnFailure
);

/**
* @internal cpssDxChIpLpmDbgPrefixLengthSet function
* @endinternal
*
* @brief   set prefix length used in addManyByOctet APIs
*
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
* @param[in] prefixLength             - prefix length
* @param[in] srcPrefixLength          - src prefix length
*/
GT_U32 cpssDxChIpLpmDbgPrefixLengthSet
(
    IN CPSS_IP_PROTOCOL_STACK_ENT protocolStack,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType,
    IN GT_U32                     prefixLength,
    IN GT_U32                     srcPrefixLength
);

/**
* @internal cpssDxChIpLpmDbgDump function
* @endinternal
*
* @brief   Print hardware LPM dump
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The device number
* @param[in] vrId                     - The virtual router id
* @param[in] protocol                 - The IP protocol
* @param[in] prefixType               - Unicast or multicast
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgDump
(
    IN GT_U8                            devNum,
    IN GT_U32                           vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT       protocol,
    IN CPSS_UNICAST_MULTICAST_ENT       prefixType
);

/**
* @internal cpssDxChIpLpmDbgHwMemPrint function
* @endinternal
*
* @brief   Print DMM partition debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChIpLpmDbgHwMemPrint
(
    IN GT_U32                           lpmDbId
);

/**
* @internal cpssDxChIpLpmDbgHwOctetPerBlockPrint function
* @endinternal
*
* @brief   Print Octet per Block debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS cpssDxChIpLpmDbgHwOctetPerBlockPrint
(
    IN GT_U32                           lpmDbId
);

/**
* @internal cpssDxChIpLpmDbgHwBlockInfoPrint function
* @endinternal
*
* @brief   Print Octet per Protocol debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS cpssDxChIpLpmDbgHwBlockInfoPrint
(
    IN GT_U32                           lpmDbId
);

/**
* @internal cpssDxChIpLpmDbgHwOctetPerProtocolPrint function
* @endinternal
*
* @brief   Print Octet per Protocol debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS cpssDxChIpLpmDbgHwOctetPerProtocolPrint
(
    IN GT_U32                           lpmDbId
);

/**
* @internal cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters
(
    IN GT_U32                           lpmDbId
);

/**
* @internal cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines and Banks memory debug information for a
*         specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] lpmDbId                  - The LPM DB id
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if can't find the lpm DB
*/
GT_STATUS cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters
(
    IN GT_U32                           lpmDbId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpLpmDbgh */


