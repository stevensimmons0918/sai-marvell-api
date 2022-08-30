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
* @file prvCpssDxChLpmTcamDbg.c
*
* @brief CPSS DXCH debug functions for TCAM based LPM
*
* @version   47
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamPatTrie.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamCommonTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/
#define PRV_SHARED_IP_LPM_DIR_LPM_TCAM_DBG_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.lpmTcamDbgSrc._var)

extern GT_VOID * prvCpssSlSearch
(
    IN GT_VOID        *ctrlPtr,
    IN GT_VOID        *dataPtr
);

/**
* @internal convertTokenToIndex function
* @endinternal
*
* @brief   Converts token to index of the entry. The index is relative to the first
*         line of the TCAM section managed by the TCAM manager.
*         The TCAM manager maintains array of entries information. The token is
*         implemented as a pointer to the element in the array that is
*         associated with the entry.
* @param[in] tcamManagerPtr           - the TCAM manager object
* @param[in] token                    - the token
*
* @param[out] indexPtr                 - points to the index associated with the token
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - token is part of tokens free list or is reserved token
* @retval GT_OUT_OF_RANGE          - token is pointing outside the entries info array
* @retval GT_FAIL                  - on error
*
* @note Note that token is array index + 1 as token value 0 represents no boundary
*       in allocation function.
*
*/
extern  GT_STATUS convertTokenToIndex
(
    IN  void   *tcamManagerPtr,
    IN  GT_U32                          token,
    OUT GT_U32                          *indexPtr
);

/**
* @enum PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_ENT
 *
 * @brief Scan patricia trie codes
*/
typedef enum{

    /** defines not valid token node. */
    PRV_CPSS_DXCH_LPM_TCAM_SCAN_NON_VALID_TOKEN = 0,

    /** scan trie fail code */
    PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_FAIL_E     = 0x7FFFFFF0,

    /** bad tcam index associated with trie node */
    PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_BAD_INDEX_E,

    /** trie node is not equal to trie node that token node pointed to. */
    PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_BAD_NODE_E

} PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_ENT;

/**
* @internal prvCpssDxChLpmTcamDbgDumpRouteTcam function
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
GT_STATUS prvCpssDxChLpmTcamDbgDumpRouteTcam
(
    IN GT_BOOL dump
)
{
    GT_U8   devNum =0;
    GT_U32  routerTtiTcamRow;
    GT_U32  routerTtiTcamColumn;
    GT_U32  tcamData32 = 0, tcamData16 = 0, tcamMask32 = 0, tcamMask16 = 0, lttData;
    GT_U32  retVal = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    GT_BOOL valid;
    GT_U32  wordOffset;
    GT_U32  keyArr[6];           /* TCAM key in hw format            */
    GT_U32  maskArr[6];          /* TCAM mask in hw format           */
          /* TCAM action (rule) in hw format  */
    GT_U32  actionArr[PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_TCAM_MAX_LINE_ACTION_SIZE_CNS];
    GT_U32  validArr[5];         /* TCAM line valid bits             */
    GT_U32  compareModeArr[5];   /* TCAM line compare mode           */


    if(dump == GT_FALSE)
        return GT_OK;

    /*Lock the access to per device data base in order to avoid corruption*/
    PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    for(routerTtiTcamRow = 0 ; routerTtiTcamRow < fineTuningPtr->tableSize.router ; routerTtiTcamRow ++)
    {
         /* clear data */
        cpssOsMemSet(keyArr, 0, sizeof(keyArr));
        cpssOsMemSet(maskArr, 0, sizeof(maskArr));
        cpssOsMemSet(actionArr, 0, sizeof(actionArr));
        cpssOsMemSet(validArr, 0, sizeof(validArr));
        cpssOsMemSet(compareModeArr, 0, sizeof(compareModeArr));

        /* read hw data */
        retVal = prvCpssDxChRouterTunnelTermTcamGetLine(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                        routerTtiTcamRow,&validArr[0],
                                                        &compareModeArr[0],
                                                        &keyArr[0],&maskArr[0],
                                                        &actionArr[0]);
        if (retVal != GT_OK)
        {
            /*Unlock the access to per device data base*/
            CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            return retVal;
        }

        /* line holds valid IPv6 prefix if the following applies:
            - all entries are valid
            - the compare mode for all entries is row compare
            - keyArr[5] bit 31 must be 0 (to indicate IPv6 entry and not TT entry) */
        valid = GT_TRUE;
        /* if entry is not valid or is single compare mode, whole line is not valid */
        if ((validArr[0] == 0) || (compareModeArr[0] == 0) ||
            (validArr[1] == 0) || (compareModeArr[1] == 0) ||
            (validArr[2] == 0) || (compareModeArr[2] == 0) ||
            (validArr[3] == 0) || (compareModeArr[3] == 0))
        {
            valid = GT_FALSE;
        }
        /* if whole line is valid, verify it is indeed IPv6 entry and not TTI entry */
        else
        if (((keyArr[5] >> 31) & 0x1) != 0)
        {
            valid = GT_FALSE;
        }

        if (valid)
        {
            /* Valid IPv6 entry - print it */
            cpssOsPrintf("R:%8d C:*    Key:0x%08X:0x%08X:0x%08X:0x%08X:0x%08X:0x%08X    Mask:0x%08X:0x%08X:0x%08X:0x%08X:0x%08X:0x%08X   LTT:0x%08X\n",
                            routerTtiTcamRow,
                            keyArr[0],
                            keyArr[1],
                            keyArr[2],
                            keyArr[3],
                            keyArr[4],
                            keyArr[5],
                            maskArr[0],
                            maskArr[1],
                            maskArr[2],
                            maskArr[3],
                            maskArr[4],
                            maskArr[5],
                            actionArr[0]);
        }
        else
        {

            for(routerTtiTcamColumn = 0; routerTtiTcamColumn < 4 ; routerTtiTcamColumn++)
            {
                /* entry holds valid IPv4 prefix if the followings applies:
                    - the entry is valid
                    - the compare mode of the entry is single compare mode */
                if ((validArr[routerTtiTcamColumn] == 1) && (compareModeArr[routerTtiTcamColumn] == 0))
                    valid = GT_TRUE;
                else
                    valid = GT_FALSE;
                if(valid)
                {
                    /* Valid IPv4 entry - print it */
                    switch (routerTtiTcamColumn)
                    {
                    case 0:
                    case 2:
                        wordOffset = (routerTtiTcamColumn == 0) ? 0 : 3;
                        tcamData32 = keyArr[wordOffset];
                        tcamData16 = keyArr[wordOffset + 1] & 0xffff;
                        tcamMask32 = maskArr[wordOffset];
                        tcamMask16 = maskArr[wordOffset + 1] & 0xffff;
                    break;

                    case 1:
                    case 3:
                        wordOffset = (routerTtiTcamColumn == 1) ? 1 : 4;

                        tcamData32 = ((keyArr[wordOffset] & 0xffff0000)>>16) | ((keyArr[wordOffset+1] & 0xffff)<<16);
                        tcamData16 = (keyArr[wordOffset+1] & 0xffff0000)>>16;
                        tcamMask32 = ((maskArr[wordOffset] & 0xffff0000)>>16) | ((maskArr[wordOffset+1] & 0xffff)<<16);
                        tcamMask16 = (maskArr[wordOffset+1] & 0xffff0000)>>16;
                    break;
                    default:
                        /*Unlock the access to per device data base*/
                        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                    lttData = actionArr[routerTtiTcamColumn];

                    cpssOsPrintf("R:%8d C:%8d    Key:0x%08X 0x%08X    Mask:0x%08X 0x%08X   LTT:0x%08X\n", routerTtiTcamRow,  routerTtiTcamColumn,
                                      tcamData32, tcamData16,  tcamMask32,  tcamMask16,  lttData);
                }
            }

        }
    }

    /*Unlock the access to per device data base*/
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return GT_OK;
}


static void patTriePrintRec
(
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC *rootPtr,
    GT_U8                                    *ipAddr,
    GT_U32                                   prefixLen,
    IN CPSS_IP_PROTOCOL_STACK_ENT            protocol,
    IN CPSS_UNICAST_MULTICAST_ENT            prefixType,
    GT_U32                                   spaces,
    GT_U32                                   numOfDash
)
{
    GT_U8 uIp[PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_MAX_PREFIX_LEN_IN_BYTES_CNS],copyIp[PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_MAX_PREFIX_LEN_IN_BYTES_CNS];
    GT_U32 i =0,addi = 0, size = 0, tcamIndex = 0;
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC *validFather;
    GT_STATUS rc;
    GT_U32 maxPrefixLengthBytes;
    GT_U8 ipAddr2[PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_MAX_PREFIX_LEN_IN_BYTES_CNS];
    PRV_CPSS_DXCH_LPM_TCAM_MC_GROUP_SRC_NODE_STC *mcGroupNode = NULL;

    cpssOsMemSet(ipAddr2,0,PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_MAX_PREFIX_LEN_IN_BYTES_CNS);
    cpssOsMemSet(&uIp,0,16);
    prvCpssDxChLpmTcamPatTrieCopyIpAddr(ipAddr,rootPtr->ipAddr,0,rootPtr->prefixLen,prefixLen);
    prvCpssDxChLpmTcamPatTrieCopyIpAddr(uIp,ipAddr,0,(prefixLen+rootPtr->prefixLen),0);
    {
        if (PRV_SHARED_IP_LPM_DIR_LPM_TCAM_DBG_SRC_GLOBAL_VAR_GET(printMcSrcTreeInProgress) == GT_TRUE)
        {
            for (i = 0; i< PRV_SHARED_IP_LPM_DIR_LPM_TCAM_DBG_SRC_GLOBAL_VAR_GET(mcPrintSpaceNum);i++)
            {
                cpssOsPrintf(" ");
            }
        }
        for (i = 0; i<(spaces*2);i++)
        {
            cpssOsPrintf(" ");
        }

        addi = 1;
        validFather = prvCpssDxChLpmTcamPatTrieGetValidFather(rootPtr);

        if(NULL != rootPtr->father)
        {
            if(rootPtr == rootPtr->father->leftSon)
            {
                cpssOsPrintf("%d.L", spaces);
            }
            else if(rootPtr == rootPtr->father->rightSon)
            {
                cpssOsPrintf("%d.R", spaces);
            }
            else
            {
                cpssOsPrintf(">>>");
            }
        }
        else
            cpssOsPrintf("root");
        cpssOsPrintf(" ");
        cpssOsPrintf("0x%.08X",rootPtr);

        cpssOsPrintf(" ");
        if (protocol == CPSS_IP_PROTOCOL_IPV4_E )
        {
            maxPrefixLengthBytes = 4;
        }
        else
        {
            maxPrefixLengthBytes = 16;
        }
        for (i = 0; i < maxPrefixLengthBytes; i++)
        {
            cpssOsPrintf("%.3d.",uIp[i]);
        }

        if (! ((rootPtr->father == NULL) && (rootPtr->pData == NULL) && (rootPtr->ipTcamShadowPtr == NULL)) )
        {
            rc = convertTokenToIndex( rootPtr->ipTcamShadowPtr->tcamManagerHandlerPtr,
                                      (GT_U32)(rootPtr->tcamAllocInfo.tokenNode->tcamManagerEntryToken),
                                      &tcamIndex);
            if(rc != GT_OK)
                return;
        }

        cpssOsPrintf("\\%d %.3d 0x%.08X 0x%.08X",
                     rootPtr->prefixLen+prefixLen,
                     rootPtr->size,
                     rootPtr->pData,
                     rootPtr->tcamAllocInfo.tokenNode);
        if (! ((rootPtr->father == NULL) && (rootPtr->pData == NULL) && (rootPtr->ipTcamShadowPtr == NULL)) )
        {
           if(NULL != rootPtr->tcamAllocInfo.tokenNode)
           {
               cpssOsPrintf(" %d:%d 0x%.08X\r\n",
                   rootPtr->tcamAllocInfo.tokenNode->tcamManagerEntryToken,
                   tcamIndex,
                   rootPtr->tcamAllocInfo.tokenNode->trieNode);
           }
        }
        if (validFather == NULL)
            if ( !((PRV_SHARED_IP_LPM_DIR_LPM_TCAM_DBG_SRC_GLOBAL_VAR_GET(printMcSrcTreeInProgress) == GT_TRUE) && (rootPtr->tcamAllocInfo.tokenNode != NULL)) )
            {
                 cpssOsPrintf("\r\n");
            }
    }
    cpssOsMemCpy(copyIp,ipAddr,PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_MAX_PREFIX_LEN_IN_BYTES_CNS);


    if ( (prefixType == CPSS_MULTICAST_E) && (rootPtr->pData != NULL) )
    {
        /* print source tree */
        mcGroupNode = (PRV_CPSS_DXCH_LPM_TCAM_MC_GROUP_SRC_NODE_STC*)(rootPtr->pData);
        if (mcGroupNode->ipMcSrcTrie != NULL)
        {
            PRV_SHARED_IP_LPM_DIR_LPM_TCAM_DBG_SRC_GLOBAL_VAR_GET(printMcSrcTreeInProgress) = GT_TRUE;
            PRV_SHARED_IP_LPM_DIR_LPM_TCAM_DBG_SRC_GLOBAL_VAR_GET(mcPrintSpaceNum) = spaces*2 + 5;
            cpssOsPrintf("================start src tree: %d===============\n", mcGroupNode->isAPointer2Def);
             patTriePrintRec(mcGroupNode->ipMcSrcTrie,ipAddr2,0,protocol,CPSS_UNICAST_E,0,2);
             cpssOsPrintf("===============end src tree======================\n");
             PRV_SHARED_IP_LPM_DIR_LPM_TCAM_DBG_SRC_GLOBAL_VAR_GET(printMcSrcTreeInProgress) = GT_TRUE;
             PRV_SHARED_IP_LPM_DIR_LPM_TCAM_DBG_SRC_GLOBAL_VAR_GET(mcPrintSpaceNum) = 0;
        }

    }

    if (rootPtr->rightSon != NULL)
    {
        if (rootPtr->size <= rootPtr->rightSon->size)
        {
            cpssOsPrintf("rightSize Error!\n");
        }
        patTriePrintRec(rootPtr->rightSon,copyIp,prefixLen+rootPtr->prefixLen,
                        protocol,prefixType,spaces+addi,numOfDash);
        size += rootPtr->rightSon->size;
    }

    if (rootPtr->leftSon != NULL)
    {
        if (rootPtr->size <= rootPtr->leftSon->size)
        {
            cpssOsPrintf("leftSize Error!\n");
        }
        patTriePrintRec(rootPtr->leftSon,ipAddr,prefixLen+rootPtr->prefixLen,
                        protocol,prefixType, spaces+addi,numOfDash);
        size += rootPtr->leftSon->size;
    }
    if (size+1 != rootPtr->size)
    {
        cpssOsPrintf("sizeMismatch!!!\n");
    }
}

/**
* @internal patTrieScan function
* @endinternal
*
* @brief   This func scans recursively Patricia Trie and print its contents
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] rootPtr                  - pointer to the current trie node
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
*                                       NoNe
*
* @note none
*
*/
static void patTrieScan
(
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC *rootPtr,
    IN CPSS_IP_PROTOCOL_STACK_ENT protocol,
    IN CPSS_UNICAST_MULTICAST_ENT prefixType
)
{
    GT_U8 ipAddr1[PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_MAX_PREFIX_LEN_IN_BYTES_CNS];
    cpssOsMemSet(ipAddr1,0,PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_MAX_PREFIX_LEN_IN_BYTES_CNS);
    cpssOsPrintf("location   node address     IP Prefix and length   Size      pData    tokenNode   token:index  TokenNodePtr \n");
    cpssOsPrintf("\n\n");
    patTriePrintRec(rootPtr,ipAddr1,0,protocol,prefixType,0,2);
}

/**
* @internal prvCpssDxChLpmTcamDbgPatTriePrint function
* @endinternal
*
* @brief   This function prints Patricia trie contents.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router identifier.
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if vrId is not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamDbgPatTriePrint
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC     *lpmDbPtr,
    IN GT_U32                                vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT            protocol,
    IN CPSS_UNICAST_MULTICAST_ENT            prefixType
)
{
    /* Pointer the 1st level bucket to which the prefix should be inserted.*/
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC *pRootTrie = NULL;

    PRV_CPSS_DXCH_LPM_TCAM_VR_TBL_BLOCK_STC *vrEntryPtr,vrEntryTmp;

    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV6_E:
            break;
        case CPSS_IP_PROTOCOL_IPV4_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    vrEntryTmp.vrId = vrId;

    vrEntryPtr = prvCpssSlSearch(lpmDbPtr->vrSl,&vrEntryTmp);
    if (vrEntryPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    switch (prefixType)
    {
        case CPSS_UNICAST_E:
            pRootTrie = vrEntryPtr->ipUcTrie[protocol];
            break;
        case CPSS_MULTICAST_E:
            pRootTrie = vrEntryPtr->ipMcTrie[protocol];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (pRootTrie!=NULL)
    {
        patTrieScan(pRootTrie,protocol,prefixType);
    }
    else
    {
        cpssOsPrintf("prvCpssDxChLpmTcamDbgPatTriePrint - return OK since the trie is empty, prefixType=%d with protocol=%d is not supported in vrId=%d .\n",
                     prefixType,protocol,vrId);
    }

    return GT_OK;
}

/**
* @internal patTrieValidityCheck function
* @endinternal
*
* @brief   This performs a validity checks on the given patrie node.
*         a. left son first bit 0 , right son first bit 1
*         b. length > 0 apart from root , ipv4 < 32 , ipv6 < 128
*         c. every node which is not root has to have a father
*         root node has to have a father==NULL
*         d. non valid node has to have 2 sons
*         e. size has to be sum of two sons + 1 (make sure also when no sons), also count all valid
*         nodes and compare to counters as a total checks this should be done for the whole trie
*         f. prefixFlagType need to check has only valid values
*         g. Compare Vrid in node to vrid which is checked.
*         h. for non valid node tokenNode value should be equal to one of his sons, and trie node
*         inside should point to a valid trie node and different from current node, for valid node it
*         must be different from sons
*         i. every mc valid node other than root need to have trie for src ip.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] vrId                     - the virtual router id.
* @param[in] rootPtr                  - pointer to the current trie node
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
* @param[in] isRootNode               -  if the given rootPtr is the root of the tree GT_TRUE else GT_FALSE.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if vrId is not found
*
* @note none
*
*/
static GT_STATUS patTrieValidityCheck
(
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC *rootPtr,
    IN CPSS_IP_PROTOCOL_STACK_ENT               protocol,
    IN CPSS_UNICAST_MULTICAST_ENT               prefixType,
    IN GT_BOOL                                  isRootNode
)
{
    GT_U32 sonSize;
    PRV_CPSS_DXCH_LPM_TCAM_TOKEN_NODE_STC *RSonTokenNode=NULL,
                                         *LSonTokenNode=NULL;
    PRV_CPSS_DXCH_LPM_TCAM_MC_GROUP_SRC_NODE_STC *mcGroupNode=NULL;

    /* Node checks */
    /*   a. left son first bit 0 , right son first bit 1   */
    if(rootPtr->rightSon != NULL)
    {
        if((rootPtr->rightSon->ipAddr[0] & 0x80) == 0)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        /* Will be used later */
        RSonTokenNode = rootPtr->rightSon->tcamAllocInfo.tokenNode;
    }

    if(rootPtr->leftSon != NULL)
    {
        if((rootPtr->leftSon->ipAddr[0] & 0x80) != 0)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        /* Will be used later */
        LSonTokenNode = rootPtr->leftSon->tcamAllocInfo.tokenNode;
    }

    /*   b. length > 0 apart from root , ipv4 < 32 , ipv6 < 128 */
    if(rootPtr->prefixLen > 0)
    {
        if(rootPtr->prefixLen > 128)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        else if((protocol == CPSS_IP_PROTOCOL_IPV4_E) && (rootPtr->prefixLen > 32))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    else if(isRootNode != GT_TRUE)
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

    /*   c. every node which is not root has to have a father */
    /*   root node  has to have a father==NULL */
    if(((isRootNode == GT_TRUE) && (rootPtr->father != NULL))||
       ((isRootNode == GT_FALSE) && (rootPtr->father == NULL)))
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

    /*   d. non valid node has to have 2 sons */
    if((rootPtr->pData == NULL) &&
       ((rootPtr->rightSon == NULL) || (rootPtr->leftSon == NULL)))
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

    /*   e. size has to be sum of two sons + 1 (make sure also when no sons), also count all valid
                   nodes and compare to counters as a total checks this should be done for the whole trie */
    sonSize = 0;
    if(rootPtr->rightSon != NULL)
        sonSize = rootPtr->rightSon->size;
    if(rootPtr->leftSon != NULL)
        sonSize += rootPtr->leftSon->size;
    if((GT_U32)(rootPtr->size) != sonSize + 1)
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

    /*   f.  prefixFlagType need to check has only valid values */
    if((rootPtr->prefixFlagType != PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_PREFIX_FLAG_ADDR_E) &&
       (rootPtr->prefixFlagType != PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_PREFIX_FLAG_ECMP_E))
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

    /*   g. Compare Vrid in node to vrid which is checked. */
    if(rootPtr->vrId != vrId)
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

    /*   h. for non valid node tokenNode value should be equal to one of his sons, and trie node
              inside should point  to a valid trie node and different from current node, for valid node it
              must be different from sons */
    if(rootPtr->pData == NULL)
    {
        if((rootPtr->tcamAllocInfo.tokenNode != RSonTokenNode) &&
           (rootPtr->tcamAllocInfo.tokenNode != LSonTokenNode))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    else
    {
        if((rootPtr->tcamAllocInfo.tokenNode == RSonTokenNode) ||
           (rootPtr->tcamAllocInfo.tokenNode == LSonTokenNode))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /*   i.  every mc valid node other than root need to have trie for src ip.   */
    if((prefixType == CPSS_MULTICAST_E) && (rootPtr->pData != NULL) && (rootPtr->father != NULL))
    {
        mcGroupNode = rootPtr->pData;
        if(mcGroupNode->ipMcSrcTrie == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal patTrieScanValidityRec function
* @endinternal
*
* @brief   This func scans recursively Patricia Trie checking bounding
*         son of each trie node in the tree and the trie node itself .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] vrId                     - the virtual router id.
* @param[in] rootPtr                  - pointer to the current trie node
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
* @param[in] isRootNode               -  if the given rootPtr is the root of the tree GT_TRUE else GT_FALSE.
*                                       Tcam index of rootPtr on success. Else
*
* @retval PRV_CPSS_DXCH_LPM_TCAM_SCAN_NON_VALID_TOKEN - if rootPtr has non valid token node.
* @retval PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_FAIL_E - scan trie fail for rootPtr
* @retval PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_BAD_INDEX_E - bad tcam index associated with trie node
* @retval PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_BAD_NODE_E - trie node is not equal to trie node that token node pointed to.
*
* @note none
*
*/
static GT_U32 patTrieScanValidityRec
(
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC *rootPtr,
    IN CPSS_IP_PROTOCOL_STACK_ENT               protocol,
    IN CPSS_UNICAST_MULTICAST_ENT               prefixType,
    IN GT_BOOL                                  isRootNode
)
{
   GT_U32 leftSonIndex, rightSonIndex, myIndex, srcTrieIndex;

   GT_STATUS rc;
   PRV_CPSS_DXCH_LPM_TCAM_MC_GROUP_SRC_NODE_STC *mcGroupNode = NULL;


   if (! ((rootPtr->father == NULL) && (rootPtr->pData == NULL) && (rootPtr->ipTcamShadowPtr == NULL)) )
   {
       rc = convertTokenToIndex( rootPtr->ipTcamShadowPtr->tcamManagerHandlerPtr,
                                 (GT_U32)(rootPtr->tcamAllocInfo.tokenNode->tcamManagerEntryToken),
                                 &myIndex);
       if(rc != GT_OK)
           return PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_BAD_INDEX_E;

       /* Add one so all indexes in this function
       bigger then PRV_CPSS_DXCH_LPM_TCAM_SCAN_NON_VALID_TOKEN*/
       myIndex += 1;
   }
   else
   {
       /* rootPtr is a fake node used in ip mc src trie. In this case
          check if left and right sons are exist and check them independentely.
          No comparation would be done for left and right sones of ip mc src trie root
          and myIndex is not used here. */
       myIndex = 0;
   }

   /* in case of mc group trie for each valid node the src tree validity shaoud be checked */
   if ( (prefixType == CPSS_MULTICAST_E) && (rootPtr->pData != NULL) )
   {
       /* check source tree */
       mcGroupNode = (PRV_CPSS_DXCH_LPM_TCAM_MC_GROUP_SRC_NODE_STC*)(rootPtr->pData);
       if (mcGroupNode->ipMcSrcTrie != NULL)
       {
           prefixType = CPSS_UNICAST_E;
           srcTrieIndex = patTrieScanValidityRec(vrId, mcGroupNode->ipMcSrcTrie, protocol, prefixType, GT_FALSE);
           prefixType = CPSS_MULTICAST_E;
           if(srcTrieIndex >= PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_FAIL_E)
           {
               cpssOsPrintf("scan for ipMcSrcTrie %x of group node %x was failed\n",mcGroupNode->ipMcSrcTrie,rootPtr);
               return srcTrieIndex;
           }
       }
   }
   if (rootPtr->rightSon != NULL)
   {
       rightSonIndex = patTrieScanValidityRec(vrId, rootPtr->rightSon, protocol, prefixType, GT_FALSE);
       if(rightSonIndex >= PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_FAIL_E)
           return rightSonIndex;

       rightSonIndex += 1;/* Add one so all indexes in this function   bigger then PRV_CPSS_DXCH_LPM_TCAM_SCAN_NON_VALID_TOKEN*/
   }
   else
   {
       rightSonIndex = PRV_CPSS_DXCH_LPM_TCAM_SCAN_NON_VALID_TOKEN;
   }

   if (rootPtr->leftSon != NULL)
   {
       leftSonIndex = patTrieScanValidityRec(vrId, rootPtr->leftSon, protocol, prefixType, GT_FALSE);
       if(leftSonIndex >= PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_FAIL_E)
           return leftSonIndex;

       leftSonIndex += 1; /* Add one so all indexes in this function   bigger then PRV_CPSS_DXCH_LPM_TCAM_SCAN_NON_VALID_TOKEN*/
   }
   else
   {
       leftSonIndex = PRV_CPSS_DXCH_LPM_TCAM_SCAN_NON_VALID_TOKEN;
   }

   if ((rootPtr->father == NULL) && (rootPtr->pData == NULL) && (rootPtr->ipTcamShadowPtr == NULL) )
   {
       /* if  rootPtr is a fake node used in ip mc src trie this check is finished */
       return myIndex;
   }

    /* Check bounding validity */
    if(rootPtr->pData != NULL)
    {
        if(myIndex < leftSonIndex)
        {
            cpssOsPrintf("patTrieScanValidityRec - myIndex %d leftt: %d\n", myIndex, leftSonIndex);
            return PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_BAD_INDEX_E;
        }
        if(myIndex < rightSonIndex)
        {
            cpssOsPrintf("patTrieScanValidityRec - myIndex %d right: %d\n", myIndex, rightSonIndex);
            return PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_BAD_INDEX_E;
        }
        if (rootPtr != rootPtr->tcamAllocInfo.tokenNode->trieNode )
        {
            cpssOsPrintf("patTrieScanValidityRec - trieNode = %x, tokenNode->trieNode =%x\n", rootPtr, rootPtr->tcamAllocInfo.tokenNode->trieNode);
            return PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_BAD_NODE_E;
        }
    }
    else
    {
        if( ((leftSonIndex > rightSonIndex) && (myIndex != leftSonIndex )) ||
            ((rightSonIndex > leftSonIndex) && (myIndex != rightSonIndex))    )
            return PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_FAIL_E;
    }

    /* Additional validity check on rootPtr */
    rc = patTrieValidityCheck(vrId, rootPtr, protocol, prefixType,isRootNode);
    if(rc != GT_OK)
        return PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_BAD_NODE_E;

   /* current node has good bounding son, return it's real index */
   return myIndex - 1;
}

/**
* @internal prvCpssDxChLpmTcamDbgPatTrieValidityCheck function
* @endinternal
*
* @brief   This function checks Patricia trie validity.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - the LPM DB information
* @param[in] vrId                     - The virtual router identifier.
* @param[in] protocol                 - ip protocol
* @param[in] prefixType               - uc/mc prefix type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_STATE             - on bad state in patricia trie
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamDbgPatTrieValidityCheck
(
    IN PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC     *lpmDbPtr,
    IN GT_U32                                vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT            protocol,
    IN CPSS_UNICAST_MULTICAST_ENT            prefixType
)
{
    GT_U32   scanRet;

    /* Pointer the 1st level bucket to which the prefix should be inserted.*/
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC *pRootTrie;

    PRV_CPSS_DXCH_LPM_TCAM_VR_TBL_BLOCK_STC *vrEntryPtr,vrEntryTmp;

    switch (protocol)
    {
        case CPSS_IP_PROTOCOL_IPV6_E:
            break;
        case CPSS_IP_PROTOCOL_IPV4_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    vrEntryTmp.vrId = vrId;

    vrEntryPtr = prvCpssSlSearch(lpmDbPtr->vrSl,&vrEntryTmp);
    if (vrEntryPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch (prefixType)
    {
    case CPSS_UNICAST_E:
        pRootTrie = vrEntryPtr->ipUcTrie[protocol];

        break;
    case CPSS_MULTICAST_E:
        pRootTrie = vrEntryPtr->ipMcTrie[protocol];
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (pRootTrie!=NULL)
    {
        scanRet = patTrieScanValidityRec(vrId, pRootTrie, protocol, prefixType, GT_TRUE);
        if(scanRet >= PRV_CPSS_DXCH_LPM_TCAM_SCAN_TRIE_FAIL_E)
        {
            cpssOsPrintf("prvCpssDxChLpmTcamDbgPatTrieValidityCheck - Error!\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        cpssOsPrintf("prvCpssDxChLpmTcamDbgPatTrieValidityCheck - return OK since the trie is empty, prefixType=%d with protocol=%d is not supported in vrId=%d .\n",
                     prefixType,protocol,vrId);
    }
    return GT_OK;
}


