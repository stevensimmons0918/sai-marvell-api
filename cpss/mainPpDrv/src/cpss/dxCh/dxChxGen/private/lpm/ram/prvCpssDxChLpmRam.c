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
* @file prvCpssDxChLpmRam.c
*
* @brief the CPSS LPM Engine support.
*
* @version   26
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTrie.h>
#include <cpssCommon/private/prvCpssDevMemManager.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamUc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMc.h>
#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMng.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamMng.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamDeviceSpecific.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hsu/private/prvCpssDxChHsu.h>
#include <cpss/dxCh/dxChxGen/private/lpm/prvCpssDxChLpmUtils.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpm.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamDbg.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/
#define PRV_SHARED_PORT_DIR_IP_LPM_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.ipLpmSrc._var)

#ifdef CPSS_DXCH_LPM_DEBUG_MEM
GT_U32 prvCpssDxChLpmRamMemSize = 0;
#endif

/* max number of PBR lines for SIP6 */
#define PRV_CPSS_SIP6_MAX_PBR_LINES_CNS 32768

/* max number of pbr banks in hw  */
#define PRV_CPSS_SIP6_MAX_NUM_PBR_BANKS_CNS 8

/*global variables macros*/
#define PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.lpmRamSrc._var)

extern GT_VOID * prvCpssSlSearch
(
    IN GT_VOID        *ctrlPtr,
    IN GT_VOID        *dataPtr
);

void * cpssOsLpmMalloc
(
    IN GT_U32 size
)
{
#ifdef CPSS_DXCH_LPM_DEBUG_MEM
    static GT_U32 k = 0;
    prvCpssDxChLpmRamMemSize += size;
    k++;
#endif

    GT_VOID_PTR ptr = cpssOsMalloc(size);

    if(ptr)
    {
        cpssOsMemSet(ptr,0,size);
    }

    return ptr;
}

void cpssOsLpmFree
(
    IN void* const memblock
)
{
#ifdef CPSS_DXCH_LPM_DEBUG_MEM
    static GT_U32 k = 0;
    GT_U32 size1,size2,size3,size4;

    size1 = size2 = size3 = 0;
    printMemDelta(&size1,&size2,&size3);
    size4 = prvCpssDxChLpmRamMemSize;
#endif
    cpssOsFree(memblock);
#ifdef CPSS_DXCH_LPM_DEBUG_MEM
    size1 = size2 = size3 = 0;
    printMemDelta(&prvCpssDxChLpmRamMemSize,&size2,&size3);
    size4 = size4 - prvCpssDxChLpmRamMemSize;
    k++;
#endif
}

/*******************************************************************************
**                               local defines                                **
*******************************************************************************/
#if 0
#define PRV_CPSS_DXCH_LPM_RAM_HSU_DMM_FREE_LINK_LIST_END_CNS 0xfffffffd
#define PRV_CPSS_DXCH_LPM_RAM_HSU_DMM_PARTITION_END_CNS      0xfffffffe
#endif
#define PRV_CPSS_DXCH_LPM_IPV4_MC_LINK_LOCAL_CHECK_MAC(_ipGroup, _ipGroupPrefixLen, _ipSrcPrefixLen)    \
        if(!((_ipGroup.arIP[0] == 224)  &&\
             (_ipGroup.arIP[1] == 0)    &&\
             (_ipGroup.arIP[2] == 0)    &&\
             (_ipGroupPrefixLen == 24)  &&\
             (_ipSrcPrefixLen == 0)))     \
        {                                 \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Not allow 'ipv4 MC link local' (224.0.0.x) and src prefix length [0]");\
        }

#define PRV_CPSS_DXCH_LPM_IPV4_MC_LINK_LOCAL_CHECK_EXACT_MATCH_MAC(_ipGroup, _ipGroupPrefixLen)    \
        if((_ipGroup.arIP[0] == 224)  &&\
           (_ipGroup.arIP[1] == 0)    &&\
           (_ipGroup.arIP[2] == 0)    &&\
           (_ipGroupPrefixLen == 32))   \
        {                               \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Not allow 'ipv4 MC link local' (224.0.0.x) with prefix length[32] ");\
        }

#define PRV_CPSS_DXCH_LPM_IPV6_MC_LINK_LOCAL_CHECK_MAC(_ipGroup, _ipGroupPrefixLen, _ipSrcPrefixLen)    \
        if(!((_ipGroup.arIP[0] == 0xff)  &&\
             (_ipGroup.arIP[1] == 0x02)   &&\
             (_ipGroupPrefixLen == 16 )  &&\
             (_ipSrcPrefixLen == 0)))     \
        {                                 \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Not allow 'ipv6 MC link local' (ff:02:x...) and src prefix length [0]");\
        }

#define PRV_CPSS_DXCH_LPM_IPV6_MC_LINK_LOCAL_CHECK_EXACT_MATCH_MAC(_ipGroup, _ipGroupPrefixLen)    \
        if((_ipGroup.arIP[0] == 0xff)  &&\
           (_ipGroup.arIP[1] == 0x02)    &&\
           (_ipGroupPrefixLen == 128))   \
        {                               \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Not allow 'ipv6 MC link local' (ff:02:x...) and prefix length [128]");\
        }

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_TYPE_ENT
 *
 * @brief Indicates how the LPM search structures should be updated
*/
typedef enum{

    /** don't allocate memory in the HW. */
    PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E = 0,

    /** allocate memory in the HW. */
    PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_AND_ALLOC_E,

    /** overwrite and allocate memory if needed. */
    PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_OVERWRITE_E

} PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_TYPE_ENT;

#if 0
/**
* @enum IP_HSU_STAGE_ENT
 *
 * @brief A enum representing an ip coreExMxPm shadow iterartor stage
 * values:
 * IP_HSU_VR_STAGE_E    - VR router stage
 * IP_HSU_UC_PREFIX_STAGE_E - ip uc prefix stage
 * IP_HSU_UC_LPM_STAGE_E  - ip uc lpm stage
 * IP_HSU_MC_PREFIX_STAGE_E - ip mc prefix stage
 * IP_HSU_MC_LPM_STAGE_E  - ip mc lpm stage
 * IP_HSU_DMM_FREE_LIST_E  - dmm free lists stage
*/
typedef enum{

    IP_HSU_VR_STAGE_E = 0,

    IP_HSU_UC_PREFIX_STAGE_E ,

    IP_HSU_UC_LPM_STAGE_E,

    IP_HSU_MC_PREFIX_STAGE_E,

    IP_HSU_MC_LPM_STAGE_E,

    IP_HSU_DMM_FREE_LIST_E,

    IP_HSU_IP_LAST_E

} IP_HSU_STAGE_ENT;

/*
 * Typedef: struct IP_HSU_ITERATOR_STC
 *
 * Description: A struct that holds an ip coreExMxPm shadow iterartor
 *
 * Fields:
 *      magic        - iterator magic number.
 *      currMemPtr   - the current memory address we're working on.
 *      currMemSize  - the current accumulated memory size.
 *      currIter - the current stage iterator.
 *      currStage - the current stage.
 *      currProtocolStack - the current Protocol stack.
 *      currVrId - the current vrId.
 *      currShadowIdx - shadow index.
 */
typedef struct IP_HSU_ITERATOR_STCT
{
    GT_U32                          magic;
    GT_VOID                         *currMemPtr;
    GT_U32                          currMemSize;
    GT_UINTPTR                      currIter;
    IP_HSU_STAGE_ENT                currStage;
    CPSS_IP_PROTOCOL_STACK_ENT      currProtocolStack;
    GT_U32                          currVrId;
    GT_U32                          currShadowIdx;
}IP_HSU_ITERATOR_STC;

/*
 * Typedef: struct IPV4_HSU_MC_ROUTE_ENTRY_STC
 *
 * Description: A structure to hold multicast route entry info. Used for HSU
 *              Pp synchronization.
 *
 * Fields:
 *      vrId                 - The virtual router identifier.
 *      mcGroup              - MC group IP address
 *      ipSrc                - MC source IP address
 *      ipSrcPrefixlength    - ip source prefix length
 *      mcRoutePointer       - Multicast Route Entry pointer
 *      last                 - weather this is the last entry
 */
typedef struct IPV4_HSU_MC_ROUTE_ENTRY_STCT
{
    GT_U32                                      vrId;
    GT_IPADDR                                   mcGroup;
    GT_IPADDR                                   ipSrc;
    GT_U32                                      ipSrcPrefixlength;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   mcRoutePointer;
    GT_BOOL                                     last;
}IPV4_HSU_MC_ROUTE_ENTRY_STC;

/*
 * Typedef: struct IPV6_HSU_MC_ROUTE_ENTRY_STC
 *
 * Description: A structure to hold multicast route entry info. Used for HSU
 *              Pp synchronization.
 *
 * Fields:
 *      vrId              - The virtual router identifier.
 *      mcGroup           - MC group IP address
 *      ipSrc             - MC source IP address
 *      ipSrcPrefixlength - ip source prefix length
 *      mcRoutePointer    - Multicast Route Entry pointer
 *      last              - weather this is the last entry
 *      groupScope        - the group scope.
 */
typedef struct IPV6_HSU_MC_ROUTE_ENTRY_STCT
{
    GT_U32                                      vrId;
    GT_IPV6ADDR                                 mcGroup;
    GT_IPV6ADDR                                 ipSrc;
    GT_U32                                      ipSrcPrefixlength;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   mcRoutePointer;
    GT_BOOL                                     last;
    CPSS_IPV6_PREFIX_SCOPE_ENT                  groupScope;
}IPV6_HSU_MC_ROUTE_ENTRY_STC;

/*
 * Typedef: struct DMM_IP_HSU_ENTRY_STC
 *
 * Description: A structure to hold hsu DMM entry info. Used for HSU
 *              Pp synchronization.
 *
 * Fields:
 *      partitionIndex            - dmm partition index.
 *      currentFreeLinkList       - it shows for which memory size this list is intended.
 *      currentDmmBlockAddrInList - current DMM block in the list.
 *      endOfList                 - list was processed but end of list signature was not done.
 *      endOfPartition            - partition was processed but end of partition signature was
 *                                  not done
 *
 */
typedef struct DMM_IP_HSU_ENTRY_STCT
{
    GT_U32                                  partitionIndex;
    GT_U32                                  currentFreeLinkList;
    GT_DMM_BLOCK                            *currentDmmBlockAddrInList;
    GT_BOOL                                 endOfList;
    GT_BOOL                                 endOfPartition;
    GT_DMM_PARTITION                        *partitionArray[8];
}DMM_IP_HSU_ENTRY_STC;
#endif

/*******************************************************************************
**                             forward declarations                           **
*******************************************************************************/

/* Check validity of values of route entry pointer */
static GT_STATUS prvCpssDxChLpmRouteEntryPointerCheck
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT          shadowType,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  *routeEntryPointerPtr
);

/*******************************************************************************
**                               local functions                              **
*******************************************************************************/

/**
* @internal prvCpssDxChLpmRamSip6CalcBankNumberIndex function
* @endinternal
*
* @brief    This function calculates the bank number index after
*           taking into consideration the PBR offset and the holes between
*           big banks and small banks
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] shadowPtr      - points to the shadow
* @param[inout] blockIndexPtr  - (pointer to) the index of the block
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
*
*/
GT_STATUS prvCpssDxChLpmRamSip6CalcBankNumberIndex
(
    IN      PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    INOUT   GT_U32                              *blockIndexPtr
)
{
    if(*blockIndexPtr < shadowPtr->memoryOffsetStartHoleValue)
    {
        /* shift big blocks over pbrBlocks*/
        *blockIndexPtr = *blockIndexPtr - shadowPtr->memoryOffsetValue;
    }
    else
    {
         if(*blockIndexPtr >= shadowPtr->memoryOffsetEndHoleValue)
         {
             /* shift small blocks over the holes */
            *blockIndexPtr = *blockIndexPtr -
                            (shadowPtr->memoryOffsetEndHoleValue-shadowPtr->memoryOffsetStartHoleValue)/* shift over the hole */
                            - shadowPtr->memoryOffsetValue;/* shift over pbrBlock */
         }
         else
         {
             /* illegal blockIndex - fall in holes */
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
         }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamSip6CalcPbrBankNumber function
* @endinternal
*
* @brief   This function calculates pbr banks number
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] bankSize                 - size of bank in lines.
* @param[in] numberOfPbrLines         - number of pbr lines.
*
* @param[out] numberOfPbrBanksPtr     - (pointer to) number of banks intended for pbr.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
*
*/
GT_STATUS prvCpssDxChLpmRamSip6CalcPbrBankNumber
(
    IN GT_U32 bankSize,
    IN GT_U32 numberOfPbrLines,
    OUT GT_U32 *numberOfPbrBanksPtr
)
{
    GT_U32 pbrMaxBankNumber = 0;
    GT_U32 pbrBankSize = bankSize;

    if ( (numberOfPbrLines > PRV_CPSS_SIP6_MAX_NUM_PBR_BANKS_CNS*bankSize) ||
         (numberOfPbrLines > PRV_CPSS_SIP6_MAX_PBR_LINES_CNS) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong PBR entries number");
    }

    if ((numberOfPbrLines > 4*bankSize) && (numberOfPbrLines <= 8*bankSize))
    {
        /* max addressable pbr banks is 8*/
        /* each bank can address 4k*/
        pbrMaxBankNumber = 8;
        if (bankSize > 4*1024)
        {
            pbrBankSize = 4*1024;
        }
    }
    else
        if ((numberOfPbrLines > 2*bankSize) && (numberOfPbrLines <= 4*bankSize))
        {
            /* max addressable pbr banks is 4*/
            /* each bank can address 8k*/
            pbrMaxBankNumber = 4;
            if (bankSize > 8*1024)
            {
                pbrBankSize = 8*1024;
            }
        }
        else
            if ((numberOfPbrLines > 1*bankSize) && (numberOfPbrLines <= 2*bankSize))
            {
               /* max addressable pbr banks is 2*/
               /* each bank can address 16k*/
               pbrMaxBankNumber = 2;
               if (bankSize > 16*1024)
               {
                   pbrBankSize = 16*1024;
               }
            }
            else
                if (numberOfPbrLines <= 1*bankSize)
                {
                    /* max addressable pbr banks is 1*/
                    /* each bank can address 32k*/
                    pbrMaxBankNumber = 1;
                    if (bankSize > 32*1024)
                    {
                        pbrBankSize = 32*1024;
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong PBR configuration");
                }
     *numberOfPbrBanksPtr = (numberOfPbrLines +(pbrBankSize-1))/(pbrBankSize);
     if (*numberOfPbrBanksPtr > pbrMaxBankNumber)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong PBR configuration");
     }
     return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamSip6FillCfg function
* @endinternal
*
* @brief   This function retrieve next LPM DB ID from LPM DBs Skip List
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] memoryCfgPtr        - provisioned LPM RAM configuration
*
* @param[out] ramMemoryPtr       - calculated in lines memory ram configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamSip6FillCfg
(
    IN    CPSS_DXCH_LPM_RAM_CONFIG_STC      *memoryCfgPtr,
    OUT   PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC  *ramMemoryPtr
)
{
    GT_U32 numberOfPbrLines;
    GT_U32 bigBanksNumber;
    GT_U32 smallBanksNumber;
    GT_U32 smallBankSize = 640;
    GT_U32 numOfBigPbrBlocks = 0;
    GT_U32 numOfSmallPbrBlocks = 0;
    GT_U32 smallBankStartOffset = 10;
    GT_U32 i,k;
    GT_U32 bigBankSize;
    GT_BOOL sharedMemoryUsed = GT_FALSE;
    GT_STATUS rc = GT_OK;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC  *ramMemoryArray;/* a list of all devices configuration */

    CPSS_NULL_PTR_CHECK_MAC(memoryCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(ramMemoryPtr);

    cpssOsMemSet(ramMemoryPtr,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC));

    ramMemoryPtr->maxNumOfPbrEntries = memoryCfgPtr->maxNumOfPbrEntries;
    numberOfPbrLines = memoryCfgPtr->maxNumOfPbrEntries/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
    if (memoryCfgPtr->maxNumOfPbrEntries%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS != 0)
    {
        numberOfPbrLines++;
    }
    if (numberOfPbrLines > PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_PBR_SIZE_IN_LPM_LINES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "bad number of PBR");
    }

    /* allocate needed configuration arrays */
    ramMemoryArray = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC)* memoryCfgPtr->lpmRamConfigInfoNumOfElements);
    if (ramMemoryArray==NULL)
    {
        /* fail to allocate space */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "fail to allocate space for ramMemoryArray in prvCpssDxChLpmRamSip6FillCfg \n");
    }
    cpssOsMemSet(ramMemoryArray,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC)* memoryCfgPtr->lpmRamConfigInfoNumOfElements);

    for (k=0;k<memoryCfgPtr->lpmRamConfigInfoNumOfElements;k++)
    {
        switch (memoryCfgPtr->lpmRamConfigInfo[k].devType)
        {
            case CPSS_AC5P_ALL_DEVICES_CASES_MAC:
                bigBankSize = _10K;
                devFamily=CPSS_PP_FAMILY_DXCH_AC5P_E;
                sharedMemoryUsed = GT_TRUE;
                break;
            case CPSS_HARRIER_ALL_DEVICES_CASES_MAC:
                bigBankSize = _1K;
                sharedMemoryUsed = GT_FALSE;
                devFamily=CPSS_PP_FAMILY_DXCH_HARRIER_E;
                break;
            case CPSS_IRONMAN_ALL_DEVICES_CASES_MAC:
                bigBankSize = 1536;
                sharedMemoryUsed = GT_FALSE;
                devFamily=CPSS_PP_FAMILY_DXCH_IRONMAN_E;
                break;
            case CPSS_PHOENIX_ALL_DEVICES_CASES_MAC:
                bigBankSize = _1K;
                sharedMemoryUsed = GT_FALSE;
                devFamily=CPSS_PP_FAMILY_DXCH_AC5X_E;
                break;
            case CPSS_FALCON_ALL_DEVICES_CASES_MAC:
                bigBankSize = 14 * _1K;
                sharedMemoryUsed = GT_TRUE;
                devFamily=CPSS_PP_FAMILY_DXCH_FALCON_E;
                break;
            default:
                cpssOsFree(ramMemoryArray);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "bad devType in lpmRamConfigInfo[0]\n");
        }

        if (sharedMemoryUsed == GT_FALSE)
        {
            /* For Ironman */
            if(devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
            {
                ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E; /* not relevant */
                ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x0;   /* no octet try to get small banks first  */
                ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FFFF;/* 0-15 octets try to get big banks first
                                                                                 also 16(one extra level) */
                ramMemoryArray[k].bigBanksNumber = 9;
                ramMemoryArray[k].bigBankSize = bigBankSize;/* _1.5K */
                ramMemoryArray[k].numOfBlocks = 9;
            }
            else
            {
               /* no shared memory , treated as 'MAX_L3' */
                ramMemoryArray[k].bigBanksNumber = 28;
                ramMemoryArray[k].bigBankSize = bigBankSize;
                ramMemoryArray[k].numOfBlocks = 30;

                ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E; /* not relevant */
                ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x2;   /* 1 octet try to get small banks first  */
                ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FFFD;/* 0, 2-15 octets try to get big banks first also 16(one extra level) */
            }
        }
        else
        {
            if (CPSS_AC5P_98DX4504_DEVICES_CHECK_MAC(memoryCfgPtr->lpmRamConfigInfo[k].devType))
            {
                ramMemoryArray[k].bigBanksNumber = 8;
                ramMemoryArray[k].bigBankSize = bigBankSize/2;
                ramMemoryArray[k].numOfBlocks = 28;
                ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E; /* not relevant */
                ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x2;   /* 1 octet try to get small banks first  */
                ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FFFD;/* 0, 2-15 octets try to get big banks first also 16(one extra level) */
            }
            else
            {
                switch (memoryCfgPtr->lpmRamConfigInfo[k].sharedMemCnfg)
                {
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E:
                    ramMemoryArray[k].bigBanksNumber = 28;
                    ramMemoryArray[k].bigBankSize = bigBankSize;
                    ramMemoryArray[k].numOfBlocks = 30;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E;

                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x2;   /* 1 octet try to get small banks first  */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FFFD;/* 0, 2-15 octets try to get big banks first
                                                                                     also 16(one extra level) */

                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E:
                    ramMemoryArray[k].bigBanksNumber = 0;
                    ramMemoryArray[k].bigBankSize = 0;
                    ramMemoryArray[k].numOfBlocks = 20;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E;

                    if (numberOfPbrLines > PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_PBR_SIZE_MIN_LPM_MODE_IN_LPM_LINES_CNS)
                    {
                        cpssOsFree(ramMemoryArray);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "bad number of PBR");
                    }
                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FFFF;/* all octets get small banks */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0;     /* no big banks */
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:
                    ramMemoryArray[k].bigBanksNumber = 24;
                    ramMemoryArray[k].bigBankSize = bigBankSize / 2;
                    ramMemoryArray[k].numOfBlocks = 30;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;

                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FE02;/* 1, 9-15 octets try to get small banks first
                                                                                     also 16(one extra level) */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FD;  /* 0, 2-8  octets try to get big banks first */

                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:
                    ramMemoryArray[k].bigBanksNumber = 24;
                    ramMemoryArray[k].bigBankSize = bigBankSize / 2;
                    ramMemoryArray[k].numOfBlocks = 30;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E;

                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FE02;/* 1, 9-15 octets try to get small banks first
                                                                                    also 16(one extra level) */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FD;  /* 0, 2-8  octets try to get big banks first */

                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E:
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_NO_EM_E:
                    ramMemoryArray[k].bigBanksNumber = 16;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;

                    if(memoryCfgPtr->lpmRamConfigInfo[k].sharedMemCnfg == CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E)
                    {
                        ramMemoryArray[k].bigBankSize = bigBankSize/2;
                        ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E;
                    }
                    else
                    {
                        ramMemoryArray[k].bigBankSize = bigBankSize;
                        ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_NO_EM_E;

                        if(CPSS_PP_FAMILY_DXCH_AC5P_E == devFamily)
                        {
                            ramMemoryArray[k].bigBanksNumber = 20;
                        }
                    }

                    ramMemoryArray[k].numOfBlocks = 30;
                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FF82;/* 1, 7-15 octets try to get small banks first
                                                                                    also 16(one extra level) */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x7D;  /* 0, 2-6  octets try to get big banks first */
                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_L2_MID_EM_E:
                    ramMemoryArray[k].bigBanksNumber = 16;
                    ramMemoryArray[k].bigBankSize = bigBankSize/2;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[0].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_L2_MID_EM_E;

                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FF82;/* 1, 7-15 octets try to get small banks first
                                                                                    also 16(one extra level) */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x7D;  /* 0, 2-6  octets try to get big banks first */

                    ramMemoryArray[k].numOfBlocks = 30;
                    break;


                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_LOW_L2_MID_EM_MAX_ARP_E:
                    ramMemoryArray[k].bigBanksNumber = 24;
                    ramMemoryArray[k].bigBankSize = bigBankSize/2;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_LOW_L2_MID_EM_MAX_ARP_E;

                    ramMemoryArray[k].numOfBlocks = 30;

                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FF82;/* 1, 7-15 octets try to get small banks first
                                                                                    also 16(one extra level) */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x7D;  /* 0, 2-6  octets try to get big banks first */
                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MID_L2_MAX_EM_E:
                    ramMemoryArray[k].bigBanksNumber = 0;
                    ramMemoryArray[k].bigBankSize = 0;
                    ramMemoryArray[k].numOfBlocks = 20;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MID_L2_MAX_EM_E;

                    if (numberOfPbrLines > PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_PBR_SIZE_MIN_LPM_MODE_IN_LPM_LINES_CNS)
                    {
                        cpssOsFree(ramMemoryArray);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "bad number of PBR");
                    }
                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FFFF;/* all octets get small banks */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0;     /* no big banks */
                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_LOW_MAX_L3_MID_LOW_L2_NO_EM_E:
                    ramMemoryArray[k].bigBanksNumber = 24;
                    ramMemoryArray[k].bigBankSize = bigBankSize;
                    ramMemoryArray[k].numOfBlocks = 30;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_LOW_MAX_L3_MID_LOW_L2_NO_EM_E;

                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FE02;/* 1, 9-15 octets try to get small banks first
                                                                                    also 16(one extra level) */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FD;  /* 0, 2-8  octets try to get big banks first */
                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_HIGH_L3_MID_LOW_L2_NO_EM_MAX_ARP_E:
                    ramMemoryArray[k].bigBanksNumber = 20;
                    ramMemoryArray[k].bigBankSize = bigBankSize;
                    ramMemoryArray[k].numOfBlocks = 30;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_HIGH_L3_MID_LOW_L2_NO_EM_MAX_ARP_E;

                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FE02;/* 1, 9-15 octets try to get small banks first
                                                                                    also 16(one extra level) */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FD;  /* 0, 2-8  octets try to get big banks first */
                    break;

                default:
                     cpssOsFree(ramMemoryArray);
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "bad share table mode");
                }
            }
        }
    }

    /* go over the array of configuration - and chose the minimal configuration
       that will fit all devices in the lpmRamConfigInfo list */
    for (k=0;k<memoryCfgPtr->lpmRamConfigInfoNumOfElements;k++)
    {
        if (k==0)
        {
            /* set first optional configuration */
            ramMemoryPtr->numOfBlocks = ramMemoryArray[k].numOfBlocks;
            ramMemoryPtr->bigBanksNumber = ramMemoryArray[k].bigBanksNumber;
            ramMemoryPtr->bigBankSize = ramMemoryArray[k].bigBankSize;
            ramMemoryPtr->octetsGettingSmallBanksPriorityBitMap = ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap;
            ramMemoryPtr->octetsGettingBigBanksPriorityBitMap = ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap;
        }
        else
        {
            if (ramMemoryArray[k].numOfBlocks < ramMemoryPtr->numOfBlocks)
            {
                ramMemoryPtr->numOfBlocks = ramMemoryArray[k].numOfBlocks;
            }
            if (ramMemoryArray[k].bigBanksNumber < ramMemoryPtr->bigBanksNumber)
            {
                ramMemoryPtr->bigBanksNumber = ramMemoryArray[k].bigBanksNumber;
                ramMemoryPtr->octetsGettingSmallBanksPriorityBitMap = ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap;
                ramMemoryPtr->octetsGettingBigBanksPriorityBitMap = ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap;
            }
            if (ramMemoryArray[k].bigBankSize < ramMemoryPtr->bigBankSize)
            {
                ramMemoryPtr->bigBankSize = ramMemoryArray[k].bigBankSize;
            }
        }
    }

    smallBanksNumber = ramMemoryPtr->numOfBlocks - ramMemoryPtr->bigBanksNumber;
    bigBanksNumber = ramMemoryPtr->bigBanksNumber;
    /* small bank always start from bank10 and above */
    if(bigBanksNumber > smallBankStartOffset)
        smallBankStartOffset = bigBanksNumber;

    if (numberOfPbrLines > 0)
    {
        if (numberOfPbrLines <= smallBankSize)
        {
            if (smallBanksNumber > 0)
            {
                /* take 1 bank from the end : it is only for PBR*/
                smallBanksNumber--;
                ramMemoryPtr->numOfBlocks--;
                numOfSmallPbrBlocks++;
            }
            else
            {
                /* num of small banks is 0*/
                ramMemoryPtr->numOfBlocks--;
                ramMemoryPtr->bigBanksNumber--;
                numOfBigPbrBlocks++;
            }
        }
        else
        {
            if (bigBanksNumber > 0)
            {
                /* The max pbr value in lines is 32K. There is a differnce in max PBR lines value
                   for shared memory configuration modes. It is limited by pbr bank size register*/
                /* pbr banks size reg config must be done correspondingly for each device */
                /* In this case pbrs will be allocated in big banks */
                rc = prvCpssDxChLpmRamSip6CalcPbrBankNumber(ramMemoryPtr->bigBankSize,numberOfPbrLines,&numOfBigPbrBlocks);
                if (rc != GT_OK)
                {
                    cpssOsFree(ramMemoryArray);
                    return rc;
                }
                ramMemoryPtr->bigBanksNumber = ramMemoryPtr->bigBanksNumber - numOfBigPbrBlocks;
                ramMemoryPtr->numOfBlocks = ramMemoryPtr->numOfBlocks - numOfBigPbrBlocks;
            }
            else
            {
                /* we have only small banks */
                /* calculate number of small banks needed for PBR */
                rc = prvCpssDxChLpmRamSip6CalcPbrBankNumber(smallBankSize,numberOfPbrLines,&numOfSmallPbrBlocks);
                if (rc != GT_OK)
                {
                    cpssOsFree(ramMemoryArray);
                    return rc;
                }
                smallBanksNumber = smallBanksNumber - numOfSmallPbrBlocks;
                ramMemoryPtr->numOfBlocks = ramMemoryPtr->numOfBlocks - numOfSmallPbrBlocks;
                /* in case of min lpm configuration we need max 8 small banks : pbr bank size 4K*/
            }
        }
    }
    ramMemoryPtr->numberOfBigPbrBanks = numOfBigPbrBlocks;

    /* fill big banks data: banks used for PBR filled with 0 size */
    for (i = numOfBigPbrBlocks; i < bigBanksNumber; i++)
    {
        ramMemoryPtr->blocksSizeArray[i] = ramMemoryPtr->bigBankSize;
    }
    /* if pbr resided in small block, it would be last */
    /* handle small blocks */
    for (i = 0; i < smallBanksNumber; i++)
    {
        if ((i+smallBankStartOffset) >= PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS)
        {
            cpssOsFree(ramMemoryArray);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Souldn't happen: Exceeded bankSize array");
        }
        ramMemoryPtr->blocksSizeArray[i + smallBankStartOffset] = smallBankSize;
    }

    cpssOsFree(ramMemoryArray);
    return GT_OK;
}




/**
* @internal deleteShadowStruct function
* @endinternal
*
* @brief   This function deletes existing shadow struct.
*
* @param[in] shadowPtr                - points to the shadow to delete
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS deleteShadowStruct
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr
)
{
    GT_STATUS       rc;
    GT_U32          i,octet;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol;
    GT_U32          numOfOctetsPerProtocol[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS] = {PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS,
                                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS,
                                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS};
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC startOfOctetList;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *tempNextMemInfoPtr;

    /* first make sure all virtual routers are deleted */
    for (i = 0 ; i < shadowPtr->vrfTblSize ; i++)
    {
        if (shadowPtr->vrRootBucketArray[i].valid == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    /* now free resources used by the shadow struct */

    /* free memory set to vr table array */
    cpssOsLpmFree(shadowPtr->vrRootBucketArray);

    /* free all memory pools allocated */
    for (i = 0 ; i < shadowPtr->numOfLpmMemories ; i++)
    {
        if(shadowPtr->lpmRamStructsMemPoolPtr[i]!=0)
        {
            rc = prvCpssDmmClosePartition(shadowPtr->lpmRamStructsMemPoolPtr[i]);
            if (rc != GT_OK)
            {
                 /* free alloctated arrays used for RAM configuration */
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);

                return  rc;
            }
        }
    }

    /* free alloctated arrays used for RAM configuration */
    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);

    /* go over all linked list per protocol for all octets and free it */
    for (protocol = 0; protocol < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; protocol++)
    {
        for (octet = 0; octet < numOfOctetsPerProtocol[protocol]; octet++)
        {
            startOfOctetList = shadowPtr->lpmMemInfoArray[protocol][octet];
            while(startOfOctetList.nextMemInfoPtr != NULL)
            {
                /* keep a temp pointer to the element in the list we need to free */
                tempNextMemInfoPtr = startOfOctetList.nextMemInfoPtr;
                /* assign a new next element for the startOfOctetList */
                startOfOctetList.nextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                /* free the memory kept in the temp pointer */
                cpssOsFree(tempNextMemInfoPtr);
            }
        }
    }

    /* free memory allocated to swap */
    if (shadowPtr->swapMemoryAddr != 0)
    {
        cpssOsLpmFree((void*)shadowPtr->swapMemoryAddr);
    }

    /* free memory allocated to second swap */
    if (shadowPtr->secondSwapMemoryAddr != 0)
    {
        cpssOsLpmFree((void*)shadowPtr->secondSwapMemoryAddr);
    }

    /* remove add devices from device list */
    if (shadowPtr->shadowDevList.shareDevs != NULL)
    {
        cpssOsLpmFree(shadowPtr->shadowDevList.shareDevs);
    }

    return GT_OK;
}

/**
* @internal deleteSip6ShadowStruct function
* @endinternal
*
* @brief   This function deletes existing shadow struct.
*
* @param[in] shadowPtr                - points to the shadow to delete
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS deleteSip6ShadowStruct
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr
)
{
    GT_STATUS       rc;
    GT_U32          i,octet;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol;
    GT_U32          numOfOctetsPerProtocol[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS] = {PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS,
                                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS,
                                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS};
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC startOfOctetList;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *tempNextMemInfoPtr;

    /* first make sure all virtual routers are deleted */
    for (i = 0 ; i < shadowPtr->vrfTblSize ; i++)
    {
        if (shadowPtr->vrRootBucketArray[i].valid == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    /* now free resources used by the shadow struct */

    /* free memory set to vr table array */
    cpssOsLpmFree(shadowPtr->vrRootBucketArray);

    /* free all memory pools allocated */
    for (i = 0 ; i < shadowPtr->numOfLpmMemories ; i++)
    {
        if(shadowPtr->lpmRamStructsMemPoolPtr[i]!=0)
        {
            rc = prvCpssDmmClosePartition(shadowPtr->lpmRamStructsMemPoolPtr[i]);
            if (rc != GT_OK)
            {
                 /* free alloctated arrays used for RAM configuration */
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);

                return  rc;
            }
        }
    }

    /* free alloctated arrays used for RAM configuration */
    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);

    /* go over all linked list per protocol for all octets and free it */
    for (protocol = 0; protocol < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; protocol++)
    {
        for (octet = 0; octet < numOfOctetsPerProtocol[protocol]; octet++)
        {
            startOfOctetList = shadowPtr->lpmMemInfoArray[protocol][octet];
            while(startOfOctetList.nextMemInfoPtr != NULL)
            {
                /* keep a temp pointer to the element in the list we need to free */
                tempNextMemInfoPtr = startOfOctetList.nextMemInfoPtr;
                /* assign a new next element for the startOfOctetList */
                startOfOctetList.nextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                /* free the memory kept in the temp pointer */
                cpssOsFree(tempNextMemInfoPtr);
            }
        }
    }

    /* free memory allocated to swap */
    if (shadowPtr->swapMemoryAddr != 0)
    {
        cpssOsLpmFree((void*)shadowPtr->swapMemoryAddr);
    }

    /* free memory allocated to second swap */
    if (shadowPtr->secondSwapMemoryAddr != 0)
    {
        cpssOsLpmFree((void*)shadowPtr->secondSwapMemoryAddr);
    }

    /* free memory allocated to third swap */
    if (shadowPtr->thirdSwapMemoryAddr != 0)
    {
        cpssOsLpmFree((void*)shadowPtr->thirdSwapMemoryAddr);
    }

    /* remove add devices from device list */
    if (shadowPtr->shadowDevList.shareDevs != NULL)
    {
        cpssOsLpmFree(shadowPtr->shadowDevList.shareDevs);
    }

    return GT_OK;
}


/**
* @internal createSip6ShadowStruct function
* @endinternal
*
* @brief   This function returns a pointer to a new shadow struct.
*
* @param[in] shadowType               - the type of shadow to maintain
* @param[in] protocolBitmap           - the protocols this shodow supports
* @param[in] memoryCfgPtr             - (pointer to) the memory configuration of this LPM DB
*
* @param[out] shadowPtr                - points to the created shadow struct
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS createSip6ShadowStruct
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT   shadowType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_BMP      protocolBitmap,
    IN  PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC    *memoryCfgPtr,
    OUT PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr
)
{
    GT_U8 numOfDevs;        /* Number of devices in system.                                     */
    GT_STATUS retVal = GT_OK;
    GT_U32 memSize;         /* size of the memory block to be inserted to                       */
                            /* the memory management unit.                                      */
    GT_U32 memSizeBetweenBlocksInBytes; /* the size between each memory block management unit   */
    GT_DMM_BLOCK  *dmmBlockPtr;
    GT_DMM_BLOCK  *secondDmmBlockPtr;
    GT_DMM_BLOCK  *thirdDmmBlockPtr;

    GT_U32      i,j,k = 0;
    GT_U32      maxAllocSizeInLpmLines;
    GT_U32      sizeLpmEntryInBytes;
    GT_U32      minAllocationSizeInBytes;
    /* when regular node is going to be converted to compressed all 6 GONs must be reunited*/
    /* to one big one. maxGonSizeUnderDeleteForSwap is the maximal possible allocation place for it*/
    GT_U32      maxGonSizeUnderDeleteForSwap;
    GT_BOOL     isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E];
    GT_U32      smallBankFirstPossiblePosition = 10;
    GT_U32      offsetCompensation = 0; /* number of banks not used by the lpm */
    GT_U32      holeSize = 0; /* number of empty banks - not used = holes */
    GT_U32      swapFirstBlockOffset;
    /* number of memories in falcon is not constant, must be taken from configuration. */
    GT_UINTPTR  structsMemPool[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    GT_U32      numOfOctetsPerProtocol[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E] = {PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS,
                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS,
                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS};
    GT_U32      firstSmallBankIndex=0;
    GT_BOOL     firstSmallBankIndexFound=GT_FALSE;
    GT_U32      firstBigBankIndex=0;
    GT_BOOL     firstBigBankIndexFound=GT_FALSE;

    GT_U32      firstBlockIndex=0;
#ifdef GM_USED
    GT_U32 gmUsed = 1;
#else
    GT_U32 gmUsed = 0;
#endif

    /* zero out the shadow */
    cpssOsMemSet(shadowPtr,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC));

    numOfDevs = PRV_CPSS_MAX_PP_DEVICES_CNS;
    maxAllocSizeInLpmLines = NUMBER_OF_RANGES_IN_SUBNODE*PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS;
    sizeLpmEntryInBytes = PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS;
    minAllocationSizeInBytes = DMM_MIN_ALLOCATE_SIZE_IN_BYTE_FALCON_CNS;
    maxGonSizeUnderDeleteForSwap = maxAllocSizeInLpmLines;

    shadowPtr->shadowType = shadowType;
    shadowPtr->lpmRamTotalBlocksSizeIncludingGap = PRV_CPSS_DXCH_LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_FALCON_CNS;

    for (i=0;i<memoryCfgPtr->lpmRamConfigInfoNumOfElements;i++)
    {
        shadowPtr->lpmRamConfigInfo[i].devType = memoryCfgPtr->lpmRamConfigInfo[i].devType;
        shadowPtr->lpmRamConfigInfo[i].sharedMemCnfg = memoryCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg;
    }

    shadowPtr->lpmRamConfigInfoNumOfElements = memoryCfgPtr->lpmRamConfigInfoNumOfElements;

    shadowPtr->maxNumOfPbrEntries = memoryCfgPtr->maxNumOfPbrEntries;
    /* update the protocols that the shadow needs to support */
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap);
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap);
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap);

    /* reset the to be freed memory list */
    shadowPtr->freeMemListDuringUpdate = NULL;
    shadowPtr->freeMemListEndOfUpdate = NULL;

    /* we keep the numOfBlocks value as the numOfLpmMemories.
       Block and Memory has the same meaning.
       We use numOfLpmMemories due to legacy code */
    shadowPtr->numOfLpmMemories = memoryCfgPtr->numOfBlocks;
    shadowPtr->bigBanksNumber = memoryCfgPtr->bigBanksNumber;
    shadowPtr->bigBankSize = memoryCfgPtr->bigBankSize;
    shadowPtr->smallBanksNumber = memoryCfgPtr->numOfBlocks - memoryCfgPtr->bigBanksNumber;

    /* allocate needed RAM configuration arrays */
    shadowPtr->lpmRamBlocksSizeArrayPtr = cpssOsMalloc(sizeof(GT_U32)*shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamBlocksSizeArrayPtr == NULL)
    {
        /* fail to allocate space */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    shadowPtr->lpmRamStructsMemPoolPtr = cpssOsMalloc(sizeof(GT_UINTPTR) * shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamStructsMemPoolPtr == NULL)
    {
        /* fail to allocate space */
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    shadowPtr->lpmRamOctetsToBlockMappingPtr = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamOctetsToBlockMappingPtr == NULL)
    {
        /* fail to allocate space */
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    /* reset needed RAM configuration arrays */
    cpssOsMemSet(shadowPtr->lpmRamBlocksSizeArrayPtr,0,sizeof(GT_U32)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->lpmRamStructsMemPoolPtr,0,sizeof(GT_UINTPTR)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->lpmRamOctetsToBlockMappingPtr,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr,0,sizeof(shadowPtr->globalMemoryBlockTakenArr));

    shadowPtr->lpmMemMode = memoryCfgPtr->lpmMemMode;

    shadowPtr->lpmRamBlocksAllocationMethod = memoryCfgPtr->blocksAllocationMethod;
    if ( ((memoryCfgPtr->bigBanksNumber + memoryCfgPtr->numberOfBigPbrBanks) < smallBankFirstPossiblePosition) &&
         (memoryCfgPtr->numOfBlocks > smallBankFirstPossiblePosition))
    {
        /* this parameters are needed for bankIndex calculation in case of small banks */
        shadowPtr->memoryOffsetStartHoleValue = memoryCfgPtr->bigBanksNumber + memoryCfgPtr->numberOfBigPbrBanks;
        shadowPtr->memoryOffsetEndHoleValue   = smallBankFirstPossiblePosition;

        holeSize = shadowPtr->memoryOffsetEndHoleValue - shadowPtr->memoryOffsetStartHoleValue;

        if (((memoryCfgPtr->bigBanksNumber + memoryCfgPtr->numberOfBigPbrBanks)==0)||/* no big blocks - only small blocks */
            (memoryCfgPtr->bigBanksNumber==0))/* no free big blocks */
        {
            swapFirstBlockOffset=smallBankFirstPossiblePosition;/* block10 */
        }
        else
        {
            if (memoryCfgPtr->numberOfBigPbrBanks==0)
            {
                swapFirstBlockOffset = 0; /* block0 */
            }
            else
            {
                swapFirstBlockOffset = memoryCfgPtr->numberOfBigPbrBanks;
            }
        }
    }
    else
    {
        /* no holes */
        shadowPtr->memoryOffsetStartHoleValue=0;
        shadowPtr->memoryOffsetEndHoleValue=0;

        swapFirstBlockOffset = memoryCfgPtr->numberOfBigPbrBanks;/* swap will be located in the first free block after the pbr blocks */
    }
    offsetCompensation = holeSize + memoryCfgPtr->numberOfBigPbrBanks;/* this parameter is needed for going over all the memory blocks */
    shadowPtr->memoryOffsetValue = memoryCfgPtr->numberOfBigPbrBanks;/* this parameter is needed for bankIndex calculation in case of big banks */

    memoryCfgPtr->smallBanksIndexesBitMap = 0;
    memoryCfgPtr->bigBanksIndexesBitMap   = 0;

    /* init the search memory pools */
    for (i = 0; i < shadowPtr->numOfLpmMemories + offsetCompensation; i++)
    {
        if (memoryCfgPtr->blocksSizeArray[i] == 0)
        {
            /* it is used for PBR or we have hole when number of big banks less than 10 */
            continue;
        }
        shadowPtr->lpmRamBlocksSizeArrayPtr[k] = memoryCfgPtr->blocksSizeArray[i];

        if (memoryCfgPtr->blocksSizeArray[i]==memoryCfgPtr->bigBankSize)
        {
            /* update big banks locations */
            memoryCfgPtr->bigBanksIndexesBitMap |= 1<<k;
        }
        else
        {
            /* update small banks locations */
            memoryCfgPtr->smallBanksIndexesBitMap |= 1<<k;
        }


        /* Swap area is in the first block. In order to leave space for it, we
           deduct the size of a max LPM bucket */
        if (k == 0)
        {
             /* in the first block there must be space for 3 swap areas in case the first block is a big block,
                otherwise we will set 3 swap areas in 3 different small banks */
            if (shadowPtr->lpmRamBlocksSizeArrayPtr[0] < (3*maxGonSizeUnderDeleteForSwap))
            {
                /* one swap area should fit into a small bank */
                if (shadowPtr->lpmRamBlocksSizeArrayPtr[0] < maxGonSizeUnderDeleteForSwap)
                {
                    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                }
                else
                {
                    memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - maxGonSizeUnderDeleteForSwap;

                    /* record the swap memory address (at the end of the structs memory)*/
                    dmmBlockPtr = (GT_DMM_BLOCK*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK));
                    if (dmmBlockPtr == NULL)
                    {
                        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }

                    /* memSize is in lines, each line hold 4 words */
                    SET_OFFSET_IN_WORDS(dmmBlockPtr, (swapFirstBlockOffset*shadowPtr->lpmRamTotalBlocksSizeIncludingGap + memSize)*4);

                    /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
            #if __WORDSIZE == 64
                    dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFFFFFFFFFE;
            #else
                    dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFE;
            #endif
                    /* set first swap area */
                    shadowPtr->swapMemoryAddr = (GT_UINTPTR)dmmBlockPtr;
                }
            }
            else
            {
                memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - maxGonSizeUnderDeleteForSwap;

                /* record the swap memory address (at the end of the structs memory)*/
                dmmBlockPtr = (GT_DMM_BLOCK*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK));
                if (dmmBlockPtr == NULL)
                {
                    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }

                /* memSize is in lines, each line hold 4 words */
                SET_OFFSET_IN_WORDS(dmmBlockPtr, (swapFirstBlockOffset*shadowPtr->lpmRamTotalBlocksSizeIncludingGap + memSize)*4);


                /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
        #if __WORDSIZE == 64
                dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFFFFFFFFFE;
        #else
                dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFE;
        #endif
                /* set first swap area */
                shadowPtr->swapMemoryAddr = (GT_UINTPTR)dmmBlockPtr;

                memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - (2*maxGonSizeUnderDeleteForSwap);
                /* record the swap memory address (at the end of the structs memory)*/
                secondDmmBlockPtr = (GT_DMM_BLOCK*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK));
                if (secondDmmBlockPtr == NULL)
                {
                    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                    cpssOsLpmFree(dmmBlockPtr);/* free first swap area allocated */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }

                /* memSize is in lines, each line hold 4 words */
                SET_OFFSET_IN_WORDS(secondDmmBlockPtr, (swapFirstBlockOffset*shadowPtr->lpmRamTotalBlocksSizeIncludingGap + memSize)*4);


                /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
        #if __WORDSIZE == 64
                secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFFFFFFFFFE;
        #else
                secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFE;
        #endif

                /* set second swap area */
                shadowPtr->secondSwapMemoryAddr = (GT_UINTPTR)secondDmmBlockPtr;

                memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - (3*maxGonSizeUnderDeleteForSwap);
                /* record the swap memory address (at the end of the structs memory)*/
                thirdDmmBlockPtr = (GT_DMM_BLOCK*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK));
                if (thirdDmmBlockPtr == NULL)
                {
                    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                    cpssOsLpmFree(dmmBlockPtr);/* free first swap area allocated */
                    cpssOsLpmFree(secondDmmBlockPtr);/* free second swap area allocated */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }

                /* memSize is in lines, each line hold 4 words */
                SET_OFFSET_IN_WORDS(thirdDmmBlockPtr, (swapFirstBlockOffset*shadowPtr->lpmRamTotalBlocksSizeIncludingGap + memSize)*4);

                /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
        #if __WORDSIZE == 64
                thirdDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFFFFFFFFFE;
        #else
                thirdDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFE;
        #endif

                /* set third swap area */
                shadowPtr->thirdSwapMemoryAddr = (GT_UINTPTR)thirdDmmBlockPtr;
            }
        }
        else
        {   /* 3 swap areas need to be defined in 3 small banks */
            if ((k==1||k==2) && (shadowPtr->bigBanksNumber==0))
            {
                /* one swap area should fit into a small bank */
                if (shadowPtr->lpmRamBlocksSizeArrayPtr[k] < maxGonSizeUnderDeleteForSwap)
                {
                    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                }

                if (k==1)
                {
                    memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[k] - maxGonSizeUnderDeleteForSwap;
                    /* record the swap memory address (at the end of the structs memory)*/
                    secondDmmBlockPtr = (GT_DMM_BLOCK*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK));
                    if (secondDmmBlockPtr == NULL)
                    {
                        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                        cpssOsLpmFree((GT_DMM_BLOCK  *)shadowPtr->swapMemoryAddr);/* free first swap area allocated */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }

                    /* memSize is in lines, each line hold 4 words */
                    SET_OFFSET_IN_WORDS(secondDmmBlockPtr, 4*((swapFirstBlockOffset+1)*shadowPtr->lpmRamTotalBlocksSizeIncludingGap + memSize));

                    /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
            #if __WORDSIZE == 64
                    secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFFFFFFFFFE;
            #else
                    secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFE;
            #endif

                    /* set second swap area */
                    shadowPtr->secondSwapMemoryAddr = (GT_UINTPTR)secondDmmBlockPtr;
                }
                else/* k==2 */
                {
                    memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[k] - maxGonSizeUnderDeleteForSwap;
                    /* record the swap memory address (at the end of the structs memory)*/
                    thirdDmmBlockPtr = (GT_DMM_BLOCK*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK));
                    if (thirdDmmBlockPtr == NULL)
                    {
                        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                        cpssOsLpmFree((GT_DMM_BLOCK  *)shadowPtr->swapMemoryAddr);/* free first swap area allocated */
                        cpssOsLpmFree((GT_DMM_BLOCK  *)shadowPtr->secondSwapMemoryAddr);/* free second swap area allocated */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }

                    /* memSize is in lines, each line hold 4 words */
                    SET_OFFSET_IN_WORDS(thirdDmmBlockPtr, 4*((swapFirstBlockOffset+2)*shadowPtr->lpmRamTotalBlocksSizeIncludingGap + memSize));

                    /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
            #if __WORDSIZE == 64
                    thirdDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFFFFFFFFFE;
            #else
                    thirdDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFE;
            #endif

                    /* set third swap area */
                    shadowPtr->thirdSwapMemoryAddr = (GT_UINTPTR)thirdDmmBlockPtr;
                }
            }
            else
            {
                memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[k];
            }
        }

        memSizeBetweenBlocksInBytes  = shadowPtr->lpmRamTotalBlocksSizeIncludingGap * sizeLpmEntryInBytes;

        if (prvCpssDmmCreatePartition(minAllocationSizeInBytes * memSize,
                                      i * memSizeBetweenBlocksInBytes,
                                      minAllocationSizeInBytes,
                                      minAllocationSizeInBytes * maxAllocSizeInLpmLines,
                                      &structsMemPool[k]) != GT_OK)
        {
            cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
            cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
            cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* keep all memory pool Id's for future use, when binding octets to the blocks */
        shadowPtr->lpmRamStructsMemPoolPtr[k] = structsMemPool[k];
        k++;
    }
    if (k != shadowPtr->numOfLpmMemories)
    {
        cpssOsPrintf(" wrong banks number!!!!!!!!!!! %d\n", shadowPtr->numOfLpmMemories);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* update allocation order of blocks according to octet index and memory mode
               Mode          /        Allocate small banks      /     Allocate big banks
      (total up to 30 banks)    (allocate big if no more small)  (allocate small if no more big)
      #Small banks  #Big banks           #Octets                       #Octets
            20          0                   All                           -         CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E
            20          8                   1, 7-15                   0,2-6         not supported yet
            18          12                  1, 7-15                   0,2-6         not supported yet
            14          16                  1, 7-15                   0,2-6         not supported yet
            10          20                  1, 9-15                   0,2-8         not supported yet
            6           24                  1, 9-15                   0,2-8         CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E / MAX_EM_E
            2           28                  1                         0,2-16        CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E

      */

    shadowPtr->octetsGettingSmallBanksPriorityBitMap = memoryCfgPtr->octetsGettingSmallBanksPriorityBitMap;
    shadowPtr->octetsGettingBigBanksPriorityBitMap   = memoryCfgPtr->octetsGettingBigBanksPriorityBitMap;
    shadowPtr->smallBanksIndexesBitMap               = memoryCfgPtr->smallBanksIndexesBitMap;
    shadowPtr->bigBanksIndexesBitMap                 = memoryCfgPtr->bigBanksIndexesBitMap;

    for (i = 0; i < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; i++)
    {
        firstSmallBankIndexFound=GT_FALSE;
        firstBigBankIndexFound=GT_FALSE;

        /* in case there is no big blocks we set 2 small blocks to octet0 and octet1,
           if there are big blocks we set a big block to octet0 and small block to octet1 */
        if (shadowPtr->bigBanksNumber==0)
        {
            /* for all the protocols octet0 is mapped to the first small bank
              octet1 is mapped to the second small bank */
            for (k=0;k<shadowPtr->numOfLpmMemories;k++)
            {
                if (((shadowPtr->smallBanksIndexesBitMap) & (1<<k))!=0)
                {
                    if (firstSmallBankIndexFound==GT_FALSE)
                    {
                        firstSmallBankIndex = k;
                        firstSmallBankIndexFound=GT_TRUE;
                        continue;
                    }
                }
            }

            firstBlockIndex=firstSmallBankIndex;
        }
        else
        {
            /* for all the protocols octet 0 is mapped to the first big bank
               for all the protocols octet 1 is mapped to the first small bank*/
            /* get the first free small bank index */
            for (k=0;k<shadowPtr->numOfLpmMemories;k++)
            {
                if ((firstBigBankIndexFound==GT_TRUE)&&(firstSmallBankIndexFound==GT_TRUE))
                {
                    break;
                }
                if (((shadowPtr->bigBanksIndexesBitMap)&(1<<k))!=0)
                {
                    if (firstBigBankIndexFound==GT_FALSE)
                    {
                        firstBigBankIndex = k;
                        firstBigBankIndexFound=GT_TRUE;
                    }
                }
                if (((shadowPtr->smallBanksIndexesBitMap)&(1<<k))!=0)
                {
                    if (firstSmallBankIndexFound==GT_FALSE)
                    {
                        firstSmallBankIndex = k;
                        firstSmallBankIndexFound=GT_TRUE;
                    }
                }
            }

            firstBlockIndex=firstBigBankIndex;
        }

/*GM do not support new memory banks logic,
  need to stay with old logic that give:
  bank0 to octet0 and bank1 to octet1 */
        if(gmUsed)
        {
            firstBlockIndex=0;
        }

        /* for all the protocols octet0 mapping*/
        shadowPtr->lpmMemInfoArray[i][0].structsMemPool = shadowPtr->lpmRamStructsMemPoolPtr[firstBlockIndex];
        shadowPtr->lpmMemInfoArray[i][0].ramIndex = firstBlockIndex;
        shadowPtr->lpmMemInfoArray[i][0].structsBase = 0;
        shadowPtr->lpmMemInfoArray[i][0].nextMemInfoPtr = NULL;

        /* octet0 was mapped above */
        for (j = 1; j < numOfOctetsPerProtocol[i]; j++)
        {
            /* reset all blocks that are not mapped */
            shadowPtr->lpmMemInfoArray[i][j].structsMemPool = 0;
            shadowPtr->lpmMemInfoArray[i][j].ramIndex = 0;
            shadowPtr->lpmMemInfoArray[i][j].structsBase = 0;
            shadowPtr->lpmMemInfoArray[i][j].nextMemInfoPtr = NULL;
        }

        /* firstBlockIndex is used by all the protocols for octet0  - mark bit firstBlockIndex as 0x1*/
        PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,i,0,firstBlockIndex);
    }

    /* firstBlockIndex is used by default entries */
    shadowPtr->lpmRamOctetsToBlockMappingPtr[firstBlockIndex].isBlockUsed=GT_TRUE;

    /* IPv4 init the lpm level's memory usage */
    /* meaning of i - octet in IP address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];

        /* for the mc grp search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];

        /* for the mc src search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i+PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];
    }

     /* IPv6 init the lpm level's memory usage */
     /* meaning of i - octet in IP address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];

        /* for the mc grp search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];

        /* for the mc src search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i+PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];
    }

    /* FCoE init the lpm level's memory usage */
    /* meaning of i - octet in FCoE address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E][i];
    }

    /* Allocate the device list */
    shadowPtr->shadowDevList.shareDevs = (GT_U8*)cpssOsLpmMalloc(sizeof(GT_U8) * numOfDevs);

    if (shadowPtr->shadowDevList.shareDevs == NULL)
    {
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* set the working devlist (which is empty curretly)*/
    shadowPtr->workDevListPtr = &shadowPtr->shadowDevList;

    cpssOsMemCpy(shadowPtr->isProtocolInitialized, isProtocolInitialized, PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E * sizeof(GT_BOOL));

    shadowPtr->vrfTblSize = PRV_CPSS_DXCH_LPM_RAM_NUM_OF_VIRTUAL_ROUTERS_CNS;
    shadowPtr->vrRootBucketArray =
        cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_STC) * shadowPtr->vrfTblSize);
    if (shadowPtr->vrRootBucketArray == NULL)
    {
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    for (i = 0; i < shadowPtr->vrfTblSize; i++)
    {
        shadowPtr->vrRootBucketArray[i].valid = GT_FALSE;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = NULL;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = NULL;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] = NULL;
    }

    shadowPtr->defragSip6MergeEnable = GT_TRUE;

    return retVal;
}


/**
* @internal createShadowStruct function
* @endinternal
*
* @brief   This function returns a pointer to a new shadow struct.
*
* @param[in] shadowType               - the type of shadow to maintain
* @param[in] protocolBitmap           - the protocols this shodow supports
* @param[in] memoryCfgPtr             - (pointer to) the memory configuration of this LPM DB
*
* @param[out] shadowPtr                - points to the created shadow struct
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS createShadowStruct
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT   shadowType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_BMP      protocolBitmap,
    IN PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC     *memoryCfgPtr,
    OUT PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr
)
{
    GT_U8 numOfDevs;        /* Number of devices in system.                                     */
    GT_STATUS retVal = GT_OK;
    GT_U32 memSize;         /* size of the memory block to be inserted to                       */
                            /* the memory management unit.                                      */
    GT_U32 memSizeBetweenBlocksInBytes; /* the size between each memory block management unit   */
    GT_DMM_BLOCK  *dmmBlockPtr;
    GT_DMM_BLOCK  *secondDmmBlockPtr;

    GT_U32      i,j;
    GT_BOOL     isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E];
    GT_UINTPTR  structsMemPool[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    GT_U32      numOfOctetsPerProtocol[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E] = {PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS,
                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS,
                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS};

    numOfDevs = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* zero out the shadow */
    cpssOsMemSet(shadowPtr,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC));
    shadowPtr->shadowType = shadowType;

    /* update the protocols that the shadow needs to support */
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap);
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap);
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap);

    /* reset the to be freed memory list */
    shadowPtr->freeMemListDuringUpdate = NULL;
    shadowPtr->freeMemListEndOfUpdate = NULL;

    /* we keep the numOfBlocks value as the numOfLpmMemories.
       Block and Memory has the same meaning.
       We use numOfLpmMemories due to legacy code */
    shadowPtr->numOfLpmMemories = memoryCfgPtr->numOfBlocks;

    /* allocate needed RAM configuration arrays */
    shadowPtr->lpmRamBlocksSizeArrayPtr = cpssOsMalloc(sizeof(GT_U32)*shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamBlocksSizeArrayPtr == NULL)
    {
        /* fail to allocate space */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    shadowPtr->lpmRamStructsMemPoolPtr = cpssOsMalloc(sizeof(GT_UINTPTR) * shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamStructsMemPoolPtr == NULL)
    {
        /* fail to allocate space */
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    shadowPtr->lpmRamOctetsToBlockMappingPtr = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamOctetsToBlockMappingPtr == NULL)
    {
        /* fail to allocate space */
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    /* reset needed RAM configuration arrays */
    cpssOsMemSet(shadowPtr->lpmRamBlocksSizeArrayPtr,0,sizeof(GT_U32)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->lpmRamStructsMemPoolPtr,0,sizeof(GT_UINTPTR)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->lpmRamOctetsToBlockMappingPtr,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr,0,sizeof(shadowPtr->globalMemoryBlockTakenArr));

    shadowPtr->lpmMemMode = memoryCfgPtr->lpmMemMode;

    for (i=0;i<shadowPtr->numOfLpmMemories;i++)
    {
         shadowPtr->lpmRamBlocksSizeArrayPtr[i] = memoryCfgPtr->blocksSizeArray[i];

         if(shadowPtr->lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
         {
            shadowPtr->lpmRamBlocksSizeArrayPtr[i]/=2;
         }
    }


    shadowPtr->lpmRamTotalBlocksSizeIncludingGap = PRV_CPSS_DXCH_LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_CNS;
    shadowPtr->lpmRamBlocksAllocationMethod = memoryCfgPtr->blocksAllocationMethod;



    /* init the search memory pools */
    for (i = 0; i < shadowPtr->numOfLpmMemories; i++)
    {
        /* Swap area is in the first block. In order to leave space for it, we
           deduct the size of a max LPM bucket */
        if (i == 0)
        {
             /* in the first block there must be space for two swap areas */
            if (shadowPtr->lpmRamBlocksSizeArrayPtr[0] < PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * 2)
            {
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }

            memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS;
            /* record the swap memory address (at the end of the structs memory)*/
            dmmBlockPtr = (GT_DMM_BLOCK*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK));
            if (dmmBlockPtr == NULL)
            {
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }

            SET_OFFSET_IN_WORDS(dmmBlockPtr, memSize);

            /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
    #if __WORDSIZE == 64
            dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFFFFFFFFFE;
    #else
            dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFE;
    #endif
            /* set first swap area */
            shadowPtr->swapMemoryAddr = (GT_UINTPTR)dmmBlockPtr;

            memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - (2*PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS);
            /* record the swap memory address (at the end of the structs memory)*/
            secondDmmBlockPtr = (GT_DMM_BLOCK*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK));
            if (secondDmmBlockPtr == NULL)
            {
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                cpssOsLpmFree(dmmBlockPtr);/* free first swap area allocated */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }

            SET_OFFSET_IN_WORDS(secondDmmBlockPtr, memSize);

            /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
    #if __WORDSIZE == 64
            secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFFFFFFFFFE;
    #else
            secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION*)0xFFFFFFFE;
    #endif

            /* set second swap area */
            shadowPtr->secondSwapMemoryAddr = (GT_UINTPTR)secondDmmBlockPtr;
        }
        else
        {
            memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[i];
        }

        memSizeBetweenBlocksInBytes  = shadowPtr->lpmRamTotalBlocksSizeIncludingGap * PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS;

        if (prvCpssDmmCreatePartition(DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * memSize,
                                      i * memSizeBetweenBlocksInBytes,
                                      DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS,
                                      DMM_MIN_ALLOCATE_SIZE_IN_BYTE_CNS * PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS,
                                      &structsMemPool[i]) != GT_OK)
        {

            cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
            cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
            cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* keep all memory pool Id's for future use, when binding octets to the blocks */
        shadowPtr->lpmRamStructsMemPoolPtr[i] = structsMemPool[i];
    }

    /* block 0 is used by default entries */
    shadowPtr->lpmRamOctetsToBlockMappingPtr[0].isBlockUsed=GT_TRUE;

    for (i = 0; i < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; i++)
    {
        /* for all the protocols octet 0 is mapped to block 0 */
        shadowPtr->lpmMemInfoArray[i][0].structsMemPool = shadowPtr->lpmRamStructsMemPoolPtr[0];
        shadowPtr->lpmMemInfoArray[i][0].ramIndex = 0;
        shadowPtr->lpmMemInfoArray[i][0].structsBase = 0;
        shadowPtr->lpmMemInfoArray[i][0].nextMemInfoPtr = NULL;

        for (j = 1; j < numOfOctetsPerProtocol[i]; j++)
        {
            /* all blocks except 0 are not mapped */
            shadowPtr->lpmMemInfoArray[i][j].structsMemPool = 0;
            shadowPtr->lpmMemInfoArray[i][j].ramIndex = 0;
            shadowPtr->lpmMemInfoArray[i][j].structsBase = 0;
            shadowPtr->lpmMemInfoArray[i][j].nextMemInfoPtr = NULL;
        }

        /* block 0 is used by all the protocols  - mark bit 0 as 0x1*/
        PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,i,0,0);
    }

    /* IPv4 init the lpm level's memory usage */
    /* meaning of i - octet in IP address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];

        /* for the mc grp search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];

        /* for the mc src search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i+PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];
    }

     /* IPv6 init the lpm level's memory usage */
     /* meaning of i - octet in IP address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];

        /* for the mc grp search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];

        /* for the mc src search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i+PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];
    }

    /* FCoE init the lpm level's memory usage */
    /* meaning of i - octet in FCoE address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E][i];
    }

    /* Allocate the device list */
    shadowPtr->shadowDevList.shareDevs = (GT_U8*)cpssOsLpmMalloc(sizeof(GT_U8) * numOfDevs);

    if (shadowPtr->shadowDevList.shareDevs == NULL)
    {
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* set the working devlist (which is empty curretly)*/
    shadowPtr->workDevListPtr = &shadowPtr->shadowDevList;

    cpssOsMemCpy(shadowPtr->isProtocolInitialized, isProtocolInitialized, PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E * sizeof(GT_BOOL));

    shadowPtr->vrfTblSize = PRV_CPSS_DXCH_LPM_RAM_NUM_OF_VIRTUAL_ROUTERS_CNS;
    shadowPtr->vrRootBucketArray =
        cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_STC) * shadowPtr->vrfTblSize);
    if (shadowPtr->vrRootBucketArray == NULL)
    {
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    for (i = 0; i < shadowPtr->vrfTblSize; i++)
    {
        shadowPtr->vrRootBucketArray[i].valid = GT_FALSE;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = NULL;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = NULL;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] = NULL;
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamDbCreate function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         LPM DB is a database that holds LPM shadow information that is shared
*         to one PP or more. The LPM DB manage adding, deleting and searching
*         This function creates LPM DB for a shared LPM managment.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtrPtr              - the LPM DB
* @param[in] protocolBitmap           - the protocols that are used in this LPM DB
* @param[in] memoryCfgPtr             - (pointer to) the memory configuration of this LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_OUT_OF_CPU_MEM        - on failure to allocate memory
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChLpmRamDbCreate
(
    IN PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT     shadowType,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  **lpmDbPtrPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP        protocolBitmap,
    IN PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC      *memoryCfgPtr
)
{
    GT_STATUS                            retVal = GT_OK;
    GT_U32                               shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr;

    /* right now only one shadow configuration can be attached to LPM DB and
       only PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E is supported */
    GT_U32                              numOfShadowCfg = 1;

    *lpmDbPtrPtr = (PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)
                    cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC));
    if (*lpmDbPtrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    lpmDbPtr = *lpmDbPtrPtr;

    /* allocate memory */
    lpmDbPtr->shadowArray =
        cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC) * numOfShadowCfg);
    if (lpmDbPtr->shadowArray == NULL)
    {
        cpssOsLpmFree(lpmDbPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /*copy out the lpm db info */
    lpmDbPtr->protocolBitmap = protocolBitmap;
    lpmDbPtr->numOfShadowCfg = numOfShadowCfg;

    /* now initlize the shadows */
    for (shadowIdx = 0; (shadowIdx < numOfShadowCfg) && (retVal == GT_OK);
          shadowIdx++)
    {
        if (shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = createSip6ShadowStruct(shadowType,
                                            protocolBitmap,
                                            memoryCfgPtr,
                                            &lpmDbPtr->shadowArray[shadowIdx]);
        }
        else
        {
            retVal = createShadowStruct(shadowType,
                                            protocolBitmap,
                                            memoryCfgPtr,
                                            &lpmDbPtr->shadowArray[shadowIdx]);
        }
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamDbDelete function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         LPM DB is a database that holds LPM shadow information that is shared
*         to one PP or more. The LPM DB manage adding, deleting and searching
*         IPv4/6 UC/MC and FCoE prefixes.
*         This function deletes LPM DB for a shared LPM managment.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin;AC3X;
*                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] shadowType               - the shadow type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_OUT_OF_CPU_MEM        - on failure to allocate memory
* @retval GT_FAIL                  - on error
*
* @note Deleting LPM DB is allowed only if no VR present in the LPM DB (since
*       prefixes reside within VR it means that no prefixes present as well).
*
*/
GT_STATUS prvCpssDxChLpmRamDbDelete
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT     shadowType
)
{
    GT_STATUS                           rc;         /* function return code */
    GT_U32                              i;

    /* make sure the lpm db id exists */
    /* delete all shadows */
    for (i = 0 ; i < lpmDbPtr->numOfShadowCfg ; i++)
    {
        if (shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            rc = deleteSip6ShadowStruct(&lpmDbPtr->shadowArray[i]);
        }
        else
        {
            rc = deleteShadowStruct(&lpmDbPtr->shadowArray[i]);

        }
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* free resources used by the lpm db */
    if (lpmDbPtr->shadowArray != NULL)
    {
        cpssOsLpmFree(lpmDbPtr->shadowArray);
    }

    cpssOsLpmFree(lpmDbPtr);

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamDbConfigGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         LPM DB is a database that holds LPM shadow information that is shared
*         to one PP or more. The LPM DB manages adding, deleting and searching
*         IPv4/6 UC/MC and FCoE prefixes.
*         This function retrieves configuration of LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - (pointer to) the LPM DB
*
* @param[out] protocolBitmapPtr        - (pointer to) the protocols that are used in this LPM DB
* @param[out] memoryCfgPtr             - (pointer to) the memory configuration of this LPM DB
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamDbConfigGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC    *lpmDbPtr,
    OUT PRV_CPSS_DXCH_LPM_PROTOCOL_BMP          *protocolBitmapPtr,
    OUT PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC        *memoryCfgPtr
)
{
    GT_U32 i=0;

    if (protocolBitmapPtr!=NULL)
    {
        *protocolBitmapPtr = lpmDbPtr->protocolBitmap;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if (memoryCfgPtr!=NULL)
    {
        memoryCfgPtr->numOfBlocks = lpmDbPtr->shadowArray[0].numOfLpmMemories;
        for (i=0;i<memoryCfgPtr->numOfBlocks;i++)
        {
            memoryCfgPtr->blocksSizeArray[i] = lpmDbPtr->shadowArray[0].lpmRamBlocksSizeArrayPtr[i];
        }
        memoryCfgPtr->blocksAllocationMethod = lpmDbPtr->shadowArray[0].lpmRamBlocksAllocationMethod;
        memoryCfgPtr->lpmMemMode = lpmDbPtr->shadowArray[0].lpmMemMode;
        memoryCfgPtr->bigBankSize = lpmDbPtr->shadowArray[0].bigBankSize;
        memoryCfgPtr->bigBanksNumber = lpmDbPtr->shadowArray[0].bigBanksNumber;
        for (i=0;i<lpmDbPtr->shadowArray[0].lpmRamConfigInfoNumOfElements;i++)
        {
             memoryCfgPtr->lpmRamConfigInfo[i].devType = lpmDbPtr->shadowArray[0].lpmRamConfigInfo[i].devType;
             memoryCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = lpmDbPtr->shadowArray[0].lpmRamConfigInfo[i].sharedMemCnfg;
        }
        memoryCfgPtr->lpmRamConfigInfoNumOfElements = lpmDbPtr->shadowArray[0].lpmRamConfigInfoNumOfElements;
        memoryCfgPtr->maxNumOfPbrEntries = lpmDbPtr->shadowArray[0].maxNumOfPbrEntries;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal isProtocolSetInBitmap function
* @endinternal
*
* @brief   Check whether the protocol is set in the protocols bitmap
*
* @param[in] protocol                 - bitmap of protocols
* @param[in] protocol                 - the protocol
*
* @retval GT_TRUE                  - the protocol bit is set in the bitmap, or
* @retval GT_FALSE                 - the protocol bit is not set in the bitmap
*/
static GT_BOOL isProtocolSetInBitmap
(
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP          protocolBitmap,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol
)
{
    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        return PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap);
    }
    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
    {
        return PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap);
    }
    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
    {
        return PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap);
    }
    return GT_FALSE;
}

/**
* @internal updateHwSearchStc function
* @endinternal
*
* @brief   updates the search structures accroding to the parameters passed
*
* @param[in] updateType               - indicates which and how entries will be updated
* @param[in] protocolBitmap           - bitmap of protocols to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS updateHwSearchStc
(
    IN PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_TYPE_ENT updateType,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP                   protocolBitmap,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                 *shadowPtr
)
{
    GT_STATUS                                       retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          tmpRootRange;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_TYPE_ENT      lpmTrieUpdateType;
    GT_U32                                          vrId;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            currProtocolStack;

    for (currProtocolStack = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
          currProtocolStack < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
          currProtocolStack++)
    {
        if ((isProtocolSetInBitmap(protocolBitmap, currProtocolStack) == GT_FALSE) ||
            (shadowPtr->isProtocolInitialized[currProtocolStack] == GT_FALSE))
        {
            continue;
        }
        for (vrId = 0 ; vrId < shadowPtr->vrfTblSize; vrId++)
        {
            /* go over all valid VR , if it's initilized for this protocol */
            if ((shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE) ||
                (shadowPtr->vrRootBucketArray[vrId].rootBucket[currProtocolStack] == NULL))
            {
                continue;
            }

            /* if we are in update mode and the vr doesn't need an update , don't
               update */
            if ((updateType != PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_OVERWRITE_E) &&
                (shadowPtr->vrRootBucketArray[vrId].needsHwUpdate == GT_FALSE))
            {
                continue;
            }

            retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                                     &shadowPtr->freeMemListDuringUpdate,shadowPtr);
            if (retVal != GT_OK)
                return retVal;

            if (shadowPtr->vrRootBucketArray[vrId].rootBucket[currProtocolStack])
            {
                switch (updateType)
                {
                case PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E:
                    lpmTrieUpdateType = PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_ONLY_E;
                    break;
                case PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_AND_ALLOC_E:
                    lpmTrieUpdateType = PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_AND_ALLOC_MEM_E;
                    break;
                case PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_OVERWRITE_E:
                    lpmTrieUpdateType = PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E;
                    break;
                CPSS_COVERITY_NON_ISSUE_BOOKMARK
                /* coverity[dead_error_begin] */
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                /* write the changes in the tree */
                tmpRootRange.lowerLpmPtr.nextBucket =
                    shadowPtr->vrRootBucketArray[vrId].rootBucket[currProtocolStack];
                tmpRootRange.pointerType =
                    (GT_U8)(shadowPtr->vrRootBucketArray[vrId].rootBucketType[currProtocolStack]);
                tmpRootRange.next = NULL;
                if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
                {
                    retVal =
                        prvCpssDxChLpmSip6RamMngBucketTreeWrite(&tmpRootRange,
                                                                shadowPtr->ucSearchMemArrayPtr[currProtocolStack],
                                                                shadowPtr,lpmTrieUpdateType,
                                                                currProtocolStack,vrId);
                }
                else
                {
                    retVal =
                        prvCpssDxChLpmRamMngBucketTreeWrite(&tmpRootRange,
                                                            shadowPtr->ucSearchMemArrayPtr[currProtocolStack],
                                                            shadowPtr,lpmTrieUpdateType,
                                                            currProtocolStack,vrId);
                }
                if (retVal == GT_OK)
                {
                    /* now write the vr table */
                    retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(vrId, currProtocolStack, shadowPtr);
                    if (retVal == GT_OK)
                    {
                        /* indicate the update was done */
                        shadowPtr->vrRootBucketArray[vrId].needsHwUpdate = GT_FALSE;

                    }
                    else
                    {

                        return retVal;

                    }

                }
                else
                {
                    return retVal;
                }
            }

            /* now it's possible to free all the "end of update to be freed
               memory" memory , now that the HW is updated (and the "during
               update to be freed" memory if needed ) */
            retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                     &shadowPtr->freeMemListDuringUpdate,shadowPtr);

            if (retVal != GT_OK)
            {
                return retVal;
            }

            retVal = prvCpssDxChLpmRamMemFreeListMng(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                     &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

            if (retVal != GT_OK)
            {
                return retVal;
            }
        }
    }

    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamDbDevListAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function adds list of devices to an existing LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] devListArray[]           - array of device ids to add to the LPM DB
* @param[in] numOfDevs                - the number of device ids in the array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbDevListAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U8                                 devListArray[],
    IN GT_U32                                numOfDevs
)
{
    GT_STATUS                                   retVal = GT_OK;
    GT_U32                                      shadowIdx, devIdx, i;
    GT_U32                                      numOfShadowDevs=0;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC  addedDevList;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC  *shadowDevListPtr;
    GT_U32                                      totalLpmSize=0,lpmRamNumOfLines=0;
    GT_BOOL                                     memoryModeFits = GT_TRUE;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC             *moduleCfgPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC               tempSystemRecovery_Info;
    GT_BOOL                                     managerHwWriteBlock;

    /* first check the devices we got fit the lpm DB shadows */
    for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
    {
        /*Lock the access to per device data base in order to avoid corruption*/
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if(PRV_CPSS_SIP_6_CHECK_MAC(devListArray[devIdx]))
        {
            retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
            if (retVal != GT_OK)
            {
                return retVal;
            }
            managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
            if (((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E) && (managerHwWriteBlock == GT_FALSE)) ||
                (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E))
            {
                retVal = prvCpssDxChSip6LpmAccParamSet(devListArray[devIdx]);
                if(retVal != GT_OK)
                {
                    /* Unlock the access to per device data base */
                    CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                    CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, LOG_ERROR_NO_MSG);
                }
            }
        }

        moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devListArray[devIdx])->moduleCfg);
        for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
        {
            /* check if shadow fit to devices*/
            if ( (lpmDbPtr->shadowArray[shadowIdx].shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E) &&
                 (!PRV_CPSS_SIP_5_CHECK_MAC(devListArray[devIdx])))
            {
                /*Unlock the access to per device data base*/
                CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if ( (lpmDbPtr->shadowArray[shadowIdx].shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E) &&
                 (!PRV_CPSS_SIP_6_CHECK_MAC(devListArray[devIdx])))
            {
                /*Unlock the access to per device data base*/
                CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            totalLpmSize = 0;
            /* calculate the size of the RAM in this LPM DB */
            for (i=0;i<lpmDbPtr->shadowArray[shadowIdx].numOfLpmMemories;i++)
            {
                totalLpmSize += lpmDbPtr->shadowArray[shadowIdx].lpmRamBlocksSizeArrayPtr[i];
            }
            /* check if the device has enough RAM space to contain the LPM DB configuration */
            if (lpmDbPtr->shadowArray[shadowIdx].shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
            {
                 lpmRamNumOfLines = PRV_CPSS_DXCH_SIP6_LPM_RAM_GET_NUM_OF_LINES_MAC(PRV_CPSS_DXCH_PP_MAC(devListArray[devIdx])->fineTuning.tableSize.lpmRam);
            }
            else
            {
                lpmRamNumOfLines = PRV_CPSS_DXCH_LPM_RAM_GET_NUM_OF_LINES_MAC(PRV_CPSS_DXCH_PP_MAC(devListArray[devIdx])->fineTuning.tableSize.lpmRam);
            }

            /*Check if memory mode fits*/
            if(lpmDbPtr->shadowArray[shadowIdx].shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E)
            {
                if(lpmDbPtr->shadowArray[shadowIdx].lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
                {
                    if(PRV_CPSS_DXCH_PP_MAC(devListArray[devIdx])->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E)
                    {
                        memoryModeFits = GT_FALSE;
                    }
                    else
                    /*if we are working in half memory mode then we have half of the lines*/
                    {
                        lpmRamNumOfLines/=2;
                    }
                }
                else
                {
                    if(PRV_CPSS_DXCH_PP_MAC(devListArray[devIdx])->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
                    {
                        memoryModeFits = GT_FALSE;
                    }
                }

                if(memoryModeFits == GT_FALSE)
                {
                    /*Unlock the access to per device data base*/
                    CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    /* not compatible memory mode */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL , LOG_ERROR_NO_MSG);
                }

            }
            if(lpmDbPtr->shadowArray[shadowIdx].shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
            {
                if( (lpmRamNumOfLines - moduleCfgPtr->ip.maxNumOfPbrEntries) < totalLpmSize)
                {
                    /*Unlock the access to per device data base*/
                    CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    /* not enough memory in the device */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                }
            }
            else
            {    /* SIP6 devices*/
                if(!((moduleCfgPtr->ip.numOfBigBanks >= lpmDbPtr->shadowArray[shadowIdx].bigBanksNumber) &&
                     (moduleCfgPtr->ip.numOfSmallBanks >= lpmDbPtr->shadowArray[shadowIdx].smallBanksNumber)&&
                     (moduleCfgPtr->ip.lpmBankSize >= lpmDbPtr->shadowArray[shadowIdx].bigBankSize) &&
                     (moduleCfgPtr->ip.numOfBigBanks+moduleCfgPtr->ip.numOfSmallBanks >= lpmDbPtr->shadowArray[shadowIdx].numOfLpmMemories) ))
                {
                    /*Unlock the access to per device data base*/
                    CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    /* not enough memory in the device */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                }
                if( (lpmRamNumOfLines - (moduleCfgPtr->ip.maxNumOfPbrEntries/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS)) < totalLpmSize)
                {
                    /*Unlock the access to per device data base*/
                    CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    /* not enough memory in the device */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                }


            }

            shadowDevListPtr =
                &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;

            /* ok the device fits, see that it doesn't exist already */
            for (i = 0 ; i < shadowDevListPtr->shareDevNum;i++)
            {
                if (shadowDevListPtr->shareDevs[i] == devListArray[devIdx])
                {
                   /*Unlock the access to per device data base*/
                    CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                   /* the device exists, can't re-add it */
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
                }
            }
        }

        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    }

    /* if we reached here, all the added devices are ok, add them */
    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        numOfShadowDevs = 0;
        for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
        {
            PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(shadowDevList)[numOfShadowDevs] = devListArray[devIdx];
            numOfShadowDevs++;
        }
        /* check if we have added device for the shadow , and if there are add
           them */
        if (numOfShadowDevs > 0)
        {
            addedDevList.shareDevNum = numOfShadowDevs;
            addedDevList.shareDevs   = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(shadowDevList);

            /* change the working devlist*/
            lpmDbPtr->shadowArray[shadowIdx].workDevListPtr = &addedDevList;
            retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
            if (retVal != GT_OK)
            {
                return retVal;
            }
            managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
            if ( ((tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) && (managerHwWriteBlock == GT_FALSE)) ||
                 ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
                 (tempSystemRecovery_Info.systemRecoveryState ==CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)) )
            {
                retVal = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_OVERWRITE_E,
                                           lpmDbPtr->protocolBitmap,
                                           &lpmDbPtr->shadowArray[shadowIdx]);
                if (retVal != GT_OK)
                {
                    return (retVal);
                }
            }

            /* change back the working devlist*/
            lpmDbPtr->shadowArray[shadowIdx].workDevListPtr =
                &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;

            /* now register them */
            shadowDevListPtr = &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;
            for (i = 0; i < numOfShadowDevs; i++)
            {
                shadowDevListPtr->shareDevs[shadowDevListPtr->shareDevNum] =
                    PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(shadowDevList)[i];
                shadowDevListPtr->shareDevNum++;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamDbDevListRemove function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function removes devices from an existing LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] devListArray[]           - array of device ids to remove from the LPM DB
* @param[in] numOfDevs                - the number of device ids in the array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChLpmRamDbDevListRemove
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U8                                 devListArray[],
    IN GT_U32                                numOfDevs
)
{
    GT_U32                                      shadowIdx, devIdx, i, j;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC  *shadowDevListPtr;
    GT_BOOL                                     devExists;

    /* first check the devices we got are ok */
    for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
    {
        /*Lock the access to per device data base in order to avoid corruption*/
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    /* first check that the devices exist in the shadows */
    for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
    {
        for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
        {
            shadowDevListPtr = &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;

            devExists = GT_FALSE;
            for (i = 0 ; i < shadowDevListPtr->shareDevNum;i++)
            {
                if (shadowDevListPtr->shareDevs[i] == devListArray[devIdx])
                {
                    /* found it */
                    devExists = GT_TRUE;
                    break;
                }
            }
            if (devExists == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
            }
        }
    }

    /* now go and remove them from their shadow */
    for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
    {
        for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
        {
            shadowDevListPtr = &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;

            /* ok the device fits, see that it doesn't exist already */
            for (i = 0 ; i < shadowDevListPtr->shareDevNum;i++)
            {
                if (shadowDevListPtr->shareDevs[i] == devListArray[devIdx])
                {
                    /* found it - remove it*/
                    for (j = i ; j < shadowDevListPtr->shareDevNum - 1; j++)
                    {
                        shadowDevListPtr->shareDevs[j] =
                            shadowDevListPtr->shareDevs[j+1];
                    }
                    shadowDevListPtr->shareDevNum--;
                }
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamDbDevListGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function retrieves the list of devices in an existing LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in,out] numOfDevsPtr             - points to the size of devListArray
* @param[in,out] numOfDevsPtr             - points to the number of devices retrieved
*
* @param[out] devListArray[]           - array of device ids in the LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to device list
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The application should allocate memory for the device list array.
*       If the function returns GT_OK, then number of devices holds
*       the number of devices the function filled.
*       If the function returns GT_BAD_SIZE, then the memory allocated by the
*       application to the device list is not enough. In this case the
*       number of devices will hold the size of array needed.
*
*/
GT_STATUS prvCpssDxChLpmRamDbDevListGet
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    INOUT GT_U32                                *numOfDevsPtr,
    OUT   GT_U8                                 devListArray[]
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC *shadowDevListPtr;

    shadowDevListPtr = &lpmDbPtr->shadowArray[0].shadowDevList;

    if (*numOfDevsPtr < shadowDevListPtr->shareDevNum)
    {
        *numOfDevsPtr = shadowDevListPtr->shareDevNum;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);
    }

    for (i = 0 ; i < shadowDevListPtr->shareDevNum ; i++)
    {
        devListArray[i] = shadowDevListPtr->shareDevs[i];
    }
    *numOfDevsPtr = shadowDevListPtr->shareDevNum;

    return GT_OK;
}



/**
* @internal removeSupportedProtocolsFromVirtualRouter function
* @endinternal
*
* @brief   This function removes all supported protocols from virtual router for specific shadow.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] vrId                     - The virtual's router ID.
* @param[in] shadowPtr                - the shadow to work on.
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_BAD_STATE             - if the existing VR is not empty.
*
* @note The VR must be empty from prefixes in order to be Removed!
*
*/
static GT_STATUS removeSupportedProtocolsFromVirtualRouter
(
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr
)
{
    GT_STATUS                                 retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protStk;
    GT_UINTPTR                                handle;
    GT_U32                                    blockIndex;

    for (protStk = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E; protStk < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; protStk++)
    {
        if (shadowPtr->vrRootBucketArray[vrId].rootBucket[protStk] == NULL)
        {
            continue;
        }

        handle = shadowPtr->vrRootBucketArray[vrId].rootBucket[protStk]->hwBucketOffsetHandle;

        if (0 == handle)
        {
            continue;
        }

        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            if (handle==0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected handle=0\n");
            }
            /*need to free memory*/
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(handle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
            }
            /* set pending flag for future need */
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates += PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(handle);
        }
        else
        {
            if (handle==0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR - Can not have handle ==0, in removeSupportedProtocolsFromVirtualRouter");
            }
            /*need to free memory*/
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(handle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            /* set pending flag for future need */
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates += PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(handle);
        }
        prvCpssDmmFree(handle);
        retVal = prvCpssDxChLpmRamMngRootBucketDelete(shadowPtr, vrId, protStk);
        if (retVal != GT_OK)
        {
            /* we fail in allocation, reset pending array */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            return retVal;
        }

        shadowPtr->vrRootBucketArray[vrId].rootBucket[protStk] = NULL;
        /* update counters for UC allocation */
        retVal = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                           shadowPtr->pendingBlockToUpdateArr,
                                                           shadowPtr->protocolCountersPerBlockArr,
                                                           shadowPtr->pendingBlockToUpdateArr,
                                                           protStk,
                                                           shadowPtr->numOfLpmMemories);
        if (retVal!=GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal;
        }
    }
    return retVal;
}

/**
* @internal lpmVirtualRouterSet function
* @endinternal
*
* @brief   This function sets a virtual router in system for specific shadow.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] vrId                     - The virtual's router ID.
* @param[in] defIpv4UcNextHopPointer  - the pointer info of the default IPv4 UC next
*                                      hop for this virtual router.
* @param[in] defIpv6UcNextHopPointer  - the pointer info of the default IPv6 UC next
*                                      hop for this virtual router.
* @param[in] defFcoeNextHopPointer    - the pointer info of the default FCoE next
*                                      hop for this virtual router.
* @param[in] defIpv4McRoutePointer    - the pointer info of the default IPv4 MC route
*                                      for this virtual router.
* @param[in] defIpv6McRoutePointer    - the pointer info of the default IPv6 MC route
*                                      for this virtual router.
* @param[in] protocolBitmap           - types of protocol stack used in this virtual router.
* @param[in] ucSupportArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS] - Boolean array stating UC support for every protocol stack
* @param[in] mcSupportArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS] - Boolean array stating MC support for every protocol stack
* @param[in] shadowPtr                - the shadow to work on.
* @param[in] updateHw                 - GT_TRUE : update the VR in the HW
*                                      GT_FALSE : do not update the VR in the HW
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_ALREADY_EXIST         - if the vr id is already used
*/
static GT_STATUS lpmVirtualRouterSet
(
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv4UcNextHopPointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv6UcNextHopPointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defFcoeNextHopPointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv4McRoutePointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv6McRoutePointer,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP               protocolBitmap,
    IN GT_BOOL                                      ucSupportArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS],
    IN GT_BOOL                                      mcSupportArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS],
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_BOOL                                      updateHw
)
{
    GT_STATUS                                       retVal;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            protStk;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *defUcNextHopEntry;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *defReservedNextHopEntry;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *defMcNextHopEntry;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       defUcNextHopPointer;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       defMcNextHopPointer;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       defReservedNextHopPointer;

    cpssOsMemSet(&defUcNextHopPointer,0,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    cpssOsMemSet(&defMcNextHopPointer,0,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    for (protStk = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
         protStk < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
         protStk++)
    {
        defUcNextHopEntry = NULL;
        defReservedNextHopEntry = NULL;
        defMcNextHopEntry = NULL;

        if (isProtocolSetInBitmap(protocolBitmap, protStk) == GT_FALSE)
        {
            if (protStk < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS)
            {
                shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protStk] = GT_FALSE;
            }
            if (protStk < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS)
            {
                shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protStk] = GT_FALSE;
            }
            continue;
        }

        if (protStk <= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
        {
            if ((ucSupportArr[protStk] == GT_FALSE) && (mcSupportArr[protStk] == GT_FALSE))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            if (ucSupportArr[protStk] == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        /* init UC structures */
        if (ucSupportArr[protStk] == GT_TRUE)
        {
            shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protStk] = GT_TRUE;
            if (protStk == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                defUcNextHopPointer = defIpv4UcNextHopPointer;
                defReservedNextHopPointer = defIpv4UcNextHopPointer;
            }
            else if (protStk == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
            {
                defUcNextHopPointer = defIpv6UcNextHopPointer;
            }
            else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E */
            {
                defUcNextHopPointer = defFcoeNextHopPointer;
            }

            /* allocate space for the new default */
            defUcNextHopEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            if (defUcNextHopEntry == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }
            cpssOsMemCpy(defUcNextHopEntry, &defUcNextHopPointer,
                         sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            if (protStk == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                defReservedNextHopEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                if (defReservedNextHopEntry == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
                cpssOsMemCpy(defReservedNextHopEntry, &defReservedNextHopPointer,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            }

        }
        else
        {
            shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protStk] = GT_FALSE;
        }

        /* now init mc stuctures */
        if (protStk <= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
        {
            if (mcSupportArr[protStk] == GT_TRUE)
            {
                shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protStk] = GT_TRUE;
                if (protStk == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
                {
                    defMcNextHopPointer = defIpv4McRoutePointer;
                }
                else /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
                {
                    defMcNextHopPointer = defIpv6McRoutePointer;
                }

                /* In the following cases we need to create a new nexthop pointer
                   info structure:
                   1. UC is not supported for this protocol so we didn't create it yet
                   2. UC is supported but the defaults of the UC and MC are different */
                if ((ucSupportArr[protStk] == GT_FALSE) ||
                    (defUcNextHopPointer.routeEntryBaseMemAddr != defMcNextHopPointer.routeEntryBaseMemAddr) ||
                    (defUcNextHopPointer.routeEntryMethod != defMcNextHopPointer.routeEntryMethod))
                {
                    defMcNextHopEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                    if (defMcNextHopEntry == NULL)
                    {
                        if (ucSupportArr[protStk] == GT_TRUE)
                        {
                             cpssOsLpmFree(defUcNextHopEntry);
                        }
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }

                    cpssOsMemCpy(defMcNextHopEntry, &defMcNextHopPointer,
                                 sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                }
                else
                {
                    /* both UC and MC are supported, with the same default NH */
                    defMcNextHopEntry = defUcNextHopEntry;
                }
            }
            else    /* mcSupportArr[protStk] == GT_FALSE */
            {
                shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protStk] = GT_FALSE;
            }
        }

        /* Create the root bucket */
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamMngRootBucketCreate(shadowPtr, vrId, protStk,
                                                              defUcNextHopEntry,
                                                              defReservedNextHopEntry,
                                                              defMcNextHopEntry,
                                                              updateHw);
        }
        else
        {
            retVal = prvCpssDxChLpmRamMngRootBucketCreate(shadowPtr, vrId, protStk,
                                                          defUcNextHopEntry,
                                                          defReservedNextHopEntry,
                                                          defMcNextHopEntry,
                                                          updateHw);
        }
        if (retVal != GT_OK)
        {
            if (ucSupportArr[protStk] == GT_TRUE)
            {
                cpssOsLpmFree(defUcNextHopEntry);
            }
            if ((protStk <= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) && (mcSupportArr[protStk] == GT_TRUE))
            {
                if ((ucSupportArr[protStk] == GT_FALSE) ||
                    (defUcNextHopPointer.routeEntryBaseMemAddr != defMcNextHopPointer.routeEntryBaseMemAddr) ||
                    (defUcNextHopPointer.routeEntryMethod != defMcNextHopPointer.routeEntryMethod))
                {
                    cpssOsLpmFree(defMcNextHopEntry);
                }
            }
            return retVal;
        }

        /* Don't touch the hw in a HSU process and in recovery process after HA event */
        if(updateHw == GT_TRUE)
        {
            /* write the mc and uc vr router table */
            retVal = prvCpssDxChLpmRamMngVrfEntryUpdate(vrId, protStk, shadowPtr);

            if (retVal != GT_OK)
            {
                if (ucSupportArr[protStk] == GT_TRUE)
                {
                    cpssOsLpmFree(defUcNextHopEntry);
                }
                if ((protStk <= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) && (mcSupportArr[protStk] == GT_TRUE))
                {
                    if ((ucSupportArr[protStk] == GT_FALSE) ||
                        (defUcNextHopPointer.routeEntryBaseMemAddr != defMcNextHopPointer.routeEntryBaseMemAddr) ||
                        (defUcNextHopPointer.routeEntryMethod != defMcNextHopPointer.routeEntryMethod))
                    {
                        cpssOsLpmFree(defMcNextHopEntry);
                    }
                }
                return retVal;
            }
        }
    }

    shadowPtr->vrRootBucketArray[vrId].valid = GT_TRUE;

    /* the data was written successfully to HW, we can reset the information regarding the new memoryPool allocations done.
    next call will set this array with new values of allocated/bound blocks */
    cpssOsMemSet(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr,0,sizeof(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));

    cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

    return GT_OK;
}

/**
* @internal lpmVirtualRouterRemove function
* @endinternal
*
* @brief   This function removes a virtual router in system for specific shadow.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] vrId                     - The virtual's router ID.
* @param[in] shadowPtr                - the shadow to work on.
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_BAD_STATE             - if the existing VR is not empty.
*
* @note The VR must be empty from prefixes in order to be Removed!
*
*/
static GT_STATUS lpmVirtualRouterRemove
(
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr
)
{
    GT_STATUS                                 retVal = GT_OK;

    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    /* this is an existing Vr - delete the allocated structures */
    retVal = removeSupportedProtocolsFromVirtualRouter(vrId,shadowPtr);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    shadowPtr->vrRootBucketArray[vrId].valid = GT_FALSE;

    return retVal;
}



/**
* @internal prvCpssDxChLpmRamVirtualRouterAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function adds a virtual router for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] vrId                     - The virtual's router ID.
* @param[in] vrConfigPtr              - Configuration of the virtual router
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_ALREADY_EXIST         - if the vr id is already used
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamVirtualRouterAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId,
    IN PRV_CPSS_DXCH_LPM_RAM_VR_CONFIG_STC   *vrConfigPtr
)
{
    GT_STATUS                              retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC       *shadowPtr;
    GT_U32                                 shadowIdx;

    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP         protocolBitmap = 0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP         protocolDifference;
    GT_BOOL                                vrUcSupport[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS];
    GT_BOOL                                vrMcSupport[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS];
    CPSS_SYSTEM_RECOVERY_INFO_STC          tempSystemRecovery_Info;
    GT_BOOL                                updateHw;
    GT_BOOL                                managerHwWriteBlock;


    vrUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = (vrConfigPtr->supportUcIpv4) ? GT_TRUE : GT_FALSE;
    vrUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = (vrConfigPtr->supportUcIpv6) ? GT_TRUE : GT_FALSE;
    vrUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] = (vrConfigPtr->supportFcoe)   ? GT_TRUE : GT_FALSE;
    vrMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = (vrConfigPtr->supportMcIpv4) ? GT_TRUE : GT_FALSE;
    vrMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = (vrConfigPtr->supportMcIpv6) ? GT_TRUE : GT_FALSE;

    /* determine the supported protocols */
    if ((vrConfigPtr->supportUcIpv4 == GT_TRUE) || (vrConfigPtr->supportMcIpv4 == GT_TRUE))
    {
        protocolBitmap |= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_BIT_E;
    }
    if ((vrConfigPtr->supportUcIpv6 == GT_TRUE) || (vrConfigPtr->supportMcIpv6 == GT_TRUE))
    {
        protocolBitmap |= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_BIT_E;
    }
    if (vrConfigPtr->supportFcoe == GT_TRUE)
    {
        protocolBitmap |= PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_BIT_E;
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    protocolDifference = protocolBitmap ^ lpmDbPtr->protocolBitmap;
    if (protocolDifference)
    {
        if (protocolDifference & protocolBitmap)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
        }
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (vrConfigPtr->supportUcIpv4 == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,
                                                          &vrConfigPtr->defaultUcIpv4RouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->supportMcIpv4 == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,
                                                          &vrConfigPtr->defaultMcIpv4RouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->supportUcIpv6 == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,
                                                          &vrConfigPtr->defaultUcIpv6RouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->supportMcIpv6 == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,
                                                          &vrConfigPtr->defaultMcIpv6RouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->supportFcoe == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,
                                                          &vrConfigPtr->defaultFcoeForwardingEntry);
            if (retVal != GT_OK)
                return retVal;
        }


        if (vrId >= shadowPtr->vrfTblSize)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
           (managerHwWriteBlock == GT_TRUE) )
        {
            updateHw=GT_FALSE;
        }
        else
        {
            updateHw=GT_TRUE;
        }
        retVal = lpmVirtualRouterSet(vrId,
                                     vrConfigPtr->defaultUcIpv4RouteEntry,
                                     vrConfigPtr->defaultUcIpv6RouteEntry,
                                     vrConfigPtr->defaultFcoeForwardingEntry,
                                     vrConfigPtr->defaultMcIpv4RouteEntry,
                                     vrConfigPtr->defaultMcIpv6RouteEntry,
                                     protocolBitmap,
                                     vrUcSupport,
                                     vrMcSupport,
                                     shadowPtr,
                                     updateHw);
        if (retVal != GT_OK)
        {
            if (retVal != GT_ALREADY_EXIST)
            {
                /* free all allocated resources  */
                removeSupportedProtocolsFromVirtualRouter(vrId,shadowPtr);
            }
            return(retVal);
        }
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamVirtualRouterDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function deletes a virtual router for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] vrId                     - The virtual's router ID.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_FOUND             - if the LPM DB id or vr id does not found
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_BAD_STATE             - if the existing VR is not empty.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All prefixes must be previously deleted.
*
*/
GT_STATUS prvCpssDxChLpmRamVirtualRouterDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId
)
{
    GT_STATUS                           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;
    GT_U32                              shadowIdx;

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* delete all the IPv4 unicast prefixes for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E])
        {
                retVal = prvCpssDxChLpmRamIpv4UcPrefixesFlush(lpmDbPtr,vrId);
                if (retVal != GT_OK)
                {
                    return (retVal);
                }
        }

        /* delete all the IPv4 multicast entries for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E])
        {
            retVal = prvCpssDxChLpmRamIpv4McEntriesFlush(lpmDbPtr,vrId);
            if (retVal != GT_OK)
            {
                return (retVal);
            }
        }

        /* delete all the IPv6 unicast prefixes for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E])
        {
            retVal = prvCpssDxChLpmRamIpv6UcPrefixesFlush(lpmDbPtr,vrId);
            if (retVal != GT_OK)
            {
                return (retVal);
            }
        }

        /* delete all the IPv6 multicast entries for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E])
        {
            retVal = prvCpssDxChLpmRamIpv6McEntriesFlush(lpmDbPtr,vrId);
            if (retVal != GT_OK)
            {
                return (retVal);
            }
        }

        /* delete all the FCoE prefixes for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E])
        {
            retVal = prvCpssDxChLpmRamFcoePrefixesFlush(lpmDbPtr,vrId);
            if (retVal != GT_OK)
            {
                return (retVal);
            }
        }

        retVal = lpmVirtualRouterRemove(vrId, shadowPtr);
        if (retVal != GT_OK)
        {
            return (retVal);
        }
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamVirtualRouterGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function gets configuration of a virtual router for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - the LPM DB.
* @param[in] vrId                     - The virtual's router ID.
*
* @param[out] vrConfigPtr              - Configuration of the virtual router
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_FOUND             - if the LPM DB id or vr id does not found
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - if the existing VR is not empty.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamVirtualRouterGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN  GT_U32                               vrId,
    OUT PRV_CPSS_DXCH_LPM_RAM_VR_CONFIG_STC  *vrConfigPtr
)
{
    GT_STATUS                                   retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *defRoutePointerPtr = NULL;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   routeEntry;
    GT_IPV6ADDR                                 ipv6Addr;
    GT_IPADDR                                   ipv4Addr;
    GT_FCID                                     fcoeAddr;

    shadowPtr = &lpmDbPtr->shadowArray[0];

    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* IPv4 MC */
    if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] == GT_TRUE)
    {
        retVal = prvCpssDxChLpmRamMcDefRouteGet(vrId ,&defRoutePointerPtr,
                                                shadowPtr ,PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((defRoutePointerPtr == NULL) || (retVal == GT_NOT_FOUND) || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportMcIpv4 = GT_FALSE;
        else
        {
            vrConfigPtr->defaultMcIpv4RouteEntry = *defRoutePointerPtr;
            vrConfigPtr->supportMcIpv4 = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportMcIpv4 = GT_FALSE;
    }

    /* IPv4 UC */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] == GT_TRUE)
    {
        cpssOsMemSet(&ipv4Addr, 0, sizeof(ipv4Addr));
        retVal = prvCpssDxChLpmRamIpv4UcPrefixSearch(lpmDbPtr,vrId,ipv4Addr,0,&routeEntry);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((retVal == GT_NOT_FOUND)  || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportUcIpv4 = GT_FALSE;
        else
        {
            vrConfigPtr->defaultUcIpv4RouteEntry = routeEntry;
            vrConfigPtr->supportUcIpv4 = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportUcIpv4 = GT_FALSE;
    }

    /* IPv6 MC */
    if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] == GT_TRUE)
    {
        retVal = prvCpssDxChLpmRamMcDefRouteGet(vrId, &defRoutePointerPtr,
                                                shadowPtr, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((defRoutePointerPtr == NULL) || (retVal == GT_NOT_FOUND) || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportMcIpv6 = GT_FALSE;
        else
        {
            vrConfigPtr->defaultMcIpv6RouteEntry = *defRoutePointerPtr;
            vrConfigPtr->supportMcIpv6 = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportMcIpv6 = GT_FALSE;
    }

    /* IPv6 UC */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] == GT_TRUE)
    {
        cpssOsMemSet(&ipv6Addr, 0, sizeof(ipv6Addr));
        retVal = prvCpssDxChLpmRamIpv6UcPrefixSearch(lpmDbPtr,vrId,ipv6Addr,0,&routeEntry);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((retVal == GT_NOT_FOUND)  || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportUcIpv6 = GT_FALSE;
        else
        {
            vrConfigPtr->defaultUcIpv6RouteEntry = routeEntry;
            vrConfigPtr->supportUcIpv6 = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportUcIpv6 = GT_FALSE;
    }

    /* FCoE */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] == GT_TRUE)
    {
        cpssOsMemSet(&fcoeAddr, 0, sizeof(fcoeAddr));
        retVal = prvCpssDxChLpmRamFcoePrefixSearch(lpmDbPtr,vrId,fcoeAddr,0,&routeEntry);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((retVal == GT_NOT_FOUND)  || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportFcoe = GT_FALSE;
        else
        {
            vrConfigPtr->defaultFcoeForwardingEntry = routeEntry;
            vrConfigPtr->supportFcoe = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportFcoe = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function adds a new IPv4 prefix to a Virtual Router in a
*         specific LPM DB or overrides an existing existing IPv4 prefix.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
* @param[in] nextHopInfoPtr           - (points to) The next hop pointer to set for this prefix.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_ALREADY_EXIST         - prefix already exist when override is GT_FALSE
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN GT_U32                                       vrId,
    IN GT_IPADDR                                    ipAddr,
    IN GT_U32                                       prefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *nextHopInfoPtr,
    IN GT_BOOL                                      override,
    IN GT_BOOL                                      defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    GT_BOOL                       managerHwWriteBlock;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,nextHopInfoPtr);
        if (retVal != GT_OK)
            return retVal;


        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
           (managerHwWriteBlock == GT_TRUE))
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E;
        }
        else
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
        }
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamUcEntryAdd(vrId,
                                                     ipAddr.arIP,
                                                     prefixLen,
                                                     nextHopInfoPtr,
                                                     insertMode,
                                                     override,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                     shadowPtr,
                                                     defragmentationEnable);
        }
        else
        {
            retVal = prvCpssDxChLpmRamUcEntryAdd(vrId,
                                                 ipAddr.arIP,
                                                 prefixLen,
                                                 nextHopInfoPtr,
                                                 insertMode,
                                                 override,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr,
                                                 defragmentationEnable);
        }
        if (retVal != GT_OK)
        {
            /* reset the array for next use */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            break;
        }
        /* reset the array for next use */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList function
* @endinternal
*
* @brief   Update a summary list of all pending block values to be used at
*         the end of bulk operation
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] totalPendingBlockToUpdateArr[] - array holding all updates done until now
* @param[in] pendingBlockToUpdateArr[] - array of new updates to be kept in totalPendingBlockToUpdateArr
*
* @retval GT_OK                    - on succes
*/
GT_STATUS prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList
(
    IN PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC totalPendingBlockToUpdateArr[],
    IN PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC pendingBlockToUpdateArr[]
)
{
    GT_U32 blockIndex=0;

    for (blockIndex = 0 ; blockIndex < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS; blockIndex++)
    {   /* we only keep the values were update=GT_TRUE, since this is the sum of all updates done */
        if(pendingBlockToUpdateArr[blockIndex].updateInc==GT_TRUE)
        {
            totalPendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
            totalPendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                                        pendingBlockToUpdateArr[blockIndex].numOfIncUpdates;
          /* reset pending array for next element */
            pendingBlockToUpdateArr[blockIndex].updateInc=GT_FALSE;
            pendingBlockToUpdateArr[blockIndex].numOfIncUpdates=0;
        }
        if(pendingBlockToUpdateArr[blockIndex].updateDec==GT_TRUE)
        {
            totalPendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            totalPendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                            pendingBlockToUpdateArr[blockIndex].numOfDecUpdates;
            /* reset pending array for next element */
            pendingBlockToUpdateArr[blockIndex].updateDec=GT_FALSE;
            pendingBlockToUpdateArr[blockIndex].numOfDecUpdates=0;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamUpdateBlockUsageCounters function
* @endinternal
*
* @brief   Update block usage counters according to pending block values
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmRamBlocksSizeArrayPtr - used for finding lpm lines per block
* @param[in] totalPendingBlockToUpdateArr[] - array holding all updates done until now
* @param[in] protocol                 - counters should be updated for given protocol
* @param[in] resetPendingBlockToUpdateArr[] - array need to be reset
* @param[in] protocol                 - counters should be updated for given protocol
* @param[in] numOfLpmMemories         -  number of LPM memories
*
* @retval GT_OK                    - on succes
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvCpssDxChLpmRamUpdateBlockUsageCounters
(
    IN GT_U32                                            *lpmRamBlocksSizeArrayPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC totalPendingBlockToUpdateArr[],
    IN PRV_CPSS_DXCH_LPM_ADDRESS_COUNTERS_INFO_STC       protocolCountersPerBlockArr[],
    IN PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC resetPendingBlockToUpdateArr[],
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT              protocol,
    IN GT_U32                                            numOfLpmMemories
)
{
    GT_U32 blockIndex=0; /* calculated according to the memory offset devided by block size including gap */
    GT_U32 numOfIncUpdatesToCounter=0;/* counter to be used for incrementing "protocol to block" usage*/
    GT_U32 numOfDecUpdatesToCounter=0;/* counter to be used for decrementing "protocol to block" usage*/
    GT_U32 numOfLinesInBlock;

    /* update the block usage counters */
    for (blockIndex = 0; blockIndex < numOfLpmMemories; blockIndex++)
    {
        numOfLinesInBlock  = lpmRamBlocksSizeArrayPtr[blockIndex];

        if((totalPendingBlockToUpdateArr[blockIndex].updateInc==GT_TRUE)||
           (totalPendingBlockToUpdateArr[blockIndex].updateDec==GT_TRUE))
        {
            numOfIncUpdatesToCounter = totalPendingBlockToUpdateArr[blockIndex].numOfIncUpdates;
            numOfDecUpdatesToCounter = totalPendingBlockToUpdateArr[blockIndex].numOfDecUpdates;
            switch (protocol)
            {
                case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
                if (((protocolCountersPerBlockArr[blockIndex].sumOfIpv4Counters +
                         numOfIncUpdatesToCounter)<numOfDecUpdatesToCounter) ||
                    (((protocolCountersPerBlockArr[blockIndex].sumOfIpv4Counters +
                         numOfIncUpdatesToCounter)-numOfDecUpdatesToCounter)>numOfLinesInBlock))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                protocolCountersPerBlockArr[blockIndex].sumOfIpv4Counters +=
                        numOfIncUpdatesToCounter - numOfDecUpdatesToCounter ;
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
                if (((protocolCountersPerBlockArr[blockIndex].sumOfIpv6Counters +
                         numOfIncUpdatesToCounter)<numOfDecUpdatesToCounter) ||
                    (((protocolCountersPerBlockArr[blockIndex].sumOfIpv6Counters +
                         numOfIncUpdatesToCounter)-numOfDecUpdatesToCounter)>numOfLinesInBlock))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                protocolCountersPerBlockArr[blockIndex].sumOfIpv6Counters +=
                        numOfIncUpdatesToCounter - numOfDecUpdatesToCounter ;
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
                if (((protocolCountersPerBlockArr[blockIndex].sumOfFcoeCounters +
                         numOfIncUpdatesToCounter)<numOfDecUpdatesToCounter) ||
                    (((protocolCountersPerBlockArr[blockIndex].sumOfFcoeCounters +
                         numOfIncUpdatesToCounter)-numOfDecUpdatesToCounter)>numOfLinesInBlock))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                protocolCountersPerBlockArr[blockIndex].sumOfFcoeCounters +=
                        numOfIncUpdatesToCounter - numOfDecUpdatesToCounter ;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

           /* reset pending for future use */
           resetPendingBlockToUpdateArr[blockIndex].updateInc=GT_FALSE;
           resetPendingBlockToUpdateArr[blockIndex].numOfIncUpdates=0;
           resetPendingBlockToUpdateArr[blockIndex].updateDec=GT_FALSE;
           resetPendingBlockToUpdateArr[blockIndex].numOfDecUpdates=0;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixBulkAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Creates a new or override an existing bulk of IPv4 prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] ipv4PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv4PrefixArrayPtr       - The UC prefix array.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if all prefixes were successfully added
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if adding one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixBulkAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *ipv4PrefixArrayPtr,
    IN GT_BOOL                              defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK,retVal1=GT_OK,retVal2=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    GT_U32                                              tempGlobalMemoryBlockTakenArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));
    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));
    cpssOsMemSet(tempGlobalMemoryBlockTakenArr,0,sizeof(tempGlobalMemoryBlockTakenArr));

    for (i = 0 ; i < ipv4PrefixArrayLen; i++)
    {
        if (ipv4PrefixArrayPtr[i].prefixLen > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if ((ipv4PrefixArrayPtr[i].ipAddr.arIP[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS) &&
            (ipv4PrefixArrayPtr[i].ipAddr.arIP[0] <= PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS))
        {
            /* Multicast range */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
        cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));

        /* first go over the prefixes and update the shadow */
        for (i = 0 ; i < ipv4PrefixArrayLen ; i++)
        {
            prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(shadowPtr->shadowType,
                                                             &(ipv4PrefixArrayPtr[i].nextHopInfo),
                                                             &lpmRouteEntry);
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,&(lpmRouteEntry.routeEntry));
            if (retVal != GT_OK)
            {
                return retVal;
            }
            if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
            {
                ipv4PrefixArrayPtr[i].returnStatus = prvCpssDxChLpmSip6RamUcEntryAdd(ipv4PrefixArrayPtr[i].vrId,
                                                                                     ipv4PrefixArrayPtr[i].ipAddr.arIP,
                                                                                     ipv4PrefixArrayPtr[i].prefixLen,
                                                                                     &lpmRouteEntry.routeEntry,
                                                                                     PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E,
                                                                                     ipv4PrefixArrayPtr[i].override,
                                                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                                                     shadowPtr,
                                                                                     defragmentationEnable);
            }
            else
            {
                ipv4PrefixArrayPtr[i].returnStatus = prvCpssDxChLpmRamUcEntryAdd(ipv4PrefixArrayPtr[i].vrId,
                                                                                ipv4PrefixArrayPtr[i].ipAddr.arIP,
                                                                                ipv4PrefixArrayPtr[i].prefixLen,
                                                                                &lpmRouteEntry.routeEntry,
                                                                                PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E,
                                                                                ipv4PrefixArrayPtr[i].override,
                                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                                                shadowPtr,
                                                                                defragmentationEnable);
            }

            if (ipv4PrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                /* need to set the last valid state of globalMemoryBlockTakenArr for next add*/
                cpssOsMemCpy(shadowPtr->globalMemoryBlockTakenArr,tempGlobalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));
                retVal2 = ipv4PrefixArrayPtr[i].returnStatus;
            }
            else
            {
                retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    /* need to reset the array */
                    cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                    return retVal1;
                }
                else
                {
                    /* keep the last valid state of globalMemoryBlockTakenArr */
                    cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));
                }
            }
        }

        prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E, &protocolBitmap);
#if 0
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP5_SHADOW_E)
        {
            /* for sip5 the assumption is that if ipv4Uc is supported the FCoE is supported */
            PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_SET_MAC(protocolBitmap);
        }
#endif
        /* now update the HW */
        retVal1 = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E,
                                    protocolBitmap,
                                    shadowPtr);
        if (retVal1 != GT_OK)
        {
            /* reset pending array for next use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            /* need to reset the array */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            return (retVal1);
        }
        else
        {
            retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr);
            if (retVal1!=GT_OK)
            {
                /* need to reset the array */
                cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                return retVal1;
            }

            retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                tempPendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal1!=GT_OK)
            {
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                /* need to reset the array */
                cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                return retVal1;
            }
        }
        /* after bulk end we need to reset the array */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return (retVal2);
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Deletes an existing IPv4 prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB.
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address of the prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NOT_FOUND             - If the given prefix doesn't exitst in the VR, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note the default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId,
    IN GT_IPADDR                             ipAddr,
    IN GT_U32                                prefixLen
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmRamSip6UcEntryDel(vrId,
                                                     ipAddr.arIP,
                                                     prefixLen,
                                                     GT_TRUE,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                     shadowPtr,
                                                     GT_FALSE);
        }
        else
        {
            retVal = prvCpssDxChLpmRamUcEntryDel(vrId,
                                                 ipAddr.arIP,
                                                 prefixLen,
                                                 GT_TRUE,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr,
                                                 GT_FALSE);
        }

        if (retVal != GT_OK)
        {
            break;
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixBulkDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Deletes an existing bulk of Ipv4 prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] ipv4PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv4PrefixArrayPtr       - The UC prefix array.
*
* @retval GT_OK                    - if all prefixes were successfully deleted
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_FAIL                  - if deleting one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixBulkDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *ipv4PrefixArrayPtr
)
{
    GT_STATUS           retVal = GT_OK,retVal1;
    GT_U32      i;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,
                     sizeof(shadowPtr->pendingBlockToUpdateArr));

        for (i = 0 ; i < ipv4PrefixArrayLen ; i++)
        {
            if (ipv4PrefixArrayPtr[i].prefixLen > 32)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            if ((ipv4PrefixArrayPtr[i].ipAddr.arIP[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS) &&
                (ipv4PrefixArrayPtr[i].ipAddr.arIP[0] <= PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS))
            {
                /* Multicast range */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
            {
                ipv4PrefixArrayPtr[i].returnStatus =
                     prvCpssDxChLpmRamSip6UcEntryDel(ipv4PrefixArrayPtr[i].vrId,
                                                     ipv4PrefixArrayPtr[i].ipAddr.arIP,
                                                     ipv4PrefixArrayPtr[i].prefixLen,
                                                     GT_TRUE,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                     shadowPtr,
                                                     GT_FALSE);
            }
            else
            {
                ipv4PrefixArrayPtr[i].returnStatus =
                    prvCpssDxChLpmRamUcEntryDel(ipv4PrefixArrayPtr[i].vrId,
                                                ipv4PrefixArrayPtr[i].ipAddr.arIP,
                                                ipv4PrefixArrayPtr[i].prefixLen,
                                                GT_FALSE,
                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                shadowPtr,
                                                GT_FALSE);
            }

            if (ipv4PrefixArrayPtr[i].returnStatus != GT_OK)
            {
               /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                retVal = ipv4PrefixArrayPtr[i].returnStatus;
            }
            else
            {
                if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
                {
                    retVal1=prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr);
                    if (retVal1!=GT_OK)
                    {
                        return retVal1;
                    }
                }
            }
        }
        if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E, &protocolBitmap);

            /* now update the HW */
            retVal1 = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_AND_ALLOC_E,
                                        protocolBitmap,
                                        shadowPtr);
            if (retVal1 != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                return (retVal1);
            }
            else
            {
                retVal1=prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    return retVal1;
                }
                retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                    tempPendingBlockToUpdateArr,
                                                                    shadowPtr->protocolCountersPerBlockArr,
                                                                    shadowPtr->pendingBlockToUpdateArr,
                                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                                    shadowPtr->numOfLpmMemories);
                if (retVal1!=GT_OK)
                {
                    /* reset pending array for future use */
                    cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                    return retVal1;
                }
            }
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixesFlush function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Flushes the unicast IPv4 Routing table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixesFlush
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRamUcEntriesFlush(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixSearch function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function searches for a given ip-uc address, and returns the next
*         hop pointer information bound to it if found.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - If found this is The next hop pointer to for this
*                                      prefix.
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixSearch
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPADDR                                   ipAddr,
    IN  GT_U32                                      prefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntrySearch(vrId,ipAddr.arIP,prefixLen,
                                            nextHopInfoPtr,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                            shadowPtr);

    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixGetNext function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns an IP-Unicast prefix with larger (ipAddrPtr,prefixLenPtr)
*         than the given one, it used for iterating over the existing prefixes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB.
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] ipAddrPtr                - The ip address to start the search from.
* @param[in,out] prefixLenPtr             - Prefix length of ipAddr.
* @param[in,out] ipAddrPtr                - The ip address of the found entry.
* @param[in,out] prefixLenPtr             - The prefix length of the found entry.
*
* @param[out] nextHopInfoPtr           - the next hop pointer associated with the found
*                                      ipAddr.
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if no more entries are left in the IP table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The values of (ipAddrPtr,prefixLenPtr) must be a valid values, it
*       means that they exist in the IP-UC Table, unless this is the first
*       call to this function, then the value of (ipAddrPtr,prefixLenPtr) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the Ipv4
*       UC prefix get function.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixGetNext
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC          *lpmDbPtr,
    IN    GT_U32                                        vrId,
    INOUT GT_IPADDR                                     *ipAddrPtr,
    INOUT GT_U32                                        *prefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC     *nextHopInfoPtr
)
{
    GT_STATUS                                   retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *tmpNextHopInfoPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryGet(vrId,ipAddrPtr->arIP,prefixLenPtr,
                                         &tmpNextHopInfoPtr,
                                         PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                         shadowPtr);
    if (retVal == GT_OK)
    {
        cpssOsMemCpy(nextHopInfoPtr,tmpNextHopInfoPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function gets a given ip address, and returns the next
*         hop pointer information bounded to the longest prefix match.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
*
* @param[out] prefixLenPtr             - Points to the number of bits that are actual valid
*                                      in the longest match
* @param[out] nextHopPointerPtr        - The next hop pointer bounded to the longest match
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPADDR                                   ipAddr,
    OUT GT_U32                                      *prefixLenPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopPointerPtr
)
{
    GT_STATUS                           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
       return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryLpmSearch(vrId,ipAddr.arIP,prefixLenPtr,
                                               nextHopPointerPtr,
                                               PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                               shadowPtr);
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4McEntryAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         To add the multicast routing information for IP datagrams from
*         a particular source and addressed to a particular IP multicast
*         group address for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
* @param[in] mcRouteEntryPtr          - the mc Route pointer to set for the mc entry.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_ERROR                 - if the virtual router does not exist.
* @retval GT_NOT_FOUND             - prefix was not found when override is GT_TRUE
* @retval GT_ALREADY_EXIST         - prefix already exist when override is GT_FALSE
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To override the default mc route use ipGroup = 0.
*       2. If (S,G) MC group is added when (,G) doesn't exists then implicit
*       (,G) is added pointing to (,) default. Application added (,G)
*       will override the implicit (,G).
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntryAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN GT_U32                                       vrId,
    IN GT_IPADDR                                    ipGroup,
    IN GT_U32                                       ipGroupPrefixLen,
    IN GT_IPADDR                                    ipSrc,
    IN GT_U32                                       ipSrcPrefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *mcRouteEntryPtr,
    IN GT_BOOL                                      override,
    IN GT_BOOL                                      defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;
    GT_BOOL                         managerHwWriteBlock;
    /* do special check for link local*/


    if ((ipGroupPrefixLen > 4) && (ipGroupPrefixLen < 32))
    {
        /* in this case all except link local must be rejected */
        PRV_CPSS_DXCH_LPM_IPV4_MC_LINK_LOCAL_CHECK_MAC(ipGroup, ipGroupPrefixLen, ipSrcPrefixLen);
    }

    if ((ipGroupPrefixLen <= 4) && (ipSrcPrefixLen > 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* link local exact mach must be also rejected */
    PRV_CPSS_DXCH_LPM_IPV4_MC_LINK_LOCAL_CHECK_EXACT_MATCH_MAC(ipGroup, ipGroupPrefixLen);

    /* if initialization has not been done for the requested protocol stack - return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,mcRouteEntryPtr);
        if (retVal != GT_OK)
            return retVal;

        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
           (managerHwWriteBlock == GT_TRUE))
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E;
        }
        else
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
        }
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamMcEntryAdd(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                     ipSrc.arIP,ipSrcPrefixLen,
                                                     mcRouteEntryPtr,
                                                     override,
                                                     insertMode,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                     shadowPtr,
                                                     defragmentationEnable);
        }
        else
        {
            retVal = prvCpssDxChLpmRamMcEntryAdd(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                 ipSrc.arIP,ipSrcPrefixLen,
                                                 mcRouteEntryPtr,
                                                 override,
                                                 insertMode,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr,
                                                 defragmentationEnable);
        }
        if (retVal != GT_OK)
        {
            /* need to reset the array */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            break;
        }
        /* reset the array for next use */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4McEntryDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         To delete a particular mc route entry for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. In order to delete the multicast entry and all the src ip addresses
*       associated with it, call this function with ipSrc = ipSrcPrefixLen = 0.
*       2. If no (,G) was added but (S,G) were added, then implicit (,G)
*       that points to (,) is added. If (,G) is added later, it will
*       replace the implicit (,G).
*       When deleting (,G), then if there are still (S,G), an implicit (,G)
*       pointing to (,) will be added.
*       When deleting last (S,G) and the (,G) was implicitly added, then
*       the (,G) will be deleted as well.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntryDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId,
    IN GT_IPADDR                            ipGroup,
    IN GT_U32                               ipGroupPrefixLen,
    IN GT_IPADDR                            ipSrc,
    IN GT_U32                               ipSrcPrefixLen
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamMcEntryDelete(vrId, ipGroup.arIP,ipGroupPrefixLen,
                                                        ipSrc.arIP,ipSrcPrefixLen,
                                                        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                        shadowPtr,GT_FALSE,NULL);
        }
        else
        {
            retVal = prvCpssDxChLpmRamMcEntryDelete(vrId, ipGroup.arIP,ipGroupPrefixLen,
                                                    ipSrc.arIP,ipSrcPrefixLen,
                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                    shadowPtr,GT_FALSE,NULL);
        }
        if (retVal != GT_OK)
        {
            break;
        }

    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4McEntriesFlush function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntriesFlush
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRamMcEntriesFlush(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4McEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the next entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrc                    - The ip Source address to get the next entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteEntryPtr          - the mc route entry ptr of the found mc mc route
*
* @retval GT_OK                    - if found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device.
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntryGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPADDR                                   ipGroup,
    IN  GT_U32                                      ipGroupPrefixLen,
    IN  GT_IPADDR                                   ipSrc,
    IN  GT_U32                                      ipSrcPrefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *mcRouteEntryPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lpmDbPtr->shadowArray->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* it's enough to look at the first shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamMcEntrySearch(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                            ipSrc.arIP,ipSrcPrefixLen,
                                            mcRouteEntryPtr,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                            shadowPtr);
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4McEntryGetNext function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns the next muticast (ipSrcPtr,ipGroupPtr) entry, used
*         to iterate over the existing multicast addresses for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id.
* @param[in,out] ipGroupPtr               - The ip Group address to get the next entry for.
* @param[in,out] ipGroupPrefixLenPtr      - ipGroupPtr prefix length.
* @param[in,out] ipSrcPtr                 - The ip Source address to get the next entry for.
* @param[in,out] ipSrcPrefixLenPtr        - ipSrcPtr prefix length.
* @param[in,out] ipGroupPtr               - The next ip Group address.
* @param[in,out] ipGroupPrefixLenPtr      - ipGroupPtr prefix length.
* @param[in,out] ipSrcPtr                 - The next ip Source address.
* @param[in,out] ipSrcPrefixLenPtr        - ipSrc prefix length.
*
* @param[out] mcRouteEntryPtr          - the mc route entry ptr of the found mc mc route
*
* @retval GT_OK                    - if found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device.
*
* @note 1. The values of (ipSrcPtr,ipGroupPtr) must be a valid values, it
*       means that they exist in the IP-Mc Table, unless this is the first
*       call to this function, then the value of (ipSrcPtr,ipGroupPtr) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the Ipv4
*       MC get function.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntryGetNext
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_IPADDR                                 *ipGroupPtr,
    INOUT GT_U32                                    *ipGroupPrefixLenPtr,
    INOUT GT_IPADDR                                 *ipSrcPtr,
    INOUT GT_U32                                    *ipSrcPrefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *mcRouteEntryPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lpmDbPtr->shadowArray->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* it's enough to look at the first shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamMcEntryGetNext(vrId,ipGroupPtr->arIP,ipGroupPrefixLenPtr,
                                             ipSrcPtr->arIP,
                                             ipSrcPrefixLenPtr,
                                             mcRouteEntryPtr,
                                             PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                             shadowPtr);
    /* group prefix 0 means this is the default entry (*,*) */
    if (*ipGroupPrefixLenPtr == 0)
    {
        ipGroupPtr->u32Ip = 0;
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         creates a new or override an existing Ipv6 prefix in a Virtual Router
*         for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - Points to the destination IP address of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
* @param[in] nextHopInfoPtr           - Points to the next hop pointer to set for this prefix.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the vrId was not created yet, or
* @retval GT_ALREADY_EXIST         - if prefix already exist when override is GT_FALSE, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN GT_U32                                       vrId,
    IN GT_IPV6ADDR                                  ipAddr,
    IN GT_U32                                       prefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *nextHopInfoPtr,
    IN GT_BOOL                                      override,
    IN GT_BOOL                                      defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;
    GT_BOOL                         managerHwWriteBlock;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,nextHopInfoPtr);
        if (retVal != GT_OK)
            return retVal;

       retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
           (managerHwWriteBlock == GT_TRUE))
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E;
        }
        else
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
        }
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamUcEntryAdd(vrId,
                                                     ipAddr.arIP,
                                                     prefixLen,
                                                     nextHopInfoPtr,
                                                     insertMode,
                                                     override,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                     shadowPtr,
                                                     defragmentationEnable);
        }
        else
        {
            retVal = prvCpssDxChLpmRamUcEntryAdd(vrId,
                                             ipAddr.arIP,
                                             prefixLen,
                                             nextHopInfoPtr,
                                             insertMode,
                                             override,
                                             PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                             shadowPtr,
                                             defragmentationEnable);
        }

        if (retVal != GT_OK)
        {
            /* reset the array for next use */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            break;
        }
        /* reset the array for next use */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixBulkAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         creates a new or override an existing bulk of Ipv6 prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] ipv6PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv6PrefixArrayPtr       - The UC prefix array.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if all prefixes were successfully added
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if adding one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixBulkAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC      *ipv6PrefixArrayPtr,
    IN GT_BOOL                                  defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK,retVal1=GT_OK,retVal2=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    GT_U32                                              tempGlobalMemoryBlockTakenArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));
    cpssOsMemSet(tempGlobalMemoryBlockTakenArr,0,sizeof(tempGlobalMemoryBlockTakenArr));

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));

    for (i = 0 ; i < ipv6PrefixArrayLen ; i++)
    {
        if (ipv6PrefixArrayPtr[i].prefixLen > 128)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if (ipv6PrefixArrayPtr[i].ipAddr.arIP[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS)
        {
            /* Multicast range */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,sizeof(shadowPtr->pendingBlockToUpdateArr));
        cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));

        /* first go over the prefixes and update the shadow */
        for (i = 0 ; i < ipv6PrefixArrayLen ; i++)
        {
            prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(shadowPtr->shadowType,
                                                             &(ipv6PrefixArrayPtr[i].nextHopInfo),
                                                             &lpmRouteEntry);
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,&(lpmRouteEntry.routeEntry));
            if (retVal != GT_OK)
            {
                return retVal;
            }

            if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
            {
                ipv6PrefixArrayPtr[i].returnStatus =
                    prvCpssDxChLpmSip6RamUcEntryAdd(ipv6PrefixArrayPtr[i].vrId,
                                                    ipv6PrefixArrayPtr[i].ipAddr.arIP,
                                                    ipv6PrefixArrayPtr[i].prefixLen,
                                                    &lpmRouteEntry.routeEntry,
                                                    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E,
                                                    ipv6PrefixArrayPtr[i].override,
                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                    shadowPtr,
                                                    defragmentationEnable);
            }
            else
            {
                ipv6PrefixArrayPtr[i].returnStatus =
                    prvCpssDxChLpmRamUcEntryAdd(ipv6PrefixArrayPtr[i].vrId,
                                                ipv6PrefixArrayPtr[i].ipAddr.arIP,
                                                ipv6PrefixArrayPtr[i].prefixLen,
                                                &lpmRouteEntry.routeEntry,
                                                PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E,
                                                ipv6PrefixArrayPtr[i].override,
                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                shadowPtr,
                                                defragmentationEnable);
            }

            if (ipv6PrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                /* need to set the last valid state of globalMemoryBlockTakenArr for next add*/
                cpssOsMemCpy(shadowPtr->globalMemoryBlockTakenArr,tempGlobalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));
                retVal2 = ipv6PrefixArrayPtr[i].returnStatus;
            }
            else
            {
                retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,
                                                                                            shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    return retVal1;
                }
                else
                {
                    /* keep the last valid state of globalMemoryBlockTakenArr */
                    cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));
                }
            }
        }

        prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E, &protocolBitmap);
        /* now update the HW */
        retVal1 = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E,
                                    protocolBitmap,
                                    shadowPtr);
        if (retVal1 != GT_OK)
        {
            /* reset pending array for next use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            /* need to reset the array */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            return (retVal1);
        }
        else
        {
            retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,
                                                                                        shadowPtr->pendingBlockToUpdateArr);
            if (retVal1!=GT_OK)
            {
                /* need to reset the array */
                cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                return retVal1;
            }
            retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                tempPendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal1!=GT_OK)
            {
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                /* need to reset the array */
                cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                return retVal1;
            }
        }
        /* after bulk end we need to reset the array */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return (retVal2);
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         deletes an existing Ipv6 prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - Points to the destination IP address of the prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the vrId was not created yet, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given prefix doesn't exitst in the VR, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
* @note The default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId,
    IN GT_IPV6ADDR                          ipAddr,
    IN GT_U32                               prefixLen
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmRamSip6UcEntryDel(vrId,
                                                     ipAddr.arIP,
                                                     prefixLen,
                                                     GT_TRUE,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                     shadowPtr,
                                                     GT_FALSE);
        }
        else
        {
            retVal = prvCpssDxChLpmRamUcEntryDel(vrId,
                                                 ipAddr.arIP,
                                                 prefixLen,
                                                 GT_TRUE,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr,
                                                 GT_FALSE);
        }
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixBulkDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         deletes an existing bulk of Ipv6 prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] ipv6PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv6PrefixArrayPtr       - The UC prefix array.
*
* @retval GT_OK                    - if all prefixes were successfully deleted
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if deleting one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixBulkDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC      *ipv6PrefixArrayPtr
)
{
    GT_STATUS           retVal = GT_OK,retVal1;
    GT_U32      i;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    if(ipv6PrefixArrayLen == 0)
        return GT_OK;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,
                     sizeof(tempPendingBlockToUpdateArr));

        for (i = 0 ; i < ipv6PrefixArrayLen ; i++)
        {
            if (ipv6PrefixArrayPtr[i].prefixLen > 128)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            if (ipv6PrefixArrayPtr[i].ipAddr.arIP[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS)
            {
                /* Multicast range */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
            {
                ipv6PrefixArrayPtr[i].returnStatus =
                     prvCpssDxChLpmRamSip6UcEntryDel(ipv6PrefixArrayPtr[i].vrId,
                                                     ipv6PrefixArrayPtr[i].ipAddr.arIP,
                                                     ipv6PrefixArrayPtr[i].prefixLen,
                                                     GT_TRUE,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                     shadowPtr,
                                                     GT_FALSE);
            }
            else
            {
                ipv6PrefixArrayPtr[i].returnStatus =
                    prvCpssDxChLpmRamUcEntryDel(ipv6PrefixArrayPtr[i].vrId,
                                                ipv6PrefixArrayPtr[i].ipAddr.arIP,
                                                ipv6PrefixArrayPtr[i].prefixLen,
                                                GT_FALSE,
                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                shadowPtr,
                                                GT_FALSE);
            }

            if (ipv6PrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                retVal = ipv6PrefixArrayPtr[i].returnStatus;
            }
            else
            {
                if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
                {
                    retVal1=prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,
                                                                                              shadowPtr->pendingBlockToUpdateArr);
                    if (retVal1!=GT_OK)
                    {
                        return retVal1;
                    }
                }
            }
        }
        if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E, &protocolBitmap);
            /* now update the HW */
            retVal1 = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_AND_ALLOC_E,
                                        protocolBitmap,
                                        shadowPtr);
            if (retVal1 != GT_OK)
            {
                /* reset pending array for next use */
                    cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                return (retVal1);
            }
            else
            {
                retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,
                                                                                            shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    return retVal1;
                }

                retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                    tempPendingBlockToUpdateArr,
                                                                    shadowPtr->protocolCountersPerBlockArr,
                                                                    shadowPtr->pendingBlockToUpdateArr,
                                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                                    shadowPtr->numOfLpmMemories);
                if (retVal1!=GT_OK)
                {
                    /* reset pending array for future use */
                    cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                    return retVal1;
                }
            }
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixesFlush function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         flushes the unicast IPv6 Routing table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixesFlush
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRamUcEntriesFlush(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixSearch function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function searches for a given ip-uc address, and returns the next
*         hop pointer information bound to it if found.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - Points to the destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - If found this is The next hop pointer to for this
*                                      prefix.
*
* @retval GT_OK                    - if the required entry was found
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixSearch
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPV6ADDR                                 ipAddr,
    IN  GT_U32                                      prefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamUcEntrySearch(vrId,ipAddr.arIP,prefixLen,
                                            nextHopInfoPtr,
                                          PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                            shadowPtr);
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixGetNext function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns an IP-Unicast prefix with larger (ipAddrPtr,prefixLenPtr)
*         than the given one, it used for iterating over the existing prefixes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] ipAddrPtr                - The ip address to start the search from.
* @param[in,out] prefixLenPtr             - Prefix length of ipAddr.
* @param[in,out] ipAddrPtr                - The ip address of the found entry.
* @param[in,out] prefixLenPtr             - The prefix length of the found entry.
*
* @param[out] nextHopInfoPtr           - the next hop pointer associated with the found
*                                      ipAddr.
*
* @retval GT_OK                    - if the required entry was found
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if no more entries are left in the IP table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The values of (ipAddrPtr,prefixLenPtr) must be a valid values, it
*       means that they exist in the IP-UC Table, unless this is the first
*       call to this function, then the value of (ipAddrPtr,prefixLenPtr) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the Ipv6
*       UC prefix get function.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixGetNext
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_IPV6ADDR                               *ipAddrPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopInfoPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *tmpNextHopInfoPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryGet(vrId,ipAddrPtr->arIP,prefixLenPtr,
                                         &tmpNextHopInfoPtr,
                                         PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                         shadowPtr);
    if (retVal == GT_OK)
    {
        cpssOsMemCpy(nextHopInfoPtr,tmpNextHopInfoPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function gets a given ip address, and returns the next
*         hop pointer information bounded to the longest prefix match.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
*
* @param[out] prefixLenPtr             - Points to the number of bits that are actual valid
*                                      in the longest match
* @param[out] nextHopInfoPtr           - The next hop pointer bounded to the longest match
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPV6ADDR                                 ipAddr,
    OUT GT_U32                                      *prefixLenPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
)
{
    GT_STATUS                       retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC   *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
       return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamUcEntryLpmSearch(vrId,ipAddr.arIP,prefixLenPtr,
                                               nextHopInfoPtr,
                                               PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                               shadowPtr);
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6McEntryAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         To add the multicast routing information for IP datagrams from
*         a particular source and addressed to a particular IP multicast
*         group address for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
* @param[in] mcRouteEntryPtr          - (pointer to) the mc Route pointer to set for the mc entry.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if prefix was not found when override is GT_TRUE, or
* @retval GT_ALREADY_EXIST         - if prefix already exist when override is GT_FALSE, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
* @note 1. To override the default mc route use ipGroup = 0.
*       2. If (S,G) MC group is added when (,G) doesn't exists then implicit
*       (,G) is added pointing to (,) default. Application added (,G)
*       will override the implicit (,G).
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntryAdd
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN GT_U32                                       vrId,
    IN GT_IPV6ADDR                                  ipGroup,
    IN GT_U32                                       ipGroupPrefixLen,
    IN GT_IPV6ADDR                                  ipSrc,
    IN GT_U32                                       ipSrcPrefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *mcRouteEntryPtr,
    IN GT_BOOL                                      override,
    IN GT_BOOL                                      defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;
    GT_BOOL                         managerHwWriteBlock;

    if ((ipGroupPrefixLen > 8) && (ipGroupPrefixLen < 128))
    {
        /* in this case all except link local must be rejected */
        PRV_CPSS_DXCH_LPM_IPV6_MC_LINK_LOCAL_CHECK_MAC(ipGroup, ipGroupPrefixLen, ipSrcPrefixLen);
    }

     if ((ipGroupPrefixLen <= 8) && (ipSrcPrefixLen > 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
     /* link local exact mach must be also rejected */
     PRV_CPSS_DXCH_LPM_IPV6_MC_LINK_LOCAL_CHECK_EXACT_MATCH_MAC(ipGroup, ipGroupPrefixLen);

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,mcRouteEntryPtr);
        if (retVal != GT_OK)
            return retVal;

        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }

        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
           (managerHwWriteBlock == GT_TRUE))
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E;
        }
        else
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
        }
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamMcEntryAdd(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                     ipSrc.arIP,ipSrcPrefixLen,
                                                     mcRouteEntryPtr,
                                                     override,
                                                     insertMode,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                     shadowPtr,
                                                     defragmentationEnable);

        }
        else
        {
            retVal = prvCpssDxChLpmRamMcEntryAdd(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                 ipSrc.arIP,ipSrcPrefixLen,
                                                 mcRouteEntryPtr,
                                                 override,
                                                 insertMode,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr,
                                                 defragmentationEnable);

        }
        /* reset the array for next use */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6McEntryDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         To delete a particular mc route entry for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
* @note 1. Inorder to delete the multicast entry and all the src ip addresses
*       associated with it, call this function with ipSrc = ipSrcPrefixLen = 0.
*       2. If no (,G) was added but (S,G) were added, then implicit (,G)
*       that points to (,) is added. If (,G) is added later, it will
*       replace the implicit (,G).
*       When deleting (,G), then if there are still (S,G), an implicit (,G)
*       pointing to (,) will be added.
*       When deleting last (S,G) and the (,G) was implicitly added, then
*       the (,G) will be deleted as well.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntryDel
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId,
    IN GT_IPV6ADDR                          ipGroup,
    IN GT_U32                               ipGroupPrefixLen,
    IN GT_IPV6ADDR                          ipSrc,
    IN GT_U32                               ipSrcPrefixLen
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmSip6RamMcEntryDelete(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                        ipSrc.arIP,ipSrcPrefixLen,
                                                        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                        shadowPtr,GT_FALSE,NULL);
        }
        else
        {
            retVal = prvCpssDxChLpmRamMcEntryDelete(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                    ipSrc.arIP,ipSrcPrefixLen,
                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                    shadowPtr,GT_FALSE,NULL);
        }

        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6McEntriesFlush function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntriesFlush
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        retVal = prvCpssDxChLpmRamMcEntriesFlush(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6McEntryGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the next entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrc                    - The ip Source address to get the next entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteEntryPtr          - (pointer to) the mc route entry ptr of the found mc mc route
*
* @retval GT_OK                    - if found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device.
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntryGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPV6ADDR                                 ipGroup,
    IN  GT_U32                                      ipGroupPrefixLen,
    IN  GT_IPV6ADDR                                 ipSrc,
    IN  GT_U32                                      ipSrcPrefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *mcRouteEntryPtr
)
{
    GT_STATUS                               retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lpmDbPtr->shadowArray->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* it's enough to look at the first shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamMcEntrySearch(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                            ipSrc.arIP,ipSrcPrefixLen,
                                            mcRouteEntryPtr,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                            shadowPtr);
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6McEntryGetNext function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns the next muticast (ipSrcPtr,ipGroupPtr) entry, used
*         to iterate over the existing multicast addresses for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id.
* @param[in,out] ipGroupPtr               - (pointer to) The ip Group address to get the next entry for.
* @param[in,out] ipGroupPrefixLenPtr      - (pointer to) the ipGroupPtr prefix length.
* @param[in,out] ipSrcPtr                 - (pointer to) The ip Source address to get the next entry for.
* @param[in,out] ipSrcPrefixLenPtr        - (pointer to) ipSrcPtr prefix length.
* @param[in,out] ipGroupPtr               - (pointer to) The next ip Group address.
* @param[in,out] ipGroupPrefixLenPtr      - (pointer to) the ipGroupPtr prefix length.
* @param[in,out] ipSrcPtr                 - (pointer to) The next ip Source address.
* @param[in,out] ipSrcPrefixLenPtr        - (pointer to) ipSrcPtr prefix length.
*
* @param[out] mcRouteEntryPtr          - (pointer to) the mc route entry ptr of the found mc mc route
*
* @retval GT_OK                    - if found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
*
* @note 1. The values of (ipSrcPtr,ipGroupPtr) must be a valid values, it
*       means that they exist in the IP-Mc Table, unless this is the first
*       call to this function, then the value of (ipSrcPtr,ipGroupPtr) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the Ipv6
*       MC get function.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntryGetNext
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_IPV6ADDR                               *ipGroupPtr,
    INOUT GT_U32                                    *ipGroupPrefixLenPtr,
    INOUT GT_IPV6ADDR                               *ipSrcPtr,
    INOUT GT_U32                                    *ipSrcPrefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *mcRouteEntryPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lpmDbPtr->shadowArray->vrRootBucketArray[vrId].valid == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    /* it's enough to look at the first shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamMcEntryGetNext(vrId,ipGroupPtr->arIP,ipGroupPrefixLenPtr,
                                             ipSrcPtr->arIP,
                                             ipSrcPrefixLenPtr,
                                             mcRouteEntryPtr,
                                             PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                             shadowPtr);
    /* group prefix 0 means this is the default entry (*,*) */
    if (*ipGroupPrefixLenPtr == 0)
    {
        cpssOsMemSet(ipGroupPtr->u32Ip,0,sizeof(GT_U32)*4);
    }
    return (retVal);
}


/**
* @internal prvCpssDxChLpmRamFcoePrefixAdd function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function adds a new FCoE prefix to a Virtual Router in a
*         specific LPM DB or overrides an existing existing FCoE prefix.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] fcoeAddr                 - The FC_ID of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the FC_ID.
* @param[in] nextHopInfoPtr           - (points to) The next hop pointer to set for this prefix.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_ALREADY_EXIST         - prefix already exist when override is GT_FALSE
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixAdd
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     fcoeAddr,
    IN  GT_U32                                      prefixLen,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr,
    IN  GT_BOOL                                     override,
    IN GT_BOOL                                      defragmentationEnable
)
{
    GT_STATUS                           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;
    GT_U32                              shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode;
    CPSS_SYSTEM_RECOVERY_INFO_STC       tempSystemRecovery_Info;
    GT_BOOL                             managerHwWriteBlock;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,nextHopInfoPtr);
        if (retVal != GT_OK)
            return retVal;

        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
            if (retVal != GT_OK)
            {
                return retVal;
            }
            managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
            if((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
               (managerHwWriteBlock == GT_TRUE))
            {
                insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E;
            }
            else
            {
                insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
            }
            retVal = prvCpssDxChLpmSip6RamUcEntryAdd(vrId,
                                                     &fcoeAddr.fcid[0],
                                                     prefixLen,
                                                     nextHopInfoPtr,
                                                     insertMode,
                                                     override,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                     shadowPtr,
                                                     defragmentationEnable);

        }
        else
        {
            retVal = prvCpssDxChLpmRamUcEntryAdd(vrId,
                                                 fcoeAddr.fcid,
                                                 prefixLen,
                                                 nextHopInfoPtr,
                                                 PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E,
                                                 override,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                 shadowPtr,
                                                 defragmentationEnable);
        }
        if (retVal != GT_OK)
        {
            /* reset the array for next use */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            break;
        }
        /* reset the array for next use */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixDel function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Deletes an existing FCoE prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB.
* @param[in] vrId                     - The virtual router id.
* @param[in] fcoeAddr                 - The destination FC_ID address of the prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the FC_ID.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NOT_FOUND             - If the given prefix doesn't exitst in the VR, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note the default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixDel
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC    *lpmDbPtr,
    IN  GT_U32                                  vrId,
    IN  GT_FCID                                 fcoeAddr,
    IN  GT_U32                                  prefixLen
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            retVal = prvCpssDxChLpmRamSip6UcEntryDel(vrId,
                                                     fcoeAddr.fcid,
                                                     prefixLen,
                                                     GT_TRUE,
                                                     PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                     shadowPtr,
                                                     GT_FALSE);
        }
        else
        {
            retVal = prvCpssDxChLpmRamUcEntryDel(vrId,
                                                 fcoeAddr.fcid,
                                                 prefixLen,
                                                 GT_TRUE,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                 shadowPtr,
                                                 GT_FALSE);
        }

        if (retVal != GT_OK)
        {
            break;
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixAddBulk function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Creates a new or override an existing bulk of FCoE prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] fcoeAddrPrefixArrayLen   - Length of FC_ID prefix array.
* @param[in] fcoeAddrPrefixArrayPtr   - The FC_ID prefix array.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if all prefixes were successfully added
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if adding one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixAddBulk
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC            *lpmDbPtr,
    IN  GT_U32                                          fcoeAddrPrefixArrayLen,
    IN  CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    *fcoeAddrPrefixArrayPtr,
    IN  GT_BOOL                                         defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK,retVal1=GT_OK,retVal2=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];
    GT_U32                                              tempGlobalMemoryBlockTakenArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));
    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));
    cpssOsMemSet(tempGlobalMemoryBlockTakenArr,0,sizeof(tempGlobalMemoryBlockTakenArr));

    for (i = 0 ; i < fcoeAddrPrefixArrayLen; i++)
    {
        if (fcoeAddrPrefixArrayPtr[i].prefixLen > 24)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
        cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));

        /* first go over the prefixes and update the shadow */
        for (i = 0 ; i < fcoeAddrPrefixArrayLen ; i++)
        {
            prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(shadowPtr->shadowType,
                                                             &(fcoeAddrPrefixArrayPtr[i].nextHopInfo),
                                                             &lpmRouteEntry);
            retVal = prvCpssDxChLpmRouteEntryPointerCheck(shadowPtr->shadowType,&(lpmRouteEntry.routeEntry));
            if (retVal != GT_OK)
            {
                return retVal;
            }
            if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
            {
                fcoeAddrPrefixArrayPtr[i].returnStatus =
                                prvCpssDxChLpmSip6RamUcEntryAdd(fcoeAddrPrefixArrayPtr[i].vrId,
                                                                fcoeAddrPrefixArrayPtr[i].fcoeAddr.fcid,
                                                                fcoeAddrPrefixArrayPtr[i].prefixLen,
                                                                &lpmRouteEntry.routeEntry,
                                                                PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E,
                                                                fcoeAddrPrefixArrayPtr[i].override,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                                shadowPtr,
                                                                defragmentationEnable);
            }
            else
            {
                fcoeAddrPrefixArrayPtr[i].returnStatus =
                                prvCpssDxChLpmRamUcEntryAdd(fcoeAddrPrefixArrayPtr[i].vrId,
                                                            fcoeAddrPrefixArrayPtr[i].fcoeAddr.fcid,
                                                            fcoeAddrPrefixArrayPtr[i].prefixLen,
                                                            &lpmRouteEntry.routeEntry,
                                                            PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E,
                                                            fcoeAddrPrefixArrayPtr[i].override,
                                                            PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                            shadowPtr,
                                                            defragmentationEnable);
            }

            if (fcoeAddrPrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                /* need to set the last valid state of globalMemoryBlockTakenArr for next add*/
                cpssOsMemCpy(shadowPtr->globalMemoryBlockTakenArr,tempGlobalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));
                retVal2 = fcoeAddrPrefixArrayPtr[i].returnStatus;
            }
            else
            {
                retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,
                                                                                            shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    /* need to reset the array */
                    cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                    return retVal1;
                }
                else
                {
                    /* keep the last valid state of globalMemoryBlockTakenArr */
                    cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));
                }
            }
        }

        prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(CPSS_IP_PROTOCOL_FCOE_E, &protocolBitmap);
        /* now update the HW */
        retVal1 = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E,
                                    protocolBitmap,
                                    shadowPtr);
        if (retVal1 != GT_OK)
        {
            /* reset pending array for next use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            /* need to reset the array */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            return (retVal1);
        }
        else
        {
            retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,
                                                                                        shadowPtr->pendingBlockToUpdateArr);
            if (retVal1!=GT_OK)
            {
                /* need to reset the array */
                cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                return retVal1;
            }

            retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                tempPendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal1!=GT_OK)
            {
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                /* need to reset the array */
                cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                return retVal1;
            }
        }
        /* after bulk end we need to reset the array */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return (retVal2);
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixDelBulk function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Deletes an existing bulk of FCoE prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] fcoeAddrPrefixArrayLen   - Length of FC_ID prefix array.
* @param[in] fcoeAddrPrefixArrayPtr   - The FC_ID prefix array.
*
* @retval GT_OK                    - if all prefixes were successfully deleted
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_FAIL                  - if deleting one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixDelBulk
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC            *lpmDbPtr,
    IN  GT_U32                                          fcoeAddrPrefixArrayLen,
    IN  CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    *fcoeAddrPrefixArrayPtr
)
{
    GT_STATUS           retVal = GT_OK,retVal1;
    GT_U32      i;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_FALCON_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,sizeof(shadowPtr->pendingBlockToUpdateArr));

        for (i = 0 ; i < fcoeAddrPrefixArrayLen ; i++)
        {
            if (fcoeAddrPrefixArrayPtr[i].prefixLen > 24)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            if (shadowPtr->shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
            {
                fcoeAddrPrefixArrayPtr[i].returnStatus =
                                    prvCpssDxChLpmRamSip6UcEntryDel(fcoeAddrPrefixArrayPtr[i].vrId,
                                                                    fcoeAddrPrefixArrayPtr[i].fcoeAddr.fcid,
                                                                    fcoeAddrPrefixArrayPtr[i].prefixLen,
                                                                    GT_FALSE,
                                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                                    shadowPtr,
                                                                    GT_FALSE);
            }
            else
            {
                fcoeAddrPrefixArrayPtr[i].returnStatus =
                                    prvCpssDxChLpmRamUcEntryDel(fcoeAddrPrefixArrayPtr[i].vrId,
                                                                fcoeAddrPrefixArrayPtr[i].fcoeAddr.fcid,
                                                                fcoeAddrPrefixArrayPtr[i].prefixLen,
                                                                GT_FALSE,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                                shadowPtr,
                                                                GT_FALSE);
            }

            if (fcoeAddrPrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                retVal = fcoeAddrPrefixArrayPtr[i].returnStatus;
            }
            else
            {
                if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
                {
                    retVal1=prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr);
                    if (retVal1!=GT_OK)
                    {
                        return retVal1;
                    }
                }
            }
        }

        if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
        {
            prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E, &protocolBitmap);

            /* now update the HW */
            retVal1 = updateHwSearchStc(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_AND_ALLOC_E,
                                        protocolBitmap,
                                        shadowPtr);
            if (retVal1 != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                return (retVal1);
            }
            else
            {
                retVal1=prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockList(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    return retVal1;
                }
                retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCounters(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                    tempPendingBlockToUpdateArr,
                                                                    shadowPtr->protocolCountersPerBlockArr,
                                                                    shadowPtr->pendingBlockToUpdateArr,
                                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                                    shadowPtr->numOfLpmMemories);
                if (retVal1!=GT_OK)
                {
                    /* reset pending array for future use */
                    cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                    return retVal1;
                }
            }
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixesFlush function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Flushes the FCoE forwarding table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixesFlush
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRamUcEntriesFlush(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixSearch function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function searches for a given FC_ID, and returns the next
*         hop pointer information bound to it if found.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] fcoeAddr                 - The FC_ID to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
*                                      FC_ID.
*
* @param[out] nextHopInfoPtr           - If found this is The next hop pointer to for this
*                                      prefix.
*
* @retval GT_OK                    - the required entry was found, or
* @retval GT_OUT_OF_RANGE          - the prefix length is too big, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - the given prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixSearch
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     fcoeAddr,
    IN  GT_U32                                      prefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntrySearch(vrId,fcoeAddr.fcid,prefixLen,
                                            nextHopInfoPtr,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                            shadowPtr);

    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixGetNext function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns an FCoE prefix with larger (FC_ID,prefix)
*         than the given one, it used for iterating over the existing prefixes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB.
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] fcoeAddrPtr              - The FC_ID to start the search from.
* @param[in,out] prefixLenPtr             - The number of bits that are actual valid in the
*                                      FC_ID.
* @param[in,out] fcoeAddrPtr              - The FC_ID of the found entry.
* @param[in,out] prefixLenPtr             - The prefix length of the found entry.
*
* @param[out] nextHopInfoPtr           - the next hop pointer associated with the found FC_ID
*
* @retval GT_OK                    - the required entry was found, or
* @retval GT_OUT_OF_RANGE          - the prefix length is too big, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - no more entries are left in the FC_ID table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The values of (FC_ID,prefix) must be a valid values, it
*       means that they exist in the forwarding Table, unless this is the
*       first call to this function, then the value of (FC_ID,prefix) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the FC_ID
*       prefix get function.
*
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixGetNext
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_FCID                                   *fcoeAddrPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopInfoPtr
)
{
    GT_STATUS                                   retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *tmpNextHopInfoPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryGet(vrId,fcoeAddrPtr->fcid,prefixLenPtr,
                                         &tmpNextHopInfoPtr,
                                         PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                         shadowPtr);
    if (retVal == GT_OK)
    {
        cpssOsMemCpy(nextHopInfoPtr,tmpNextHopInfoPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function gets a given FC_ID address, and returns the next
*         hop pointer information bounded to the longest prefix match.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] fcoeAddr                 - The FC_ID to look for.
*
* @param[out] prefixLenPtr             - Points to the number of bits that are actual valid
*                                      in the longest match
* @param[out] nextHopInfoPtr           - The next hop pointer bound to the longest match
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     fcoeAddr,
    OUT GT_U32                                      *prefixLenPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
)
{
    GT_STATUS                           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
       return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryLpmSearch(vrId,fcoeAddr.fcid,prefixLenPtr,
                                               nextHopInfoPtr,
                                               PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                               shadowPtr);
    return (retVal);
}

#if 0
/**
* @internal ipLpmDbGetL3Vr function
* @endinternal
*
* @brief   Retrives the valid virtual routers
*
* @param[in] ipShadowPtr              - points to the shadow to retrieve from
* @param[in,out] iterPtr                  - points to the current iteration
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes)
* @param[out] tablePtrPtr              - the table size info block
* @param[in,out] iterPtr                  - points to the current iteration
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbGetL3Vr
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *ipShadowPtr,
    OUT    GT_U32                               *tableSizePtr,
    OUT    GT_VOID                              **tablePtrPtr,
    INOUT  GT_UINTPTR                           *iterPtr,
    INOUT  GT_U32                               *dataSizePtr
)
{
    GT_STATUS retVal = GT_OK;
    GT_U8 ipAddr[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32 *vrId;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC currNh;
    PRV_CPSS_EXMXPM_IPV6MC_PCL_ENTRY_STC ipv6McPclEntry,*ipv6McPclEntryPtr;

    PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC *vrEntry;
    GT_U32 numOfEntriesLeft = 0;

    GT_U32 numOfEntries = 0;

    if (*iterPtr == 0)
    {
        /* this is the first call , so intilize */
        *iterPtr = (GT_UINTPTR)cpssOsLpmMalloc(sizeof (GT_U32));
        if (*iterPtr == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        vrId = (GT_U32*)(*iterPtr);
        *vrId = 0;
    }
    else
    {
        vrId = (GT_U32*)(*iterPtr);

    }

    numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* set the pointer */
    vrEntry = (PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC*)(*tablePtrPtr);
    for(;(numOfEntries < numOfEntriesLeft) &&
          (*vrId < ipShadowPtr->vrfTblSize); (*vrId)++)
    {

        if (ipShadowPtr->vrRootBucketArray[*vrId].valid == GT_FALSE)
        {
            continue;
        }

        if (tableSizePtr != NULL)
        {
            *tableSizePtr += sizeof(PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC);
        }
        if (*tablePtrPtr != NULL)
        {
            vrEntry->vrId = *vrId;
            vrEntry->isLast = GT_FALSE;

            /* check the protocol stack */
            if ((ipShadowPtr->vrRootBucketArray[*vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] != NULL) &&
                (ipShadowPtr->vrRootBucketArray[*vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] != NULL))
            {
                vrEntry->protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
            }
            else if (ipShadowPtr->vrRootBucketArray[*vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] != NULL)
            {
                vrEntry->protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
            }
            else
            {
                vrEntry->protocolStack = CPSS_IP_PROTOCOL_IPV6_E;
            }

            /* check IPv4 uc */
            if (ipShadowPtr->vrRootBucketArray[*vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] != NULL)
            {
                retVal = prvCpssDxChLpmRamUcEntrySearch(*vrId,ipAddr,0,&currNh,
                                                        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                        ipShadowPtr);
                if (retVal != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                cpssOsMemCpy(&vrEntry->defUcRouteEntrey[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E],
                             &currNh,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                vrEntry->vrIpUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = GT_TRUE;
            }
            else
            {
                 vrEntry->vrIpUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = GT_FALSE;
            }
            /* check ipv4 mc */
            retVal = prvCpssDxChLpmRamMcEntrySearch(*vrId,ipAddr,0,NULL
                                                    ipAddr,0,&currNh,
                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                    ipShadowPtr);
            if ((retVal != GT_OK) && (retVal !=GT_NOT_FOUND))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            if (retVal == GT_NOT_FOUND)
            {
                /* ipv4 mc is disable */
                vrEntry->vrIpMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = GT_FALSE;
                 cpssOsMemSet(&vrEntry->defMcRouteEntrey[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E],
                         0,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            }
            else
            {
                vrEntry->vrIpMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = GT_TRUE;
                 cpssOsMemCpy(&vrEntry->defMcRouteEntrey[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E],
                         &currNh,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            }

            /* check IPv6 uc */
            if (ipShadowPtr->vrRootBucketArray[*vrId].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] != NULL)
            {
                retVal = prvCpssDxChLpmRamUcEntrySearch(*vrId,ipAddr,0,&currNh,
                                                        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                        ipShadowPtr);
                if (retVal != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                cpssOsMemCpy(&vrEntry->defUcRouteEntrey[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E],
                             &currNh,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                vrEntry->vrIpUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = GT_TRUE;
            }
            else
            {
                vrEntry->vrIpUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = GT_FALSE;
            }
            /* check ipv6 mc */

            ipv6McPclEntry.ipGroupPrefix = 0;
            ipv6McPclEntryPtr = prvCpssSlSearch(ipShadowPtr->vrRootBucketArray[*vrId].coreIPv6McGroup2PceIdDb,
                              &ipv6McPclEntry);

            if (ipv6McPclEntryPtr == NULL)
            {
                vrEntry->vrIpMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = GT_FALSE;
                cpssOsMemSet(&vrEntry->defMcRouteEntrey[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E],
                             0,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            }
            else
            {
                vrEntry->vrIpMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = GT_TRUE;
                cpssOsMemCpy(&vrEntry->defMcRouteEntrey[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E],
                             ipv6McPclEntryPtr->lpmBucketPtr->rangeList->lowerLpmPtr.pNextHopEntry,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

                vrEntry->defIpv6McRuleIndex = ipv6McPclEntryPtr->pceId;

                vrEntry->vrIpv6McPclId =
                    ipShadowPtr->vrRootBucketArray[*vrId].vrIpv6McPclId;
            }
            /* advance the pointer */
            vrEntry++;
        }
        numOfEntries++;
    }

    /* check if we finished iterating*/
    if ( (*vrId == ipShadowPtr->vrfTblSize) && (numOfEntriesLeft > numOfEntries))
    {
        if (*tablePtrPtr != NULL)
        {
            vrEntry->isLast = GT_TRUE;
            *tablePtrPtr = (GT_VOID *)(vrEntry + 1);
        }
        else
        {
            /* take in account "last" virtual router entry: */
            *tableSizePtr += sizeof(PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC);
        }
        numOfEntries++;
        /* free & zero the iterator */
        cpssOsLpmFree((GT_PTR)*iterPtr);
        *iterPtr = 0;

        retVal = GT_OK;
    }
    else
    {
        /* we didn't finish record the ptr */
        *tablePtrPtr = (GT_VOID *)vrEntry;
    }

    /* update the left num of entries */
    numOfEntriesLeft -= numOfEntries;


    *dataSizePtr = *dataSizePtr - numOfEntries *(sizeof(PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC));

    return (retVal);
}

/**
* @internal ipLpmDbGetL3UcPrefix function
* @endinternal
*
* @brief   Retrives a specifc device's core ip uc prefix memory Size needed and info
*
* @param[in] ipShadowPtr              - points to the shadow to retrive from
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in,out] iterPtr                  - points to the current iteration
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes)
* @param[out] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iteration
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbGetL3UcPrefix
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    IN     GT_U32                           vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT    GT_U32                           *tableSizePtr,
    OUT    GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_STATUS retVal = GT_OK;
    GT_U8 ipAddr[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32 ipPrefix;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC currNh;
    GT_U32 numOfEntries = 0;
    GT_U32 numOfEntriesLeft = 0;
    PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC *ipv6Prefix;
    PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC *ipv4Prefix;
    GT_PTR  dummy;

    cpssOsMemSet(&currNh,0,sizeof(currNh));

    if (*iterPtr == 0)
    {
        /* this is the first call , so intilize */
        *iterPtr = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E) ?
            (GT_UINTPTR)cpssOsLpmMalloc(sizeof (PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC)):
            (GT_UINTPTR)cpssOsLpmMalloc(sizeof (PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC));
        ipPrefix = 0;
    }
    else
    {
        /* get the current */
        if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        {
            ipv4Prefix = (PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC*)(*iterPtr);
            cpssOsMemCpy(ipAddr,ipv4Prefix->ipAddr.arIP,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
            ipPrefix = ipv4Prefix->prefixLen;

        }
        else
        {
            ipv6Prefix = (PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC*)(*iterPtr);
            cpssOsMemCpy(ipAddr,ipv6Prefix->ipAddr.arIP,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
            ipPrefix = ipv6Prefix->prefixLen ;
        }
    }


     if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
         numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC));
     else
         numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* set the pointer */
    ipv4Prefix = (PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC*)(*tablePtrPtr);
    ipv6Prefix = (PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC*)(*tablePtrPtr);

    retVal =
        prvCpssDxChLpmRamEntryGet(ipShadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack],
                                  ipAddr,&ipPrefix,&dummy);
    if(retVal == GT_OK)
        currNh = *((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)dummy);

    while (retVal == GT_OK)
    {
        if (tableSizePtr != NULL)
        {
            *tableSizePtr += (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
                          sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC):
                sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC);
        }
        if (*tablePtrPtr != NULL)
        {
            /* copy the prefix info, start with ip address*/
            if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
            {
                cpssOsMemCpy(ipv4Prefix->ipAddr.arIP,ipAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
                cpssOsMemCpy(&ipv4Prefix->nextHopPointer,&currNh,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                ipv4Prefix->prefixLen = ipPrefix;
                ipv4Prefix->vrId = vrId;

                /* advance the pointer */
                ipv4Prefix = ipv4Prefix + 1;
            }
            else
            {
                cpssOsMemCpy(ipv6Prefix->ipAddr.arIP,ipAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
                cpssOsMemCpy(&ipv6Prefix->nextHopPointer,&currNh,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                ipv6Prefix->prefixLen = ipPrefix;
                ipv6Prefix->vrId = vrId;

                /* advance the pointer */
                ipv6Prefix = ipv6Prefix + 1;
            }
        }
        numOfEntries++;
        if (numOfEntries >= numOfEntriesLeft)
            /* we run out of entries , we'll conitune later */
            break;

        retVal =
            prvCpssDxChLpmRamEntryGet(ipShadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack],
                                      ipAddr,&ipPrefix,&dummy);

        if(retVal == GT_OK)
            currNh = *((PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC*)dummy);
    }

    /* check if we finished iterating*/
    if (retVal != GT_OK)
    {
        if (tableSizePtr != NULL)
        {
            /* make space for that the last one with invalid prefix of 255 */
            *tableSizePtr += (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
                sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC):
                sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC);

        }
        if (*tablePtrPtr != NULL)
        {
            /* indicate the last one with invalid prefix of 255 */
            if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
            {
                ipv4Prefix->prefixLen = 0xFF;
                /* advance the pointer */
                *tablePtrPtr = (GT_VOID *)(ipv4Prefix + 1);
            }
            else
            {
                ipv6Prefix->prefixLen = 0xFF;
                /* advance the pointer */
                *tablePtrPtr = (GT_VOID *)(ipv6Prefix + 1);
            }
        }
        numOfEntries++;
        /* free & zero the iterator */
        cpssOsLpmFree((GT_PTR)*iterPtr);
        *iterPtr = 0;

        retVal = GT_OK;
    }
    else
    {
        /* we didn't finish , record the currnet prefix. */
        if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        {
            PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC *tmpIpv4Prefix =
                (PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC*)*iterPtr;
            cpssOsMemCpy(tmpIpv4Prefix->ipAddr.arIP,ipAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
            tmpIpv4Prefix->prefixLen = ipPrefix;

            /* record the ptr */
            *tablePtrPtr = (GT_VOID *)ipv4Prefix;

        }
        else
        {
            PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC *tmpIpv6Prefix =
                (PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC*)*iterPtr;
            cpssOsMemCpy(tmpIpv6Prefix->ipAddr.arIP,ipAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
            tmpIpv6Prefix->prefixLen = ipPrefix;

            /* record the ptr */
            *tablePtrPtr = (GT_VOID *)ipv6Prefix ;
        }
    }

    /* update the left num of entries */
    numOfEntriesLeft -= numOfEntries;

    if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC));
    else
        *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC));

    return(retVal);
}

/**
* @internal ipLpmDbGetL3UcLpm function
* @endinternal
*
* @brief   Retrives a specifc device's core ip uc prefix memory Size needed and info
*
* @param[in] ipShadowPtr              - points to the shadow to retrive from
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in,out] iterPtr                  - points to the current iteration
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes)
* @param[out] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iteration
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbGetL3UcLpm
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    IN     GT_U32                           vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT    GT_U32                           *tableSizePtr,
    OUT    GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC tmpRootRange;

    GT_U32 memAllocIndex = 0;
    GT_U32 numOfEntriesLeft = 0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_OP_ENT traOp;
    PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC *memAllocInfo = NULL;

    if (ipShadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack] == NULL)
    {
        /* no uc prefixes */
        return GT_OK;
    }
    if (*tablePtrPtr != NULL)
    {
        /* set the params for lpm traverse */
        traOp = PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_REC_WITH_PCL_E;

        memAllocInfo = (PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC*)*tablePtrPtr;
    }
    else
    {
        traOp = PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_COUNT_WITH_PCL_E;
    }


    numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));
    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    tmpRootRange.lowerLpmPtr.nextBucket =
        ipShadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];
    tmpRootRange.pointerType =
        (GT_U8)(ipShadowPtr->vrRootBucketArray[vrId].rootBucketType[protocolStack]);
    tmpRootRange.next = NULL;

    retVal = prvCpssDxChLpmRamMemTraverse(traOp,
                                          memAllocInfo,
                                          &memAllocIndex,
                                          numOfEntriesLeft,
                                          &tmpRootRange,
                                          GT_FALSE,
                                          ipShadowPtr->ipUcSearchMemArrayPtr[protocolStack],
                                          iterPtr);

    /* update the left num of entries */
    numOfEntriesLeft = numOfEntriesLeft - memAllocIndex;

    *dataSizePtr = *dataSizePtr - memAllocIndex * (sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));
    if (tableSizePtr != NULL)
    {
        /* calc the lpm memory need */
        *tableSizePtr += memAllocIndex * sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC);
    }
    if (*tablePtrPtr != NULL)
    {
        /* advance the pointer */
        *tablePtrPtr = (GT_VOID*)(&memAllocInfo[memAllocIndex]);
    }
    return (retVal);
}

/**
* @internal ipLpmDbGetL3McRoutes function
* @endinternal
*
* @brief   Retrives a specifc device's core ip mc routes memory Size needed and info
*
* @param[in] ipShadowPtr              - points to the shadow to retrive from
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes)
* @param[out] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbGetL3McRoutes
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    IN     GT_U32                           vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT    GT_U32                           *tableSizePtr,
    OUT    GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_STATUS retVal = GT_OK;
    GT_U8 groupIp[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS],srcAddr[PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS];
    GT_U32 groupPrefix,srcPrefix;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC mcEntry;
    GT_U32 numOfEntries = 0;
    GT_U32 numOfEntriesLeft = 0;
    IPV6_HSU_MC_ROUTE_ENTRY_STC *ipv6McRoute;
    IPV4_HSU_MC_ROUTE_ENTRY_STC *ipv4McRoute;

    cpssOsMemSet(&mcEntry,0,sizeof(mcEntry));

    if (*iterPtr == 0)
    {
        /* this is the first call , so intilize */
        *iterPtr = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E) ?
               (GT_UINTPTR)cpssOsLpmMalloc(sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC)):
               (GT_UINTPTR)cpssOsLpmMalloc(sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC));
        groupPrefix = 0;
    }
    else
    {
        /* get the current */
        if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        {
            ipv4McRoute = (IPV4_HSU_MC_ROUTE_ENTRY_STC*)(*iterPtr);
            cpssOsMemCpy(groupIp,ipv4McRoute->mcGroup.arIP,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
            cpssOsMemCpy(srcAddr,ipv4McRoute->ipSrc.arIP,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));

            groupPrefix = (ipv4McRoute->mcGroup.u32Ip == 0)?0:32;
        }
        else
        {
            ipv6McRoute = (IPV6_HSU_MC_ROUTE_ENTRY_STC*)(*iterPtr);
            cpssOsMemCpy(groupIp,ipv6McRoute->mcGroup.arIP,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
            cpssOsMemCpy(srcAddr,ipv6McRoute->ipSrc.arIP,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));

            groupPrefix = ((ipv6McRoute->mcGroup.u32Ip[0] == 0) &&
                           (ipv6McRoute->mcGroup.u32Ip[1] == 0) &&
                           (ipv6McRoute->mcGroup.u32Ip[2] == 0) &&
                           (ipv6McRoute->mcGroup.u32Ip[3] == 0))?0:128;
        }
    }


     if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
         numOfEntriesLeft = *dataSizePtr/(sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC));
     else
         numOfEntriesLeft = *dataSizePtr/(sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* set the ptrs */
    ipv4McRoute = (IPV4_HSU_MC_ROUTE_ENTRY_STC*)*tablePtrPtr;
    ipv6McRoute = (IPV6_HSU_MC_ROUTE_ENTRY_STC*)*tablePtrPtr;

    retVal = prvCpssDxChLpmRamMcEntryGetNext(vrId,groupIp,&groupPrefix,
                                             NULL,srcAddr,&srcPrefix,&mcEntry,
                                             protocolStack,ipShadowPtr);

    while (retVal == GT_OK)
    {
        if (tableSizePtr != NULL)
        {
            *tableSizePtr  +=
                (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
                sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC):
                sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC);
        }
        if (*tablePtrPtr != NULL)
        {
            if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
            {
                cpssOsMemCpy(ipv4McRoute->mcGroup.arIP,groupIp,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
                cpssOsMemCpy(ipv4McRoute->ipSrc.arIP,srcAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
                ipv4McRoute->ipSrcPrefixlength = srcPrefix;
                ipv4McRoute->vrId = vrId;
                cpssOsMemCpy(&ipv4McRoute->mcRoutePointer,&mcEntry,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                ipv4McRoute->last = GT_FALSE;

                ipv4McRoute++;
            }
            else
            {
                cpssOsMemCpy(ipv6McRoute->mcGroup.arIP,groupIp,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
                cpssOsMemCpy(ipv6McRoute->ipSrc.arIP,srcAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
                ipv6McRoute->vrId = vrId;
                ipv6McRoute->ipSrcPrefixlength = srcPrefix;
                cpssOsMemCpy(&ipv6McRoute->mcRoutePointer,&mcEntry,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                ipv6McRoute->last = GT_FALSE;
                ipv6McRoute++;
            }
        }

        numOfEntries++;

        if (numOfEntries >= numOfEntriesLeft)
            /* we run out of entries , we'll continue later */
            break;

        retVal = prvCpssDxChLpmRamMcEntryGetNext(vrId,groupIp,&groupPrefix,NULL,
                                                 srcAddr,&srcPrefix,&mcEntry,
                                                 protocolStack,ipShadowPtr);
    }

    /* check if we finished iterating*/
    if (retVal != GT_OK)
    {
        if (tableSizePtr != NULL)
        {
            /* make space for that the last one with invalid prefix of 255 */
            *tableSizePtr  += (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
                sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC):
                sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC);
        }

        if (*tablePtrPtr != NULL)
        {
            /* indicate last for mc*/
            if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
            {
                ipv4McRoute->last = GT_TRUE;
                *tablePtrPtr = (GT_VOID*)(ipv4McRoute + 1);
            }
            else
            {
                ipv6McRoute->last = GT_TRUE;
                *tablePtrPtr = (GT_VOID*)(ipv6McRoute + 1);
            }
        }
        numOfEntries++;
        /* free & zero the iterator */
        cpssOsLpmFree((GT_PTR)*iterPtr);
        *iterPtr = 0;

        retVal = GT_OK;
    }
    else
    {
        /* we didn't finish , record the currnet prefix. */
        if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        {
            IPV4_HSU_MC_ROUTE_ENTRY_STC* tmpIpv4McRoute =
                (IPV4_HSU_MC_ROUTE_ENTRY_STC*)*iterPtr;
            cpssOsMemCpy(tmpIpv4McRoute->mcGroup.arIP,groupIp,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
            cpssOsMemCpy(tmpIpv4McRoute->ipSrc.arIP,srcAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS*sizeof(GT_U8));
            /* record the ptr */
            *tablePtrPtr = (GT_VOID *)ipv4McRoute;
        }
        else
        {
            IPV6_HSU_MC_ROUTE_ENTRY_STC* tmpIpv6McRoute =
                (IPV6_HSU_MC_ROUTE_ENTRY_STC*)*iterPtr;
            cpssOsMemCpy(tmpIpv6McRoute->mcGroup.arIP,groupIp,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
            cpssOsMemCpy(tmpIpv6McRoute->ipSrc.arIP,srcAddr,PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS*sizeof(GT_U8));
            /* record the ptr */
            *tablePtrPtr = (GT_VOID *)ipv6McRoute;
        }
    }

    /* update the left num of entries */
    numOfEntriesLeft -= numOfEntries;

     if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
         *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC));
     else
         *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC));
    return (retVal);
}

/**
* @internal ipLpmDbGetL3McLpm function
* @endinternal
*
* @brief   Retrives a specifc device's core ip mc routes memory Size needed and info
*
* @param[in] ipShadowPtr              - points to the shadow to retrive from
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration step
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes)
* @param[out] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbGetL3McLpm
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    IN     GT_U32                           vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    OUT    GT_U32                           *tableSizePtr,
    OUT    GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_EXMXPM_IPV6MC_PCL_ENTRY_STC ipv6McPclEntry,*ipv6McPclEntryPtr;
    GT_U32 memAllocIndex = 0;
    GT_U32 numOfEntriesLeft = 0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_OP_ENT traOp;
    PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC *memAllocInfo = NULL;

    /* check if mc prefixes are exist */
    if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E )
    {
        if(ipShadowPtr->vrRootBucketArray[vrId].mcRootBucket[CPSS_IP_PROTOCOL_IPV4_E] == NULL)
        {
            /* no ipv4 mc prefixes */
            return GT_OK;
        }
    }
    if (protocolStack == CPSS_IP_PROTOCOL_IPV6_E )
    {
        if(ipShadowPtr->vrRootBucketArray[vrId].mcRootBucket[CPSS_IP_PROTOCOL_IPV6_E] == NULL)
        {
            /* no ipv4 mc prefixes */
            return GT_OK;
        }
    }
    if (*tablePtrPtr != NULL)
    {
        /* set the params for lpm traverse */
        traOp = PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_REC_WITH_PCL_E;

        memAllocInfo = (PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC*)*tablePtrPtr;
    }
    else
    {
        traOp = PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_COUNT_WITH_PCL_E;
    }
    memAllocIndex = 0;


    numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* move on to mc LPM*/
    retVal = prvCpssDxChLpmRamMcTraverse(traOp,memAllocInfo,
                                         &memAllocIndex,
                                         numOfEntriesLeft,
                                         vrId,ipShadowPtr,iterPtr);
    /* update the left num of entries */
    numOfEntriesLeft = numOfEntriesLeft - memAllocIndex;


    *dataSizePtr = *dataSizePtr - memAllocIndex * (sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));

    if (tableSizePtr != NULL)
    {
        /* calc the mc lpm memory need */
        *tableSizePtr  += memAllocIndex * sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC);
    }
    if (*tablePtrPtr != NULL)
    {
        /* advance the pointer */
        *tablePtrPtr = (GT_VOID*)(&memAllocInfo[memAllocIndex]);
    }
    return (retVal);
}

/**
* @internal ipLpmDbGetL3DmmFreeLists function
* @endinternal
*
* @brief   Retrives needed dmm free lists memory size and info.
*
* @param[in] ipShadowPtr              - points to the shadow to retrive from
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration step
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes)
* @param[out] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbGetL3DmmFreeLists
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    OUT    GT_U32                           *tableSizePtr,
    OUT    GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_U32 numberInternalEntiresLeft = 0;
    GT_U32 numberInternalEntriesUsed = 0;
    DMM_IP_HSU_ENTRY_STC *dmmIterEntry;
    GT_U32 i = 0;
    GT_U32 j = 0;
    GT_U32 *hsuMemPtr;
    GT_DMM_BLOCK *tempPtr;
    if (*iterPtr == 0)
    {
        /* this is the first call , so intilize */
        *iterPtr = (GT_UINTPTR)cpssOsLpmMalloc(sizeof(DMM_IP_HSU_ENTRY_STC));
        if (*iterPtr == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        dmmIterEntry = (DMM_IP_HSU_ENTRY_STC*)(*iterPtr);
        dmmIterEntry->currentDmmBlockAddrInList = NULL;
        dmmIterEntry->currentFreeLinkList = 0;
        dmmIterEntry->partitionIndex = 0;
        dmmIterEntry->endOfList = 0;
        dmmIterEntry->endOfPartition = 0;
        cpssOsMemSet(dmmIterEntry->partitionArray,0,sizeof(dmmIterEntry->partitionArray));

        /* need to change logic, since for bobcat2 and above we support multi blocks mapping per octet*/
        CPSS_TBD_BOOKMARK

        /*this code need to be checked and the next code should be deleted*/

        for (j = 0, i= 0; (j <8) && (i < ipShadowPtr->numOfLpmMemories); i++, j++;)
        {
           dmmIterEntry->partitionArray[j] = (GT_DMM_PARTITION*)ipShadowPtr->lpmRamStructsMemPoolPtr[i];
        }

        /* init dmm partition array. Partition array should contain only different partition Ids */
        /*for (j = 0, i= 0; (j <8) && (i < ipShadowPtr->numOfLpmMemories); i++)
        {
           if (ipShadowPtr->lpmMemInfoArray[0][i].structsMemPool ==
               ipShadowPtr->lpmMemInfoArray[1][i].structsMemPool)
           {
               dmmIterEntry->partitionArray[j] = (GT_DMM_PARTITION*)ipShadowPtr->lpmMemInfoArray[0][i].structsMemPool;
               j++;
           }
           else
           {
               dmmIterEntry->partitionArray[j] = ((GT_DMM_PARTITION*)ipShadowPtr->lpmMemInfoArray[0][i].structsMemPool);
               dmmIterEntry->partitionArray[j+1] = (GT_DMM_PARTITION*)ipShadowPtr->lpmMemInfoArray[1][i].structsMemPool;
               j = j+2;
           }
        }
        */
    }
    else
    {
        dmmIterEntry = (DMM_IP_HSU_ENTRY_STC*)(*iterPtr);
    }

    numberInternalEntiresLeft = *dataSizePtr/(sizeof(GT_U32));

    if (numberInternalEntiresLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* set the pointer */
    hsuMemPtr = (GT_U32*)(*tablePtrPtr);
    i = dmmIterEntry->currentFreeLinkList;
    j = dmmIterEntry->partitionIndex;
    for (; j < 8; j ++)
    {
        if (dmmIterEntry->partitionArray[j] == NULL)
        {
            continue;
        }
        dmmIterEntry->partitionIndex = j;
        if (dmmIterEntry->endOfPartition == GT_FALSE)
        {
            for(;i < DMM_MAXIMAL_BLOCK_SIZE_ALLOWED+1;i++)
            {
                dmmIterEntry->currentFreeLinkList = i;
                if (i == 0)
                {
                    /* first link list in partition is examined. Write partition index */
                    if (numberInternalEntiresLeft > numberInternalEntriesUsed )
                    {
                        if (tableSizePtr != NULL)
                        {
                            *tableSizePtr+= sizeof(GT_U32);
                        }
                        if (*tablePtrPtr != NULL)
                        {
                            *hsuMemPtr = j;
                            hsuMemPtr++;
                        }
                        numberInternalEntriesUsed++;
                    }
                    else
                    {
                        *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                        return GT_OK;
                    }
                }
                if (dmmIterEntry->currentDmmBlockAddrInList == NULL)
                {
                    /* new free list is in process */
                    if (dmmIterEntry->partitionArray[j]->tableOfSizePointers[i] == NULL)
                    {
                        /* no export action for empty free list */
                        continue;
                    }
                    else
                    {
                        /* write link list number-name */
                        if (numberInternalEntiresLeft > numberInternalEntriesUsed )
                        {
                            if (tableSizePtr != NULL)
                            {
                                *tableSizePtr+= sizeof(GT_U32);
                            }
                            if (*tablePtrPtr != NULL)
                            {
                                *hsuMemPtr = i;
                                hsuMemPtr++;
                            }
                            numberInternalEntriesUsed++;
                        }
                        else
                        {
                            *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                            return GT_OK;
                        }
                        tempPtr = dmmIterEntry->partitionArray[j]->tableOfSizePointers[i];
                    }
                }
                else
                {
                    /*  free list from previuos iteration is not finished yet. Write
                        hw offset of remains link list members */
                    tempPtr = dmmIterEntry->currentDmmBlockAddrInList;
                }
                /* write hw offset of  link list members */
                if (dmmIterEntry->endOfList == GT_FALSE)
                {
                    do
                    {
                        dmmIterEntry->currentDmmBlockAddrInList = tempPtr;
                        if (numberInternalEntiresLeft > numberInternalEntriesUsed )
                        {
                            if (tableSizePtr != NULL)
                            {
                                *tableSizePtr+= sizeof(GT_U32);
                            }
                            if (*tablePtrPtr != NULL)
                            {
                                *hsuMemPtr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempPtr);
                                hsuMemPtr++;
                            }
                            numberInternalEntriesUsed++;
                        }
                        else
                        {
                            *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                            return GT_OK;
                        }
                        tempPtr = tempPtr->nextBySizeOrPartitionPtr.nextBySize;
                    }while(tempPtr != NULL);
                }

                /* write end of the list */
                if (numberInternalEntiresLeft > numberInternalEntriesUsed )
                {
                    if (tableSizePtr != NULL)
                    {
                        *tableSizePtr+= sizeof(GT_U32);
                    }
                    if (*tablePtrPtr != NULL)
                    {
                        *hsuMemPtr = PRV_CPSS_DXCH_LPM_RAM_HSU_DMM_FREE_LINK_LIST_END_CNS;
                        hsuMemPtr++;
                    }
                    numberInternalEntriesUsed++;
                    dmmIterEntry->endOfList = GT_FALSE;
                }
                else
                {
                    dmmIterEntry->endOfList = GT_TRUE;
                    *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                    return GT_OK;
                }
                dmmIterEntry->currentDmmBlockAddrInList = NULL;
            }
            /* small blocks export was done */
            if (dmmIterEntry->currentFreeLinkList <= DMM_MAXIMAL_BLOCK_SIZE_ALLOWED)
            {
                dmmIterEntry->currentFreeLinkList = DMM_MAXIMAL_BLOCK_SIZE_ALLOWED+1;
            }
            /* now handle big blocks list */

            /* check if big block list is exist */
            if (dmmIterEntry->partitionArray[j]->bigBlocksList != NULL)
            {
                tempPtr = dmmIterEntry->partitionArray[j]->bigBlocksList;
                if (numberInternalEntiresLeft > numberInternalEntriesUsed )
                {
                    if (tableSizePtr != NULL)
                    {
                        *tableSizePtr+= sizeof(GT_U32);
                    }
                    if (*tablePtrPtr != NULL)
                    {
                        *hsuMemPtr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempPtr);
                        hsuMemPtr++;
                    }
                    dmmIterEntry->currentFreeLinkList = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(tempPtr);
                    numberInternalEntriesUsed++;
                }
                else
                {
                    *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                    return GT_OK;
                }
                if (dmmIterEntry->currentDmmBlockAddrInList == NULL)
                {
                    /* first offset in big blocks list */
                    dmmIterEntry->currentDmmBlockAddrInList = dmmIterEntry->partitionArray[j]->bigBlocksList;
                }
                tempPtr = dmmIterEntry->currentDmmBlockAddrInList;
                do
                {
                    dmmIterEntry->currentDmmBlockAddrInList = tempPtr;
                    if (numberInternalEntiresLeft > numberInternalEntriesUsed )
                    {
                        if (tableSizePtr != NULL)
                        {
                            *tableSizePtr+= sizeof(GT_U32);
                        }
                        if (*tablePtrPtr != NULL)
                        {
                            *hsuMemPtr = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempPtr);
                            hsuMemPtr++;
                        }
                        numberInternalEntriesUsed++;
                    }
                    else
                    {
                        *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                        return GT_OK;
                    }
                    tempPtr = tempPtr->nextBySizeOrPartitionPtr.nextBySize;
                }while(tempPtr != NULL);
            }
            /* prepare free link list indexes for new partition */
            dmmIterEntry->currentDmmBlockAddrInList = NULL;
            i = 0;
            dmmIterEntry->currentFreeLinkList = 0;
        }
        /* partition is done : partition end signature is assigned */
        if (numberInternalEntiresLeft > numberInternalEntriesUsed )
        {
            if (tableSizePtr != NULL)
            {
                *tableSizePtr+= sizeof(GT_U32);
            }
            if (*tablePtrPtr != NULL)
            {
                *hsuMemPtr = PRV_CPSS_DXCH_LPM_RAM_HSU_DMM_PARTITION_END_CNS;
                hsuMemPtr++;
            }
            numberInternalEntriesUsed++;
            dmmIterEntry->endOfPartition = GT_FALSE;
        }
        else
        {
            dmmIterEntry->endOfPartition = GT_TRUE;
            *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
            return GT_OK;
        }
    }
    *tablePtrPtr = (GT_VOID*)hsuMemPtr;
    /* in case all data was processed */
    /* update the left num of entries */

    numberInternalEntiresLeft -= numberInternalEntriesUsed;

    *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
    /* free & zero the iterator */
    cpssOsLpmFree((GT_PTR)*iterPtr);
    *iterPtr = 0;

    return GT_OK;
}

/**
* @internal ipLpmDbGetL3 function
* @endinternal
*
* @brief   Retrives a specifc shadow's ip Table memory Size needed and info
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in,out] iterationSizePtr         - points to data size in bytes supposed to be processed
*                                      in current iteration
* @param[in,out] iterPtr                  - points to the iterator, to start - set to 0.
*
* @param[out] tableSizePtr             - points to the table size calculated (in bytes).
* @param[out] tablePtr                 - points to the table size info block.
* @param[in,out] iterationSizePtr         - points to data size in bytes left after iteration .
* @param[in,out] iterPtr                  - points to the iterator, if = 0 then the operation is done.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad iterPtr
*/
GT_STATUS ipLpmDbGetL3
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    OUT   GT_U32                                *tableSizePtr,
    OUT   GT_VOID                               *tablePtr,
    INOUT GT_U32                                *iterationSizePtr,
    INOUT GT_UINTPTR                            *iterPtr
)
{
    GT_STATUS                       retVal = GT_OK;
    IP_HSU_ITERATOR_STC             *currentIterPtr;

    /* if HSU process called this function currDataAmount is size in bytes.
       otherwise it is number of entries */
    GT_U32                              *currDataSizePtr = iterationSizePtr;
    GT_U32                              curentDataSizeBeforeStage;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *ipShadowPtr;

    currentIterPtr = (IP_HSU_ITERATOR_STC*)*iterPtr;

    if (currentIterPtr == NULL)
    {
        /* we need to allocate the iterator */
        currentIterPtr =
            (IP_HSU_ITERATOR_STC*)cpssOsLpmMalloc(sizeof(IP_HSU_ITERATOR_STC));
        if (currentIterPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        /*reset it */
        cpssOsMemSet(currentIterPtr,0,sizeof(IP_HSU_ITERATOR_STC));

        /* set the starting stage */
        currentIterPtr->currStage = IP_HSU_VR_STAGE_E;
        currentIterPtr->magic = PRV_CPSS_EXMXPM_HSU_ITERATOR_MAGIC_NUMBER_CNC;
        *iterPtr = (GT_UINTPTR)currentIterPtr;

        /* init the table size */
        if (tableSizePtr != NULL)
        {
            *tableSizePtr = 0;
        }
    }
    else
    {
        if(currentIterPtr->magic != PRV_CPSS_EXMXPM_HSU_ITERATOR_MAGIC_NUMBER_CNC)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    /* set the memory position */
    currentIterPtr->currMemPtr = tablePtr;

    for (;currentIterPtr->currShadowIdx < lpmDbPtr->numOfShadowCfg;
          currentIterPtr->currShadowIdx++)
    {
        ipShadowPtr = &lpmDbPtr->shadowArray[currentIterPtr->currShadowIdx];

        if (currentIterPtr->currStage == IP_HSU_VR_STAGE_E)
        {
            curentDataSizeBeforeStage = *currDataSizePtr;
            retVal = ipLpmDbGetL3Vr(ipShadowPtr,tableSizePtr,
                                    &currentIterPtr->currMemPtr,
                                    &currentIterPtr->currIter,
                                    currDataSizePtr);
            if ((retVal == GT_NO_RESOURCE)&&
                (*currDataSizePtr == curentDataSizeBeforeStage))
            {
                /* the stage is not done: the data size was smaller than entry in shadow */
                return GT_OK;
            }
            /* check if we finished with the phase */
            if (currentIterPtr->currIter == 0)
            {
                /* set the next stage */
                currentIterPtr->currStage = IP_HSU_UC_PREFIX_STAGE_E;
            }

            if ((retVal != GT_OK) || (*currDataSizePtr == 0))
            {
                return(retVal);
            }
        }

        if (currentIterPtr->currStage != IP_HSU_DMM_FREE_LIST_E)
        {
            for (;currentIterPtr->currProtocolStack < 2; currentIterPtr->currProtocolStack++)
            {
                if (ipShadowPtr->isIpVerIsInitialized[currentIterPtr->currProtocolStack] == GT_FALSE)
                {
                    continue;
                }

                while (currentIterPtr->currVrId < ipShadowPtr->vrfTblSize)
                {
                    /* go over all valid VR */
                    if ((ipShadowPtr->vrRootBucketArray[currentIterPtr->currVrId].valid == GT_FALSE) ||
                        (ipShadowPtr->vrRootBucketArray[currentIterPtr->currVrId].rootBucket[currentIterPtr->currProtocolStack] == NULL))
                    {
                        currentIterPtr->currVrId++;
                        continue;
                    }

                    if (currentIterPtr->currStage == IP_HSU_UC_PREFIX_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbGetL3UcPrefix(ipShadowPtr,
                                                      currentIterPtr->currVrId,
                                                      currentIterPtr->currProtocolStack,
                                                      tableSizePtr,
                                                      &currentIterPtr->currMemPtr,
                                                      &currentIterPtr->currIter,
                                                      currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }

                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_UC_LPM_STAGE_E;
                        }

                        if ((retVal != GT_OK) || (*currDataSizePtr == 0))
                        {
                            return(retVal);
                        }
                    }

                    if (currentIterPtr->currStage == IP_HSU_UC_LPM_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbGetL3UcLpm(ipShadowPtr,
                                                   currentIterPtr->currVrId,
                                                   currentIterPtr->currProtocolStack,
                                                   tableSizePtr,
                                                   &currentIterPtr->currMemPtr,
                                                   &currentIterPtr->currIter,
                                                   currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }
                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_MC_PREFIX_STAGE_E;
                        }
                        if ((retVal != GT_OK) || (*currDataSizePtr == 0))
                        {
                            return(retVal);
                        }
                    }

                    if (currentIterPtr->currStage == IP_HSU_MC_PREFIX_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbGetL3McRoutes(ipShadowPtr,
                                                      currentIterPtr->currVrId,
                                                      currentIterPtr->currProtocolStack,
                                                      tableSizePtr,
                                                      &currentIterPtr->currMemPtr,
                                                      &currentIterPtr->currIter,
                                                      currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }
                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_MC_LPM_STAGE_E;
                        }

                        if ((retVal != GT_OK) || (*currDataSizePtr == 0))
                        {
                            return(retVal);
                        }
                    }

                    if (currentIterPtr->currStage == IP_HSU_MC_LPM_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbGetL3McLpm(ipShadowPtr,
                                                   currentIterPtr->currVrId,
                                                   currentIterPtr->currProtocolStack,
                                                   tableSizePtr,
                                                   &currentIterPtr->currMemPtr,
                                                   &currentIterPtr->currIter,
                                                   currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }
                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_UC_PREFIX_STAGE_E;
                            /* this is the end for this vrid */
                            currentIterPtr->currVrId++;
                        }
                        if (retVal != GT_OK)
                        {
                            return(retVal);
                        }
                   }
                }
                /* we finished with this protocol stack zero the vrid */
                currentIterPtr->currVrId = 0;
            }
        }

        /* the last stage is IP_HSU_DMM_FREE_LIST_E for this shadow*/
        currentIterPtr->currStage = IP_HSU_DMM_FREE_LIST_E;
        if (currentIterPtr->currStage == IP_HSU_DMM_FREE_LIST_E)
        {
            curentDataSizeBeforeStage = *currDataSizePtr;
            retVal = ipLpmDbGetL3DmmFreeLists(ipShadowPtr,
                                              tableSizePtr,
                                              &currentIterPtr->currMemPtr,
                                              &currentIterPtr->currIter,
                                              currDataSizePtr);
            if ((retVal == GT_NO_RESOURCE)&&
                (*currDataSizePtr == curentDataSizeBeforeStage))
            {
                /* the stage is not done: the data size was smaller than entry in shadow */
                return GT_OK;
            }
            /* check if we finished with the last phase */
            if (currentIterPtr->currIter == 0)
            {
                /* set the start stage */
                currentIterPtr->currStage = IP_HSU_VR_STAGE_E;
            }

            if ((retVal != GT_OK)||(currentIterPtr->currIter != 0))
            {
                return(retVal);
            }
        }
        /* we finished with the shadow , set the starting stage */
        currentIterPtr->currStage = IP_HSU_VR_STAGE_E;
    }

    /* we reached the end free the iterator */
    cpssOsLpmFree(currentIterPtr);
    *iterPtr = 0;

    return (GT_OK);
}

/**
* @internal ipLpmDbSetL3Vr function
* @endinternal
*
* @brief   Retrives the valid virtual routers
*
* @param[in] ipShadowPtr              - points to the shadow to retrieve from
* @param[in] tablePtrPtr              - the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbSetL3Vr
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *ipShadowPtr,
    IN     GT_VOID                              **tablePtrPtr,
    INOUT  GT_UINTPTR                           *iterPtr,
    INOUT  GT_U32                               *dataSizePtr
)
{
    GT_STATUS   retVal = GT_OK;
    GT_U32      numOfEntries = 0;
    GT_U32      numOfEntriesLeft = 0;

    PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC *vrEntry;

    numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* set the pointer */
    vrEntry = (PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC*)(*tablePtrPtr);

    while ((vrEntry->isLast == GT_FALSE) && (numOfEntries < numOfEntriesLeft))
    {
        /* setting the VR automatically set the default UC and MC prefixes in the lpm DB */
        retVal = lpmVirtualRouterSet(vrEntry->vrId,
                                     vrEntry->defUcRouteEntrey[CPSS_IP_PROTOCOL_IPV4_E],
                                     vrEntry->defUcRouteEntrey[CPSS_IP_PROTOCOL_IPV6_E],
                                     vrEntry->defMcRouteEntrey[CPSS_IP_PROTOCOL_IPV4_E],
                                     vrEntry->defMcRouteEntrey[CPSS_IP_PROTOCOL_IPV6_E],
                                     vrEntry->protocolStack,
                                     vrEntry->vrIpUcSupport,
                                     vrEntry->vrIpMcSupport,
                                     vrEntry->defIpv6McRuleIndex,
                                     vrEntry->vrIpv6McPclId,
                                     ipShadowPtr,
                                     GT_FALSE);
        if (retVal != GT_OK)
        {
            return (retVal);
        }

        /* advance the pointer */
        vrEntry++;

        numOfEntries++;
    }

    /* check if we finished iterating*/
    if ( (vrEntry->isLast == GT_TRUE)&& (numOfEntriesLeft > numOfEntries))
    {
        /* zero the iterator */
        *iterPtr = 0;

        /* set the ptr */
        *tablePtrPtr = (GT_VOID *)(vrEntry + 1);
         numOfEntries++;
    }
    else
    {
         /* just indicate we didn't finish using a fake iterator */
        *iterPtr = 0xff;
        *tablePtrPtr = (GT_VOID *)vrEntry;
    }

    /* update the left num of entries */
    numOfEntriesLeft -= numOfEntries;

    *dataSizePtr = *dataSizePtr - numOfEntries *(sizeof(PRV_CPSS_DXCH_LPM_RAM_VR_INFO_STC));
    return (retVal);
}

/**
* @internal ipLpmDbSetL3UcPrefix function
* @endinternal
*
* @brief   Set the data needed for core uc prefix shadow reconstruction used
*         after HSU.
* @param[in] ipShadowPtr              - points to the shadow to set in
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbSetL3UcPrefix
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    IN     GT_U32                           vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    IN     GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopPointerPtr;
    GT_STATUS retVal = GT_OK;
    GT_U8 *ipAddr;
    GT_U32 ipPrefix;
    GT_U32 numOfEntries = 0;
    GT_U32 numOfEntriesLeft = 0;
    PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC *ipv6Prefix;
    PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC *ipv4Prefix;
    GT_BOOL isLast;

    /* set the pointer */
    ipv4Prefix = (PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC*)(*tablePtrPtr);
    ipv6Prefix = (PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC*)(*tablePtrPtr);

    /* start with uc prefixes */
    isLast = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
             (ipv4Prefix->prefixLen == 0xFF):(ipv6Prefix->prefixLen == 0xFF);


     if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
         numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC));
     else
         numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }


    while ((!isLast) && (numOfEntries < numOfEntriesLeft))
    {
        /* copy the prefix info, start with ip address*/
        if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        {
            ipAddr = ipv4Prefix->ipAddr.arIP;
            ipPrefix = ipv4Prefix->prefixLen;
            nextHopPointerPtr = &ipv4Prefix->nextHopPointer;
            vrId = ipv4Prefix->vrId;

            /* advance the pointer */
            ipv4Prefix = ipv4Prefix + 1;

            isLast = (ipv4Prefix->prefixLen == 0xFF);

        }
        else
        {
            ipAddr = ipv6Prefix->ipAddr.arIP;
            ipPrefix = (GT_U8)ipv6Prefix->prefixLen;
            nextHopPointerPtr = &ipv6Prefix->nextHopPointer;
            vrId = ipv6Prefix->vrId;

            /* advance the pointer */
            ipv6Prefix = ipv6Prefix + 1;

            isLast = (ipv6Prefix->prefixLen == 0xFF);

        }

        /* The default UC was already defined in the call to ipLpmDbSetL3Vr */
        if (ipPrefix != 0)
        {
            retVal = prvCpssDxChLpmRamUcEntryAdd(vrId,ipAddr,ipPrefix,nextHopPointerPtr,
                                                 PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E,
                                                 GT_FALSE,protocolStack,ipShadowPtr);
        }

        if (retVal != GT_OK)
        {
            return (retVal);
        }

        numOfEntries++;

    }

    /* check if we finished */
    if ( (isLast == GT_TRUE) && (numOfEntries < numOfEntriesLeft))
    {
        /* advance the pointer */
        *tablePtrPtr = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
            (GT_VOID *)(ipv4Prefix + 1) : (GT_VOID *)(ipv6Prefix + 1);
        numOfEntries++;
        /* just indicate we finished using a fake iterator */
        *iterPtr = 0;
    }
    else
    {
        /* just indicate we didn't finish using a fake iterator */
        *iterPtr = 0xff;

        /* and record the pointer */
        *tablePtrPtr = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
            (GT_VOID *)ipv4Prefix : (GT_VOID *)ipv6Prefix;
    }

    /* update the num of entries left */
    numOfEntriesLeft -= numOfEntries;

    if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV4_UC_PREFIX_BULK_OPERATION_STC));
    else
        *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(PRV_CPSS_DXCH_LPM_RAM_IPV6_UC_PREFIX_BULK_OPERATION_STC));
    return (retVal);
}

/**
* @internal ipLpmDbSetL3UcLpm function
* @endinternal
*
* @brief   Set the data needed for core uc LPM shadow reconstruction used
*         after HSU.
* @param[in] ipShadowPtr              - points to the shadow to set in
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbSetL3UcLpm
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    IN     GT_U32                           vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT       protocolStack,
    IN     GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC tmpRootRange;

    GT_U32 memAllocIndex = 0;
    GT_U32 numOfEntriesLeft = 0;

    PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC *memAllocInfo = NULL;

    numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* check uc prefixes */
    if (ipShadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack] == NULL)
    {
        /* no uc prefixes */
        return GT_OK;
    }
    /* set the pointer for lpm traverse */
    memAllocInfo = (PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC*)*tablePtrPtr;

    memAllocIndex = 0;

    tmpRootRange.lowerLpmPtr.nextBucket =
    ipShadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];
    tmpRootRange.pointerType =
    (GT_U8)(ipShadowPtr->vrRootBucketArray[vrId].rootBucketType[protocolStack]);
    tmpRootRange.next = NULL;
    retVal = prvCpssDxChLpmRamMemTraverse(PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_SET_WITH_PCL_E,
                                          memAllocInfo,
                                          &memAllocIndex,
                                          numOfEntriesLeft,
                                          &tmpRootRange,
                                          GT_FALSE,
                                          ipShadowPtr->ipUcSearchMemArrayPtr[protocolStack],
                                          iterPtr);

    /* update the left num of entries */
    numOfEntriesLeft = numOfEntriesLeft - memAllocIndex;

    *dataSizePtr = *dataSizePtr - memAllocIndex * (sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));

    /* advance the pointer */
    *tablePtrPtr = (GT_VOID*)(&memAllocInfo[memAllocIndex]);

    return (retVal);
}

/**
* @internal ipLpmDbSetL3McRoutes function
* @endinternal
*
* @brief   Set the data needed for core shadow reconstruction used after HSU.
*
* @param[in] ipShadowPtr              - points to the shadow to set in
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbSetL3McRoutes
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *ipShadowPtr,
    IN     GT_U32                               vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN     GT_VOID                              **tablePtrPtr,
    INOUT  GT_UINTPTR                           *iterPtr,
    INOUT  GT_U32                               *dataSizePtr
)
{
    GT_STATUS                               retVal = GT_OK;
    GT_U8                                   *groupIp = NULL;
    GT_U8                                   *srcAddr = NULL;
    GT_U32                                  groupPrefix,srcPrefix;

    /* regarding group scope, here we give a false one, it will
       updated as part of the traverse function on the ipv6 groups */
    CPSS_IPV6_PREFIX_SCOPE_ENT          groupScope =
                                            CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
    CPSS_EXMXPM_IP_MC_ADDRESS_TYPE_ENT  addressType =
                                            CPSS_EXMXPM_IP_MC_REGULAR_ADDRESS_TYPE_E;

    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *mcEntry;

    GT_U32 numOfEntries = 0;
    GT_U32 numOfEntriesLeft = 0;

    IPV6_HSU_MC_ROUTE_ENTRY_STC *ipv6McRoute;
    IPV4_HSU_MC_ROUTE_ENTRY_STC *ipv4McRoute;

    GT_BOOL isLast;

    ipv4McRoute = (IPV4_HSU_MC_ROUTE_ENTRY_STC*)*tablePtrPtr;
    ipv6McRoute = (IPV6_HSU_MC_ROUTE_ENTRY_STC*)*tablePtrPtr;

    isLast = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
             ipv4McRoute->last:ipv6McRoute->last;

     if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
         numOfEntriesLeft = *dataSizePtr/(sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC));
     else
         numOfEntriesLeft = *dataSizePtr/(sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
    {
        groupIp =  (GT_U8 *)cpssOsMalloc(sizeof(GT_U8)*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS);
        srcAddr =  (GT_U8 *)cpssOsMalloc(sizeof(GT_U8)*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS);
    }
    else
    {
        groupIp =  (GT_U8 *)cpssOsMalloc(sizeof(GT_U8)*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS);
        srcAddr =  (GT_U8 *)cpssOsMalloc(sizeof(GT_U8)*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS);
    }
     cpssOsMemSet(groupIp, 0, sizeof(groupIp));
     cpssOsMemSet(srcAddr, 0, sizeof(srcAddr));


    while ((isLast == GT_FALSE) && (numOfEntries < numOfEntriesLeft))
    {
        if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
        {
            cpssOsMemCpy(groupIp,ipv4McRoute->mcGroup.arIP,sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS);
            groupPrefix = (ipv4McRoute->mcGroup.u32Ip == 0)?0:32;

            cpssOsMemCpy(srcAddr,ipv4McRoute->ipSrc.arIP,sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS);
            srcPrefix = ipv4McRoute->ipSrcPrefixlength;

            vrId = ipv4McRoute->vrId;
            mcEntry = &ipv4McRoute->mcRoutePointer;

            ipv4McRoute++;

            isLast = ipv4McRoute->last;
        }
        else
        {
            cpssOsMemCpy(groupIp,ipv6McRoute->mcGroup.arIP,sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS);
            groupPrefix = ((ipv6McRoute->mcGroup.u32Ip[0] == 0) &&
                           (ipv6McRoute->mcGroup.u32Ip[1] == 0) &&
                           (ipv6McRoute->mcGroup.u32Ip[2] == 0) &&
                           (ipv6McRoute->mcGroup.u32Ip[3] == 0))?0:128;

            cpssOsMemCpy(srcAddr,ipv6McRoute->ipSrc.arIP,sizeof(GT_U8) * PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS);
            srcPrefix = ipv6McRoute->ipSrcPrefixlength;

            vrId = ipv6McRoute->vrId;
            mcEntry = &ipv6McRoute->mcRoutePointer;

            ipv6McRoute++;

            isLast = ipv6McRoute->last;
        }

        /* we are overriding an entry that exists in core*/
        retVal = prvCpssDxChLpmRamMcEntryAdd(vrId,groupIp,groupPrefix,addressType,
                                             0,srcAddr,srcPrefix,
                                             mcEntry,GT_FALSE,
                                             PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E,
                                             protocolStack,ipShadowPtr);
        if (retVal != GT_OK)
        {
            cpssOsFree(groupIp);
            cpssOsFree(srcAddr);
            return (retVal);
        }

        numOfEntries++;

    }

    /* check if we finished */
    if ( (isLast == GT_TRUE) && (numOfEntries < numOfEntriesLeft) )
    {
        /* just indicate we finished using a fake iterator */
        *iterPtr = 0;

        /* advance the pointer */
        *tablePtrPtr = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
            (GT_VOID *)(ipv4McRoute + 1) : (GT_VOID *)(ipv6McRoute + 1);
        numOfEntries++;
    }
    else
    {
        /* just indicate we didn't finish using a fake iterator */
        *iterPtr = 0xff;

        /* and record the pointer */
        *tablePtrPtr = (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)?
            (GT_VOID *)ipv4McRoute : (GT_VOID *)ipv6McRoute;
    }

    /* update the num of entries left */
    numOfEntriesLeft -= numOfEntries;

     if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E)
         *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(IPV4_HSU_MC_ROUTE_ENTRY_STC));
     else
         *dataSizePtr = *dataSizePtr - numOfEntries * (sizeof(IPV6_HSU_MC_ROUTE_ENTRY_STC));
    cpssOsFree(groupIp);
    cpssOsFree(srcAddr);
    return (retVal);
}

/**
* @internal ipLpmDbSetL3McLpm function
* @endinternal
*
* @brief   Set the data needed for core shadow reconstruction used after HSU.
*
* @param[in] ipShadowPtr              - points to the shadow to set in
* @param[in] vrId                     - the virtual router id
* @param[in] protocolStack            - type of ip protocol stack to work on.
* @param[in] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbSetL3McLpm
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *ipShadowPtr,
    IN     GT_U32                               vrId,
    IN     CPSS_IP_PROTOCOL_STACK_ENT           protocolStack,
    IN     GT_VOID                              **tablePtrPtr,
    INOUT  GT_UINTPTR                           *iterPtr,
    INOUT  GT_U32                               *dataSizePtr
)
{
    GT_STATUS                                   retVal = GT_OK;
    PRV_CPSS_EXMXPM_IPV6MC_PCL_ENTRY_STC        ipv6McPclEntry,*ipv6McPclEntryPtr;
    GT_U32                                      memAllocIndex = 0;
    GT_U32                                      numOfEntriesLeft = 0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC    *memAllocInfo = NULL;
    /* set the pointer for lpm traverse */

    /* check if mc prefixes are exist */
    if (protocolStack == CPSS_IP_PROTOCOL_IPV4_E )
    {
        if(ipShadowPtr->vrRootBucketArray[vrId].mcRootBucket[CPSS_IP_PROTOCOL_IPV4_E] == NULL)
        {
            /* no ipv4 mc prefixes */
            return GT_OK;
        }
    }
    if (protocolStack == CPSS_IP_PROTOCOL_IPV6_E )
    {
        if(ipShadowPtr->vrRootBucketArray[vrId].mcRootBucket[CPSS_IP_PROTOCOL_IPV6_E] == NULL)
        {
            /* no ipv4 mc prefixes */
            return GT_OK;
        }
    }
    memAllocInfo = (PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC*)*tablePtrPtr;
    memAllocIndex = 0;

    numOfEntriesLeft = *dataSizePtr/(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));

    if (numOfEntriesLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* move on to mc LPM*/
    retVal = prvCpssDxChLpmRamMcTraverse(PRV_CPSS_DXCH_LPM_RAM_MEM_TRAVERSE_SET_WITH_PCL_E,
                                         memAllocInfo,&memAllocIndex,
                                         numOfEntriesLeft,vrId,ipShadowPtr,
                                         iterPtr);
    /* advance the pointer */
    *tablePtrPtr = (GT_VOID*)(&memAllocInfo[memAllocIndex]);

    /* update the left num of entries */
    numOfEntriesLeft = numOfEntriesLeft - memAllocIndex;

    *dataSizePtr = *dataSizePtr - memAllocIndex * (sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_ALLOC_INFO_STC));

    return (retVal);
}

/**
* @internal ipLpmDbSetL3DmmFreeLists function
* @endinternal
*
* @brief   Set needed dmm free lists in dmm manager .
*
* @param[in] ipShadowPtr              - points to the shadow to retrive from
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes supposed to be processed
*                                      in current iteration step
*
* @param[out] tablePtrPtr              - points to the table size info block
* @param[in,out] iterPtr                  - points to the current iter
* @param[in,out] dataSizePtr              - points to data size in bytes left after iteration step.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS ipLpmDbSetL3DmmFreeLists
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *ipShadowPtr,
    OUT    GT_VOID                          **tablePtrPtr,
    INOUT  GT_UINTPTR                       *iterPtr,
    INOUT  GT_U32                           *dataSizePtr
)
{
    GT_U32 numberInternalEntiresLeft = 0;
    GT_U32 numberInternalEntriesUsed = 0;
    DMM_IP_HSU_ENTRY_STC *dmmIterEntry;
    GT_DMM_BLOCK *tempDmmPtr = NULL;
    GT_DMM_BLOCK *foundDmmBlock = NULL;
    GT_DMM_BLOCK *tempNextBySize = NULL;
    GT_DMM_BLOCK *tempPrevBySize = NULL;
    GT_DMM_BLOCK *tempCurrentNextBySize = NULL;
    GT_BOOL updateSmallFreeListArray = GT_FALSE;
    GT_BOOL updateBigFreeList = GT_FALSE;
    GT_U32 tempListName = 0;
    GT_U32 i = 0;
    GT_U32 j = 0;
    GT_U32 *hsuMemPtr;
    GT_U32 data;

    if (*iterPtr == 0)
    {
        /* this is the first call , so intilize */
        *iterPtr = (GT_UINTPTR)cpssOsLpmMalloc(sizeof(DMM_IP_HSU_ENTRY_STC));
        if (*iterPtr == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        dmmIterEntry = (DMM_IP_HSU_ENTRY_STC*)(*iterPtr);
        dmmIterEntry->currentDmmBlockAddrInList = NULL;
        dmmIterEntry->currentFreeLinkList = 0xffffffff;
        dmmIterEntry->partitionIndex = 0;
        dmmIterEntry->endOfList = 0;
        dmmIterEntry->endOfPartition = GT_TRUE;
        cpssOsMemSet(dmmIterEntry->partitionArray,0,sizeof(dmmIterEntry->partitionArray));

        /* need to change logic, since for bobcat2 and above we support multi blocks mapping per octet*/
        CPSS_TBD_BOOKMARK

        /*this code need to be checked and the next code should be deleted*/

        for (j = 0, i= 0; (j <8) && (i < ipShadowPtr->numOfLpmMemories); i++, j++;)
        {
           dmmIterEntry->partitionArray[j] = (GT_DMM_PARTITION*)ipShadowPtr->lpmRamStructsMemPoolPtr[i];
        }

        /* init dmm partition array. Partition array should contain only different partition Ids */
        /*for (j = 0, i= 0; (j <8) && (i < ipShadowPtr->numOfLpmMemories); i++)
        {
           if (ipShadowPtr->lpmMemInfoArray[0][i].structsMemPool ==
               ipShadowPtr->lpmMemInfoArray[1][i].structsMemPool)
           {
               dmmIterEntry->partitionArray[j] = (GT_DMM_PARTITION*)ipShadowPtr->lpmMemInfoArray[0][i].structsMemPool;
               j++;
           }
           else
           {
               dmmIterEntry->partitionArray[j] = (GT_DMM_PARTITION*)ipShadowPtr->lpmMemInfoArray[0][i].structsMemPool;
               dmmIterEntry->partitionArray[j+1] = (GT_DMM_PARTITION*)ipShadowPtr->lpmMemInfoArray[1][i].structsMemPool;
               j = j+2;
           }
        }
        */
    }
    else
    {
        dmmIterEntry = (DMM_IP_HSU_ENTRY_STC*)(*iterPtr);
    }

    numberInternalEntiresLeft = *dataSizePtr/(sizeof(GT_U32));

    if (numberInternalEntiresLeft == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    /* set the pointer */

    hsuMemPtr = (GT_U32*)(*tablePtrPtr);
    j = dmmIterEntry->partitionIndex;
    for (; j < 8; j ++)
    {
        if (dmmIterEntry->partitionArray[j] == NULL)
        {
            continue;
        }

        /* partition index in the begining of each partition data */
        /* first reading of new partition block */
        if (dmmIterEntry->endOfPartition == GT_TRUE)
        {
            dmmIterEntry->partitionIndex = j;
            if (numberInternalEntiresLeft > numberInternalEntriesUsed )
            {
                data = *hsuMemPtr;
                hsuMemPtr++;
                numberInternalEntriesUsed++;
                dmmIterEntry->endOfPartition = GT_FALSE;
            }
            else
            {
                *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                return GT_OK;
            }
            if (dmmIterEntry->partitionIndex != data)
            {
                /* export and import are not synchronized */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        while (1)
        {
            if (numberInternalEntiresLeft > numberInternalEntriesUsed )
            {
                data = *hsuMemPtr;
                hsuMemPtr++;
                numberInternalEntriesUsed++;
            }
            else
            {
                *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
                return GT_OK;
            }
            if (data == PRV_CPSS_DXCH_LPM_RAM_HSU_DMM_PARTITION_END_CNS)
            {
                /* partition is done */
                dmmIterEntry->endOfPartition = GT_TRUE;
                dmmIterEntry->currentFreeLinkList = 0xffffffff;
                break;
            }

            if (data == PRV_CPSS_DXCH_LPM_RAM_HSU_DMM_FREE_LINK_LIST_END_CNS)
            {
                /* this list was finished */
                dmmIterEntry->currentFreeLinkList = 0xffffffff;
                continue;
            }
            /* if iteration mode is activated it is importent to understand when
               link list name is read and when hw offset is read */
            if (dmmIterEntry->currentFreeLinkList == 0xffffffff)
            {
                dmmIterEntry->currentFreeLinkList = data;
                dmmIterEntry->currentDmmBlockAddrInList = NULL;
            }
            else
            {
                tempListName = dmmIterEntry->currentFreeLinkList;
                /* hw offset was read -- set it in existing list */
                if (dmmIterEntry->currentDmmBlockAddrInList == NULL)
                {
                    /* first hw offset in this list */
                    if (tempListName <= DMM_MAXIMAL_BLOCK_SIZE_ALLOWED)
                    {
                        updateSmallFreeListArray = GT_TRUE;
                        dmmIterEntry->currentDmmBlockAddrInList = dmmIterEntry->partitionArray[dmmIterEntry->partitionIndex]->tableOfSizePointers[tempListName];
                    }
                    else
                    {
                        /* big blocks */
                        updateBigFreeList = GT_TRUE;
                        dmmIterEntry->currentDmmBlockAddrInList = dmmIterEntry->partitionArray[dmmIterEntry->partitionIndex]->bigBlocksList;
                    }
                }
                /* find the element with received hw offset in current list */

                if (dmmIterEntry->currentDmmBlockAddrInList == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
                /* except of case when  first hw offset in the list was read
                  currentDmmBlockAddrInList is already processed so we start
                  from the next. If next dmm block == NULL, it is bad state:
                  list is done but end of list still wasn't arrived  */

                if ( (updateSmallFreeListArray == GT_TRUE) || (updateBigFreeList == GT_TRUE) )
                {
                    tempDmmPtr = dmmIterEntry->currentDmmBlockAddrInList;
                }
                else
                {
                    tempDmmPtr = dmmIterEntry->currentDmmBlockAddrInList->nextBySizeOrPartitionPtr.nextBySize;
                }
                if (tempDmmPtr == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
                foundDmmBlock = NULL;
                while (tempDmmPtr != NULL)
                {
                    if (PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(tempDmmPtr) == data)
                    {
                        foundDmmBlock = tempDmmPtr;
                        break;
                    }
                    tempDmmPtr = tempDmmPtr->nextBySizeOrPartitionPtr.nextBySize;
                }
                if (foundDmmBlock == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                if (foundDmmBlock->prevBySize == NULL)
                {
                    /* the found block with correct hw offset is first in the list */
                    /* it is the right place - no move is needed */
                    updateSmallFreeListArray = GT_FALSE;
                    updateBigFreeList = GT_FALSE;
                    dmmIterEntry->currentDmmBlockAddrInList = foundDmmBlock;
                    continue;
                }
                if ( (updateSmallFreeListArray == GT_FALSE) && (updateBigFreeList == GT_FALSE) )
                {
                    /* after first iteration in list is done and first currentDmmBlockAddrInList
                       moved on its constant place */
                    if (foundDmmBlock->prevBySize == dmmIterEntry->currentDmmBlockAddrInList)
                    {
                        /* the foundDmmBlock block is not first in the list. This is the
                           case when founded block should be located just after previously
                           found block. So if this block points by prevBySize on the
                           currentDmmBlockAddrInList --  it is the right place - no move is needed */
                        dmmIterEntry->currentDmmBlockAddrInList = foundDmmBlock;
                        continue;
                    }
                }
                /* the block with the same hw offset is found */
                tempDmmPtr = dmmIterEntry->currentDmmBlockAddrInList;
                tempCurrentNextBySize = tempDmmPtr->nextBySizeOrPartitionPtr.nextBySize;
                tempNextBySize = foundDmmBlock->nextBySizeOrPartitionPtr.nextBySize;
                tempPrevBySize = foundDmmBlock->prevBySize;

                if ( (updateSmallFreeListArray == GT_TRUE) || (updateBigFreeList == GT_TRUE) )
                {
                    /* set this block as first in the list */
                    if (updateSmallFreeListArray == GT_TRUE)
                    {
                        dmmIterEntry->partitionArray[j]->tableOfSizePointers[tempListName] = foundDmmBlock;
                    }
                    else
                    {
                        dmmIterEntry->partitionArray[j]->bigBlocksList = foundDmmBlock;
                    }
                    foundDmmBlock->prevBySize = NULL;
                    foundDmmBlock->nextBySizeOrPartitionPtr.nextBySize = tempDmmPtr;
                    tempDmmPtr->prevBySize = foundDmmBlock;
                    updateSmallFreeListArray = GT_FALSE;
                    updateBigFreeList = GT_FALSE;
                }
                else
                {
                    /* set this block after current block in the list */
                    foundDmmBlock->prevBySize = tempDmmPtr;
                    foundDmmBlock->nextBySizeOrPartitionPtr.nextBySize = tempDmmPtr->nextBySizeOrPartitionPtr.nextBySize;
                    tempDmmPtr->nextBySizeOrPartitionPtr.nextBySize = foundDmmBlock;
                    tempCurrentNextBySize->prevBySize = foundDmmBlock;
                }
                if (tempNextBySize != NULL)
                {
                    /* last element in the list */
                    tempNextBySize->prevBySize = tempPrevBySize;
                }
                tempPrevBySize->nextBySizeOrPartitionPtr.nextBySize = tempNextBySize;
                dmmIterEntry->currentDmmBlockAddrInList = foundDmmBlock;
            }
        }
    }
    /* in case all data was processed */
    /* update the left num of entries */

    numberInternalEntiresLeft -= numberInternalEntriesUsed;

    *dataSizePtr = *dataSizePtr - numberInternalEntriesUsed * sizeof(GT_U32);
    /* free & zero the iterator */
    cpssOsLpmFree((GT_PTR)*iterPtr);
    *iterPtr = 0;

    return GT_OK;
}

/**
* @internal ipLpmDbSetL3 function
* @endinternal
*
* @brief   Set the data needed for core IP shadow reconstruction used after HSU
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] tablePtr                 - points to the table size info block
* @param[in,out] iterationSizePtr         - points to data size in bytes supposed to be processed
*                                      in current iteration
* @param[in,out] iterPtr                  - points to the iterator, to start - set to 0.
* @param[in,out] iterationSizePtr         - points to data size in bytes left after iteration step.
* @param[in,out] iterPtr                  - points to the iterator, if = 0 then the operation is done.
*
* @retval GT_OK                    - on success
*/
GT_STATUS ipLpmDbSetL3
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN    GT_VOID                               *tablePtr,
    INOUT GT_U32                                *iterationSizePtr,
    INOUT GT_UINTPTR                            *iterPtr
)
{
    GT_STATUS retVal = GT_OK;
    IP_HSU_ITERATOR_STC *currentIterPtr;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *ipShadowPtr;
    GT_U32                              *currDataSizePtr = iterationSizePtr;
    GT_U32                              curentDataSizeBeforeStage;

    currentIterPtr = (IP_HSU_ITERATOR_STC*)*iterPtr;

    if (currentIterPtr == NULL)
    {
        /* we need to allocate the iterator */
        currentIterPtr =
        (IP_HSU_ITERATOR_STC*)cpssOsLpmMalloc(sizeof(IP_HSU_ITERATOR_STC));
        if (currentIterPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        /*reset it */
        cpssOsMemSet(currentIterPtr,0,sizeof(IP_HSU_ITERATOR_STC));
        /* set the starting stage */
        currentIterPtr->currStage = IP_HSU_VR_STAGE_E;
       *iterPtr = (GT_UINTPTR)currentIterPtr;
    }
     /* set the memory position */
        currentIterPtr->currMemPtr = tablePtr;
    for (; currentIterPtr->currShadowIdx < lpmDbPtr->numOfShadowCfg;
        currentIterPtr->currShadowIdx++)
    {
        ipShadowPtr = &lpmDbPtr->shadowArray[currentIterPtr->currShadowIdx];

        if (currentIterPtr->currStage == IP_HSU_VR_STAGE_E)
        {
            curentDataSizeBeforeStage = *currDataSizePtr;
            retVal = ipLpmDbSetL3Vr(ipShadowPtr,
                                    &currentIterPtr->currMemPtr,
                                    &currentIterPtr->currIter,
                                    currDataSizePtr);
            if ((retVal == GT_NO_RESOURCE)&&
                (*currDataSizePtr == curentDataSizeBeforeStage))
            {
                /* the stage is not done: the data size was smaller than entry in shadow */
                return GT_OK;
            }
            /* check if we finished with the phase */
            if (currentIterPtr->currIter == 0)
            {
                /* set the next stage */
                currentIterPtr->currStage = IP_HSU_UC_PREFIX_STAGE_E;
            }

            if ((retVal != GT_OK) || (*currDataSizePtr == 0))
            {
                return (retVal);
            }
        }
        if (currentIterPtr->currStage != IP_HSU_DMM_FREE_LIST_E)
        {
            for (; currentIterPtr->currProtocolStack < 2; currentIterPtr->currProtocolStack++)
            {
                if (ipShadowPtr->isIpVerIsInitialized[currentIterPtr->currProtocolStack] == GT_FALSE)
                {
                    continue;
                }

                while (currentIterPtr->currVrId < ipShadowPtr->vrfTblSize)
                {
                    /* go over all valid VR */
                    if ((ipShadowPtr->vrRootBucketArray[currentIterPtr->currVrId].valid == GT_FALSE) ||
                        (ipShadowPtr->vrRootBucketArray[currentIterPtr->currVrId].rootBucket == NULL))
                    {
                        currentIterPtr->currVrId++;
                        continue;
                    }

                    if (currentIterPtr->currStage == IP_HSU_UC_PREFIX_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbSetL3UcPrefix(ipShadowPtr,
                                                      currentIterPtr->currVrId,
                                                      currentIterPtr->currProtocolStack,
                                                      &currentIterPtr->currMemPtr,
                                                      &currentIterPtr->currIter,
                                                      currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }
                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_UC_LPM_STAGE_E;
                        }

                        if ((retVal != GT_OK) || (*currDataSizePtr == 0))
                        {
                            return (retVal);
                        }
                    }

                    if (currentIterPtr->currStage == IP_HSU_UC_LPM_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbSetL3UcLpm(ipShadowPtr,
                                                   currentIterPtr->currVrId,
                                                   currentIterPtr->currProtocolStack,
                                                   &currentIterPtr->currMemPtr,
                                                   &currentIterPtr->currIter,
                                                   currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }

                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_MC_PREFIX_STAGE_E;
                        }

                        if ((retVal != GT_OK) || (*currDataSizePtr == 0))
                        {
                            return (retVal);
                        }

                    }

                    if (currentIterPtr->currStage == IP_HSU_MC_PREFIX_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbSetL3McRoutes(ipShadowPtr,
                                                      currentIterPtr->currVrId,
                                                      currentIterPtr->currProtocolStack,
                                                      &currentIterPtr->currMemPtr,
                                                      &currentIterPtr->currIter,
                                                      currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }
                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_MC_LPM_STAGE_E;
                        }

                        if ((retVal != GT_OK) || (*currDataSizePtr == 0))
                        {
                            return (retVal);
                        }
                    }
                    if (currentIterPtr->currStage == IP_HSU_MC_LPM_STAGE_E)
                    {
                        curentDataSizeBeforeStage = *currDataSizePtr;
                        retVal = ipLpmDbSetL3McLpm(ipShadowPtr,
                                                   currentIterPtr->currVrId,
                                                   currentIterPtr->currProtocolStack,
                                                   &currentIterPtr->currMemPtr,
                                                   &currentIterPtr->currIter,
                                                   currDataSizePtr);
                        if ((retVal == GT_NO_RESOURCE)&&
                            (*currDataSizePtr == curentDataSizeBeforeStage))
                        {
                            /* the stage is not done: the data size was smaller than entry in shadow */
                            return GT_OK;
                        }
                        /* check if we finished with the phase */
                        if (currentIterPtr->currIter == 0)
                        {
                            /* set the next stage */
                            currentIterPtr->currStage = IP_HSU_UC_PREFIX_STAGE_E;

                            /* we finished with this vrid go to the next */
                            currentIterPtr->currVrId++;
                        }
                        if (retVal != GT_OK)
                        {
                            return (retVal);
                        }
                    }
                }
                /* we finished with this protocol stack zero the vrid */
                currentIterPtr->currVrId = 0;
            }

        }

        /* the last stage is IP_HSU_DMM_FREE_LIST_E */
        currentIterPtr->currStage = IP_HSU_DMM_FREE_LIST_E;
        if (currentIterPtr->currStage == IP_HSU_DMM_FREE_LIST_E)
        {
            curentDataSizeBeforeStage = *currDataSizePtr;
            retVal = ipLpmDbSetL3DmmFreeLists(ipShadowPtr,
                                              &currentIterPtr->currMemPtr,
                                              &currentIterPtr->currIter,
                                              currDataSizePtr);
            if ((retVal == GT_NO_RESOURCE)&&
                (*currDataSizePtr == curentDataSizeBeforeStage))
            {
                /* the stage is not done: the data size was smaller than entry in shadow */
                return GT_OK;
            }
            /* check if we finished with the phase */
            if (currentIterPtr->currIter == 0)
            {
                /* set the next stage */
                currentIterPtr->currStage = IP_HSU_UC_PREFIX_STAGE_E;
            }
            if ((retVal != GT_OK) || ( currentIterPtr->currIter != 0))
            {
                /*osSemSignal(ipShadowPtr->ipUnitSem);*/
                return(retVal);
            }
        }
        /* we finished with the shadow , set the starting stage */
        currentIterPtr->currStage = IP_HSU_VR_STAGE_E;
    }
    /* we reached the end free the iterator */
    cpssOsLpmFree(currentIterPtr);
    *iterPtr = 0;

    return (GT_OK);
}

/**
* @internal cpssExMxPmIpLpmDbMemSizeGet function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function calculates the memory size needed to export the Lpm DB
*         (used for HSU and unit Hotsync)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
*
* @param[out] lpmDbSizePtr             - pointer to lpmDb size calculated in bytes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssExMxPmIpLpmDbMemSizeGet
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC    *lpmDbPtr,
    OUT GT_U32                                  *lpmDbSizePtr
)
{
    GT_UINTPTR iterPtr = 0;
    GT_U32 iterationSise = 0xffffffff;/* perform size calculating in one iteration*/
    return ipLpmDbGetL3(lpmDbPtr,lpmDbSizePtr,NULL,&iterationSise,&iterPtr);
}

/**
* @internal cpssExMxPmIpLpmDbExport function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function exports the Lpm DB into the preallocated memory,
*         (used for HSU and unit Hotsync)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in,out] lpmDbMemBlockSizePtr     - pointer to block data size supposed to be exported
*                                      in current iteration.
* @param[in] lpmDbMemBlockPtr         - pointer to allocated for lpm DB memory area
* @param[in,out] iterPtr                  - the iterator, to start - set to 0.
* @param[in,out] lpmDbMemBlockSizePtr     - pointer to block data size that was not used
*                                      in current iteration.
* @param[in,out] iterPtr                  - the iterator, if = 0 then the operation is done.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Change behaving: from this version treatment of lpmDbMemBlockPtr was changed.
*       Now assumption is that it points on concrete block allocated only for
*       this iteration and not on big contiguous memory block for whole export/import
*       operation. User working with contiguous memory should give exact pointer
*       for next iteration: previous pointer + calculated iteration size.
*
*/
GT_STATUS cpssExMxPmIpLpmDbExport
(
    IN     PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    INOUT  GT_U32                               *lpmDbMemBlockSizePtr,
    IN     GT_U32                               *lpmDbMemBlockPtr,
    INOUT  GT_UINTPTR                           *iterPtr
)
{
    return ipLpmDbGetL3(lpmDbPtr,NULL,lpmDbMemBlockPtr,lpmDbMemBlockSizePtr,iterPtr);
}

/**
* @internal cpssExMxPmIpLpmDbImport function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function imports the Lpm DB recived and reconstruct it,
*         (used for HSU and unit Hotsync)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in,out] lpmDbMemBlockSizePtr     - pointer to block data size supposed to be imported
*                                      in current iteration.
* @param[in] lpmDbMemBlockPtr         - pointer to allocated for lpm DB memory area
* @param[in,out] iterPtr                  - the iterator, to start - set to 0.
* @param[in,out] lpmDbMemBlockSizePtr     - pointer to block data size that was not used
*                                      in current iteration.
* @param[in,out] iterPtr                  - the iterator, if = 0 then the operation is done.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Change behaving: from this version treatment of lpmDbMemBlockPtr was changed.
*       Now assumption is that it points on concrete block allocated only for
*       this iteration and not on big contiguous memory block for whole export/import
*       operation. User working with contiguous memory should give exact pointer
*       for next iteration: previous pointer + calculated iteration size.
*
*/
GT_STATUS cpssExMxPmIpLpmDbImport
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    INOUT GT_U32                                *lpmDbMemBlockSizePtr,
    IN    GT_U32                                *lpmDbMemBlockPtr,
    INOUT GT_UINTPTR                            *iterPtr
)
{
    return ipLpmDbSetL3(lpmDbPtr,lpmDbMemBlockPtr,lpmDbMemBlockSizePtr,iterPtr);
}
#endif

/**
* @internal prvCpssDxChLpmRamMemFreeListMng function
* @endinternal
*
* @brief   This function is used to collect all Pp Narrow Sram memory free
*         operations inorder to be freed at the end of these operations.
* @param[in] ppMemAddr                - the address in the HW (the device memory pool) to record.
* @param[in] operation                - the  (see
*                                      PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
*                                      PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
*                                      PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E
* @param[in] memBlockListPtrPtr       - (pointer to) the memory block list to act upon.
* @param[in] shadowPtr                - (pointer to) the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMemFreeListMng
(
    IN  GT_UINTPTR                                  ppMemAddr,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_OP_ENT       operation,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC    **memBlockListPtrPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC  *ppMemEntry,*tmpPtr; /* Pp memory list entry.    */
    GT_STATUS                   retVal = GT_OK;     /* Functions return value.      */
    GT_U32                      blockIndex=0;/* calculated according to the ppMemAddr*/
    GT_U32                      sizeOfMemoryBlockInlines=0;/* calculated according to the memory handle */
    GT_BOOL                     oldUpdateDec=GT_FALSE;/* for reconstruct */
    GT_U32                      oldNumOfDecUpdates=0;/* for reconstruct */
    switch(operation)
    {
    case (PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E):
        /* make sure it an empty list */
        if (*memBlockListPtrPtr != NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

        break;

    case (PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E):
        ppMemEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC));
        if (ppMemEntry == NULL)
        {
            retVal = GT_OUT_OF_CPU_MEM;
        }
        else
        {
            ppMemEntry->memAddr  = ppMemAddr;
            ppMemEntry->next = *memBlockListPtrPtr;
            *memBlockListPtrPtr = ppMemEntry;

        }
        break;

    case (PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E):
        ppMemEntry = *memBlockListPtrPtr;
        while (ppMemEntry != NULL)
        {
            /* create a pending list of blocks that will be used to
               update protocolCountersPerBlockArr */
            if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
            {
                if(ppMemEntry->memAddr==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR - Can not have ppMemEntry->memAddr ==0, in prvCpssDxChLpmRamMemFreeListMng");
                }
                /* the block index updated out of 20 blocks*/
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_OFFSET_FROM_DMM_MAC(ppMemEntry->memAddr)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                sizeOfMemoryBlockInlines = PRV_CPSS_DXCH_LPM_RAM_GET_LPM_SIZE_FROM_DMM_MAC(ppMemEntry->memAddr);
            }
            else
            {
                if (ppMemEntry->memAddr==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected ppMemEntry->memAddr=0\n");
                }
                /* the block index updated out of 20 blocks*/
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(ppMemEntry->memAddr)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                retVal = prvCpssDxChLpmRamSip6CalcBankNumberIndex(shadowPtr,&blockIndex);
                if (retVal != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                }
                sizeOfMemoryBlockInlines = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_SIZE_FROM_DMM_MAC(ppMemEntry->memAddr);
            }

            /* keep values incase reconstruct is needed */
            oldUpdateDec = shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec;
            oldNumOfDecUpdates = shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates;

            /* set pending flag for future need */
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates += sizeOfMemoryBlockInlines;

            if (prvCpssDmmFree(ppMemEntry->memAddr) == 0)
            {
                /* reconstruct */
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec = oldUpdateDec;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates = oldNumOfDecUpdates;

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            tmpPtr = ppMemEntry;
            ppMemEntry = ppMemEntry->next;

            cpssOsLpmFree(tmpPtr);
        }
        *memBlockListPtrPtr = NULL;
        break;
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamUpdateVrTableFuncWrapper function
* @endinternal
*
* @brief   This function is a wrapper to PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PTR
*
* @param[in] data                     - the parmeters for PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PTR
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUpdateVrTableFuncWrapper
(
    IN  GT_PTR                  data
)
{
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC *params;

    params = (PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC*)data;

    return prvCpssDxChLpmRamMngVrfEntryUpdate(params->vrId, params->protocol,
                                              params->shadowPtr);
}

/**
* @internal prvCpssDxChLpmRouteEntryPointerCheck function
* @endinternal
*
* @brief   Check validity of values of route entry pointer
*
* @param[in] routeEntryPointerPtr     - route entry pointer to check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChLpmRouteEntryPointerCheck
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT          shadowType,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  *routeEntryPointerPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(routeEntryPointerPtr);

    if ((routeEntryPointerPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E) &&
        (routeEntryPointerPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E) &&
        (routeEntryPointerPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_MULTIPATH_E) &&
        (routeEntryPointerPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "routeEntryMethod: not supported type ");
    }

    if (shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
        if ((routeEntryPointerPtr->routeEntryMethod == PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E) ||
            (routeEntryPointerPtr->routeEntryMethod == PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "routeEntryMethod : Sip6 not supports types : 'ECMP' , 'QOS' ");
        }
    }
    /* validate the ipv6 MC group scope level */
    switch (routeEntryPointerPtr->ipv6McGroupScopeLevel)
    {
        case CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E:
        case CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E:
        case CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E:
        case CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E:
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "groupScopeLevel : not supported type ");

    }
    if (shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
        switch (routeEntryPointerPtr->priority)
        {
            case PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E:
            case PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "priority : not supported type ");
        }

    }
    /* note that since LPM DB can exist without any devices added to it, there
       is no way to know how many route entries or ECMP/QoS entries are
       available in specific device; therefore neither the route entries base
       address nor the size of the ECMP/QoS block value can't be checked */

    return GT_OK;
}

#if 0
/**
* @internal prvCpssExMxPmLpmDbIdGetNext function
* @endinternal
*
* @brief   This function retrieve next LPM DB ID from LPM DBs Skip List
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in,out] slIteratorPtr            - The iterator Id that was returned from the last call to
*                                      this function.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NO_MORE               - on absence of elements in skip list
*
* @note none.
*
*/
GT_STATUS prvCpssExMxPmLpmDbIdGetNext
(
    OUT     GT_U32      *lpmDbIdPtr,
    INOUT   GT_UINTPTR  *slIteratorPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_LPM_DB_STC *lpmDbPtr = NULL;
    CPSS_NULL_PTR_CHECK_MAC(slIteratorPtr);
    CPSS_NULL_PTR_CHECK_MAC(lpmDbIdPtr);

    lpmDbPtr = (PRV_CPSS_DXCH_LPM_RAM_LPM_DB_STC *)prvCpssSlGetNext(lpmDbSL,slIteratorPtr);
    if (lpmDbPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
    }
    *lpmDbIdPtr = lpmDbPtr->lpmDBId;
    return GT_OK;
}
#endif

/**
* @internal prvCpssDxChIpLpmRamIpv4UcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 UC prefix for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipPtr                    - Pointer to the ip address to look for.
* @param[in] prefixLen                - ipAddr prefix length.
* @param[in] clearActivity            - Indicates to clear activity status.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChIpLpmRamIpv4UcPrefixActivityStatusGet
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *ipPtr,
    IN  GT_U32                              prefixLen,
    IN  GT_BOOL                             clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    if(shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
        return prvCpssDxChIpLpmRamUcPrefixActivityStatusGet(
                    vrId,
                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                    ipPtr,
                    prefixLen,
                    clearActivity,
                    shadowPtr,
                    activityStatusPtr);
    }
    else
    {
        return prvCpssDxChIpLpmSip6RamUcPrefixActivityStatusGet(
                    vrId,
                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                    ipPtr,
                    prefixLen,
                    clearActivity,
                    shadowPtr,
                    activityStatusPtr);

    }
}

/**
* @internal prvCpssDxChIpLpmRamIpv6UcPrefixActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 UC prefix for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipPtr                    - Pointer to the ip address to look for.
* @param[in] prefixLen                - ipAddr prefix length.
* @param[in] clearActivity            - Indicates to clear activity status.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChIpLpmRamIpv6UcPrefixActivityStatusGet
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *ipPtr,
    IN  GT_U32                              prefixLen,
    IN  GT_BOOL                             clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    if(shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
        return prvCpssDxChIpLpmRamUcPrefixActivityStatusGet(
                    vrId,
                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                    ipPtr,
                    prefixLen,
                    clearActivity,
                    shadowPtr,
                    activityStatusPtr);
    }
    else
    {
        return prvCpssDxChIpLpmSip6RamUcPrefixActivityStatusGet(
                    vrId,
                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                    ipPtr,
                    prefixLen,
                    clearActivity,
                    shadowPtr,
                    activityStatusPtr);
    }
}

/**
* @internal prvCpssDxChIpLpmRamIpv4McEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv4 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipGroupPtr               - Pointer to the ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrcPtr                 - Pointer to the ip Source address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
* @param[in] clearActivity            - Indicates to clear activity status.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChIpLpmRamIpv4McEntryActivityStatusGet
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *ipGroupPtr,
    IN  GT_U32                              ipGroupPrefixLen,
    IN  GT_U8                               *ipSrcPtr,
    IN  GT_U32                              ipSrcPrefixLen,
    IN  GT_BOOL                             clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    if(shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
        return prvCpssDxChIpLpmRamMcEntryActivityStatusGet(
                    vrId,
                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                    ipGroupPtr,
                    ipGroupPrefixLen,
                    ipSrcPtr,
                    ipSrcPrefixLen,
                    clearActivity,
                    shadowPtr,
                    activityStatusPtr);
    }
    else
    {
        return prvCpssDxChIpLpmSip6RamMcEntryActivityStatusGet(
                    vrId,
                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                    ipGroupPtr,
                    ipGroupPrefixLen,
                    ipSrcPtr,
                    ipSrcPrefixLen,
                    clearActivity,
                    shadowPtr,
                    activityStatusPtr);

    }
}

/**
* @internal prvCpssDxChIpLpmRamIpv6McEntryActivityStatusGet function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipGroupPtr               - Pointer to the ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrcPtr                 - Pointer to the ip Source address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
* @param[in] clearActivity            - Indicates to clear activity status.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChIpLpmRamIpv6McEntryActivityStatusGet
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *ipGroupPtr,
    IN  GT_U32                              ipGroupPrefixLen,
    IN  GT_U8                               *ipSrcPtr,
    IN  GT_U32                              ipSrcPrefixLen,
    IN  GT_BOOL                             clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    if(shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    if (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP6_SHADOW_E)
    {
        return prvCpssDxChIpLpmRamMcEntryActivityStatusGet(
                    vrId,
                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                    ipGroupPtr,
                    ipGroupPrefixLen,
                    ipSrcPtr,
                    ipSrcPrefixLen,
                    clearActivity,
                    shadowPtr,
                    activityStatusPtr);
    }
    else
    {
        return prvCpssDxChIpLpmSip6RamMcEntryActivityStatusGet(
                    vrId,
                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                    ipGroupPtr,
                    ipGroupPrefixLen,
                    ipSrcPtr,
                    ipSrcPrefixLen,
                    clearActivity,
                    shadowPtr,
                    activityStatusPtr);
    }
}

/**
* @internal
*           prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncRanges
*           function
* @endinternal
*
* @brief   Shadow and HW synchronization of bucket's ranges
*
* @param[in] devNum                   - The device number
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
*                                      maskForRangeInTrieArray   - array of the ranges masks as found in the
*                                      trie. The array index represents the range
*                                      start address
*                                      validRangeInTrieArray     - array to indicate if a range was found in
*                                      the trie. The array index represents the
*                                      range start address.
* @param[in]nodeTotalChildTypesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS];
*                                   6 elements in a regular node
*                                   3 types of child that can be
*                                   for each range
*                                   (leaf,regular,compress)
* @param[in] level                    - the  in the tree (first  is 0)
* @param[in] numOfMaxAllowedLevels    - the maximal number of levels that is allowed
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncRanges
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U32                                    nodeTotalChildTypesArr[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS],
    IN  GT_U8                                     level,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC  *rangePtr;
    GT_U8                                   prefixTypeFirstRange, prefixTypeLastRange, prefixMcTypeFirstRange=0;
    GT_U8                                   prefixTypeSecondRange=0, prefixTypeSecondLastRange=0;
    GT_U32                                  tmpPrefixTypeRange;
    GT_STATUS                               status, retVal = GT_OK;
    GT_U32    hwBucketDataArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
    GT_U32    gonNodeSize=0;/* the node size can be 6 for regular or 1 for compress */
    GT_U32    gonPointerIndex=0;/* can be 0 for compress or 0-5 for regular, this is the pointer to the GON */
    GT_U32    tempGonPointerIndex=0;/* can be 0 for compress or 0-5 for regular, this is the pointer to the GON */
    GT_U32    gonNodeAddr=0; /* Node address to read the HW data from */
    GT_U32    hwNodeOffset = 0;/* base node offset address to read the HW data from */
    GT_U32    rangeType;/* CPSS_DXCH_LPM_CHILD_TYPE_ENT */
    GT_U32    totalChildTypesUntilNow=0;
    GT_U32    totalCompressedChilds=0;
    GT_U32    totalRegularChilds=0;

    cpssOsMemSet(&hwBucketDataArr[0], 0, sizeof(hwBucketDataArr));

    if (level == 0)
    {
        if (prefixType == CPSS_UNICAST_E)
        {
            prefixTypeFirstRange = 0;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS - 1) :
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS - 1);
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                prefixTypeSecondRange = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
                prefixTypeSecondLastRange = PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
            }
        }
        else /* CPSS_MULTICAST_E */
        {
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixMcTypeFirstRange = (GT_U8)tmpPrefixTypeRange;

            /* Need to consider ranges of unicast also while calculating node offset in the GON */
            prefixTypeFirstRange = (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) ? 220 : 0;

            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

            prefixTypeSecondLastRange = prefixTypeLastRange;
        }
    }
    else
    {
        prefixTypeFirstRange = 0;
        prefixTypeLastRange = 255;
        prefixTypeSecondRange = 0;
        prefixTypeSecondLastRange = 255;
    }

    /* Check next buckets */
    rangePtr = bucketPtr->rangeList;
    while (rangePtr)
    {
        /* Skip unicast ranges for multicast sync or multicast ranges for
           unicast sync */
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                ((rangePtr->startAddr > prefixTypeLastRange) && (rangePtr->startAddr<prefixTypeSecondRange))||
                 (rangePtr->startAddr > prefixTypeSecondLastRange))
            {
                rangePtr = rangePtr->next;
                continue;
            }
        }
        else
        {
            if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                (rangePtr->startAddr > prefixTypeLastRange))
            {
                rangePtr = rangePtr->next;
                continue;
            }
        }

        /* do not enter the if incase of pointer to NH or MC with type (G,*) */
        if ((rangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            (rangePtr->pointerType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) &&
            (!(((GT_U32)rangePtr->pointerType==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E) &&
               ((rangePtr->lowerLpmPtr.nextBucket->isImplicitGroupDefault==GT_FALSE)))))/* when isImplicitGroupDefault==GT_FALSE we are dealing with (G,*) */
        {
            if (bucketPtr->bucketType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                /* find the correct GON pointer out of 6 possible pointers */
                gonPointerIndex = rangePtr->startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
                /* in case we move from one gone index to the next one,
                   we need to reset the counter of the childs */
                if (gonPointerIndex!=tempGonPointerIndex)
                {
                    totalCompressedChilds = 0;
                    totalRegularChilds    = 0;
                    tempGonPointerIndex = gonPointerIndex;

                }
            }
            else
            {
                gonPointerIndex = 0;
            }

            if (rangePtr->pointerType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                rangeType = PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E;
                totalChildTypesUntilNow = totalRegularChilds;
                totalRegularChilds++;
            }
            else
            {
                rangeType = PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E;
                totalChildTypesUntilNow = totalCompressedChilds;
                totalCompressedChilds++;
            }

            if ((prefixType != CPSS_UNICAST_E) && (rangePtr->startAddr < prefixMcTypeFirstRange))
            {
                rangePtr = rangePtr->next;
                continue;
            }

            /* for MC case , need to calculate the address and size of a single GON
               according to the parameters we have in the Leaf */
            if((GT_U32)rangePtr->pointerType==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
            {
                /* root of SRC tree keep the hwNodeOffset in the hwBucketOffsetHandle
                   (same as root of MC GRP, and root of UC)*/
                /* at this point rangePtr->lowerLpmPtr.nextBucket->hwBucketOffsetHandle was already updated from the HW and can be used */
                if (rangePtr->lowerLpmPtr.nextBucket->hwBucketOffsetHandle==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected rangePtr->lowerLpmPtr.nextBucket->hwBucketOffsetHandle=0\n");
                }

                hwNodeOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(rangePtr->lowerLpmPtr.nextBucket->hwBucketOffsetHandle);
                gonNodeAddr = hwNodeOffset;
                if(rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                {
                    gonNodeSize = 6;
                }
                else
                {
                    if((rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)||
                       (rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)||
                       (rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E)||
                       (rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E))

                    {
                        gonNodeSize = 1;
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
               }
                /* we check that that the address we calculated according to the HW is the same as the nodeMemAddr we keep in the shadow */
                if (gonNodeAddr!=rangePtr->lowerLpmPtr.nextBucket->nodeMemAddr)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "gonNodeAddr different from nodeMemAddr ");
                }
            }
            else
            {
              /* calculate the address and the size of a single node from the GON
                (6 lines for regular all the rest 1 line) according to the
                 parameters we got from getNumOfRangesFromHWSip6 */

                /* when we get here the value in bucketPtr->hwGroupOffsetHandle[gonPointerIndex] was already sync with HW
                   so we can use it*/
                if (bucketPtr->hwGroupOffsetHandle[gonPointerIndex]==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected rbucketPtr->hwGroupOffsetHandle[gonPointerIndex]=0\n");
                }

                hwNodeOffset = PRV_CPSS_DXCH_LPM_RAM_GET_FALCON_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[gonPointerIndex]);
                status = getFromTheGonOneNodeAddrAndSize(hwNodeOffset,
                                                     nodeTotalChildTypesArr[gonPointerIndex],
                                                     rangeType,
                                                     totalChildTypesUntilNow,
                                                     &gonNodeAddr,
                                                     &gonNodeSize);
                if (status != GT_OK)
                {
                    return status;
                }

                /* set the nodeMemAddr in SW with the HW value */
                rangePtr->lowerLpmPtr.nextBucket->nodeMemAddr = gonNodeAddr;

            }


            /* read the HW data for the specific range */
            status = prvCpssDxChReadTableMultiEntry(devNum,
                                                 CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                 gonNodeAddr,
                                                 gonNodeSize,
                                                 &hwBucketDataArr[0]);

            if (status != GT_OK)
            {
                return status;
            }

            /* keep values in case reconstruct is needed */
            cpssOsMemCpy(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,shadowPtr->lpmRamOctetsToBlockMappingPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

            status = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucket(devNum,
                                                                 shadowPtr,
                                                                 vrId,
                                                                 rangePtr->lowerLpmPtr.nextBucket,
                                                                 gonNodeAddr,
                                                                 &hwBucketDataArr[0],
                                                                 (GT_U8)(level + 1),
                                                                 numOfMaxAllowedLevels,
                                                                 prefixType,
                                                                 protocol,
                                                                 GT_FALSE/*not the root bucket*/);
            if (status != GT_OK)
            {
                 /* free the allocated/bound RAM memory */
                retVal = prvCpssDxChLpmSip6RamMngAllocatedAndBoundMemFree(shadowPtr,protocol);
                if (retVal!=GT_OK)
                {
                    return retVal;
                }
                /* in case of fail we will need to recondtruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
                cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                             sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

               return status;
            }
            else
            {
                /* the data was written successfully to HW, we can reset the information regarding the new memoryPool allocations done.
                   next call to ADD will set this array with new values of allocated/bound blocks */
                cpssOsMemSet(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr,0,sizeof(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));

                cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));
            }
        }
        else
        {
            /* in case of NextHop pointers no need to do anything since
               the SW and HW NextHop synchronization was done in
               prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucket */
        }
        rangePtr = rangePtr->next;
    }

    return retVal;
}

/**
* @internal
*           prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucket
*           function
* @endinternal
*
* @brief   Shadow and HW synchronization of a bucket
*
* @param[in] devNum                   - The device number
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] hwBucketGonAddr          - node address of the read HW
* @param[in] hwBucketDataArr          - array holding hw data.
*                                       in case of root this is
*                                       a null pointer
* @param[in] level                    - the  level in the tree
*                                       (first is 0)
* @param[in] numOfMaxAllowedLevels    - the maximal allowed number of levels
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
* @param[in] isRootBucket             - GT_TRUE:the bucketPtr is the root bucket
*                                      GT_FALSE:the bucketPtr is not the root bucket
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucket
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U32                                    hwBucketGonAddr,
    IN  GT_U32                                    hwBucketDataArr[],
    IN  GT_U8                                     level,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_BOOL                                   isRootBucket
)
{
    GT_STATUS status = GT_OK;
    GT_U32    nodeSize;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tempRangePtr=NULL;/* Current range pointer.   */
    GT_U32    hwNumOfRanges;            /* the number of ranges in the HW */
    GT_U32    *hwRangesArr;             /* the ranges values retrieved from the HW */
    GT_U32    *hwRangesTypesArr;        /* for each range keep its type (regular,compress,leaf) */
    GT_U32    *hwRangesBitLineArr;      /* for each range keep its bit vector line */
    GT_U32    *hwRangesTypeIndexArr;    /* what is the number of this range type
                                           from total number of lines with the same type -
                                           where it is located in the GON */
    GT_U32    nodeChildAddressesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];/* the child_pointer located at the beginning of each line
                                                                                            for regular node we have 6 pointers, for compress node one pointer */
    GT_U32    nodeTotalChildTypesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS];/* 6 elements in a regular node
                                                                                                3 types of child that can be for each range (leaf,regular,compress) */

    GT_U32    nodeTotalLinesPerTypeArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS];   /* sum the number of lines in the GON according to the Type */
    GT_U32    nodeTotalBucketPerTypesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS]; /* sum the number of buckets (copmress, regular or leaf) in the GON according to the Type */

    GT_U32    bankIndexsOfTheGonsArray[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];  /* the indexs of the banks were the GON is located */

    GT_U32    hwPointerType;   /* The type of the next entry this entry points to. can be Leaf/Trigger */
    GT_U32    i=0;
    GT_U32    nonRealRangeIndex=0;
    GT_U32                              hwNodeOffset[6];/* HW node address */
    GT_U32                              hwNodeSize=0; /* size of the node in lines
                                                         regular node have 6 lines, all the rest hold 1 line*/
    PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT    hwNodeType = 0;/* the bucketType retrieved from HW */
    GT_U32                              hwRootNodeAddr=0;/* HW Root node address */
    PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT    hwRootNodeType =0;/* the root bucketType retrieved from HW */

    GT_U32                              swTotalNumOfRanges=0;/* number of ranges from sw including the ranges that are hidden inside a regular node */
    GT_U32                              *swTotalRangesArr;/* the ranges values retrieved from the SW */
    GT_U32                              *swTotalRangesTypesArr;/* the ranges types values retrieved from the SW */
    GT_BOOL                             *swTotalRangesIsNonRealArr;/* Is the range a hidden one or not */

    GT_U32 nextNodeType; /* bit 22
                            Defines the next node entry type
                            0x0 = Regular
                            0x1 = Compressed
                            valid if EntryType="Trigger" */

    GT_U32 nhPointer;       /* bits 21-7
                            Pointer to the Next Hop Table or the ECMP Table,
                            based on the Leaf Type.
                            valid if EntryType="Leaf" */
    GT_U32 nhAdditionalData;/* bits 6-3
                            Contains a set of UC security check enablers
                            and IPv6 MC scope level:
                            1. [6..6] UC SIP SA Check Mismatch Enable
                            2. [5..5] UC RPF Check Enable
                            3. [4..3] IPv6 MC Group Scope Level[1..0]
                            valid if EntryType="Leaf" */
    GT_U32 nextBucketPointer;/* bits 21-2
                                Pointer to the next bucket on the (G,S) lookup
                                valid if EntryType="Trigger" */
    GT_U32 leafType; /* bit 2
                        The leaf entry type
                        0x0 = Regular Leaf
                        0x1 = Multipath Leaf
                        valid if EntryType="Leaf" */
    GT_U32 entryType;/* bit 1
                        In the process of (*,G) lookup.
                        When an entry has this bit set, the (*, G) lookup
                        terminates with a match at the current entry, and (S,
                        G) SIP based lookup is triggered.
                        Note that in such case, head of trie start address for
                        the (S,G) lookup is obtained from the the (*, G)
                        lookup stage.
                         0x0 = Leaf
                         0x1 = Trigger; Trigger IP MC S+G Lookup */
    GT_U32 lpmOverEmPriority;/* bit 0
                                Define the resolution priority between LPM and
                                Exact Match results
                                 0x0 = Low; Exact Match has priority over LPM result
                                 0x1 = High;LPM result has priority over Exact Match */

    GT_U32  leafLineHwAddr=0;
    GT_U32  offsetOfLeafInLine=0;/*the offset in HW were the 20 bits for
                                  the pointer to the next bucket on the (G,S) lookup
                                  starts this value can be:
                                  for a line leaf structure: 2 for Leaf0, 25 for Leaf1,
                                  48 for Leaf2, 71 for Leaf3, 94 for Leaf4 (92-23*leafOffsetInLine)+2
                                  for a line embedded leaf structure:
                                  91, 68, 45  (89 - leafOffsetInLine*23)+2*/

    cpssOsMemSet(&hwNodeOffset[0], 0, sizeof(hwNodeOffset));
    if (isRootBucket==GT_TRUE)
    {
        /* read the HW data directly from the VRF table and update SW data in the shadow */
        status = prvCpssDxChLpmHwVrfEntryReadSip6(devNum, vrId, protocol,&hwRootNodeType, &hwRootNodeAddr);
        if (status != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(status, "Error on reading Vrf table for vrfId=%d, and protocol=%d\n");
        }
    }
    else
    {
        /* if this is not a call to the root bucket phase, but a call done from the ranges loop in
           prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncRanges,
           then we need to take the data address of the bucket from the shadow and to sync
           all missing data from HW data */
        switch (bucketPtr->bucketType)
        {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            hwNodeOffset[i] = 0;/* need to read this data later on in the code */
            hwNodeSize=6;
            hwNodeType = PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E;
            break;
        case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
            /* in cases of last GON level the pointer will be null
               also in all other cases since the SW was not yet Sync with HW address */
            hwNodeOffset[0] = 0;/* need to read this data later on in the code */
            hwNodeSize=1;
            hwNodeType = PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E;
            break;
        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
        default:
            /* error - we should not get here in case of a leaf */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error - we should not get here in case of a leaf. vrfId=%d, and protocol=%d\n", vrId, protocol);
        }
    }
    if (isRootBucket==GT_TRUE)
    {
        /* Allocate Root SW Head Of Trie to be the same as HW Head Of Trie */
        if(bucketPtr->hwBucketOffsetHandle==0)
        {
            /* only the root uses hwBucketOffsetHandle pointer and not hwGroupOffsetHandle */
            nodeSize = (hwNodeType == PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E) ?
                            PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS:
                            PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS;

            status = prvCpssDxChLpmRamMngHaSwPointerAllocateAccordingtoHwAddress(shadowPtr,
                                                                                 hwRootNodeAddr,
                                                                                 nodeSize,
                                                                                 protocol,
                                                                                 isRootBucket,
                                                                                 0,/* relevant only if isRootBucket==GT_FALSE*/
                                                                                 0,/* Root is always octet0 */
                                                                                 prefixType,
                                                                                 bucketPtr);

            if (status!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(status, "error in swPointerAllocateAccordingtoHwAddress hwRootNodeAddr=%d",hwRootNodeAddr);
            }
        }
    }
    else
    {
        /* in case of non-root bucket the checking of the address was done in previous recursive call */
    }

    cpssOsMemSet(nodeChildAddressesArr,0,sizeof(nodeChildAddressesArr));
    cpssOsMemSet(nodeTotalChildTypesArr,0,sizeof(nodeTotalChildTypesArr));
    cpssOsMemSet(nodeTotalLinesPerTypeArr,0,sizeof(nodeTotalLinesPerTypeArr));
    cpssOsMemSet(nodeTotalBucketPerTypesArr,0,sizeof(nodeTotalBucketPerTypesArr));
    cpssOsMemSet(bankIndexsOfTheGonsArray,0,sizeof(bankIndexsOfTheGonsArray));

    hwRangesArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (hwRangesArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(hwRangesArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    hwRangesTypesArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (hwRangesTypesArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(hwRangesTypesArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    hwRangesBitLineArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (hwRangesBitLineArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(hwRangesBitLineArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    hwRangesTypeIndexArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (hwRangesTypeIndexArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(hwRangesTypeIndexArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));

    swTotalRangesArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (swTotalRangesArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(swTotalRangesArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    swTotalRangesTypesArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (swTotalRangesTypesArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(swTotalRangesTypesArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    swTotalRangesIsNonRealArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (swTotalRangesIsNonRealArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(swTotalRangesIsNonRealArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));

    if (isRootBucket==GT_TRUE)
    {
        /* hw pointer in LPM entry is in LPM lines*/
        hwNodeSize=PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;

        /* read the bit vector according to the address we got from the VRF entry*/
        status = prvCpssDxChReadTableMultiEntry(devNum,CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                hwRootNodeAddr, hwNodeSize, &hwBucketDataArr[0]);
        if (status != GT_OK)
        {
            cpssOsFree(hwRangesArr);
            cpssOsFree(hwRangesTypesArr);
            cpssOsFree(hwRangesBitLineArr);
            cpssOsFree(hwRangesTypeIndexArr);

            cpssOsFree(swTotalRangesArr);
            cpssOsFree(swTotalRangesTypesArr);
            cpssOsFree(swTotalRangesIsNonRealArr);

            return status;
        }

       status = getNumOfRangesFromHWSip6(protocol,
                                          level,
                                          hwRootNodeType,       /* the Root node type */
                                          &hwBucketDataArr[0],  /* read from HW according to Root address */
                                          hwRootNodeAddr,       /* hwAddr is used in the fuction only for print incase of an error */
                                          nodeChildAddressesArr,
                                          nodeTotalChildTypesArr,
                                          nodeTotalLinesPerTypeArr,
                                          nodeTotalBucketPerTypesArr,
                                          &hwNumOfRanges,
                                          hwRangesArr,
                                          hwRangesTypesArr,
                                          hwRangesTypeIndexArr,
                                          hwRangesBitLineArr);
        if (status != GT_OK)
        {
            cpssOsFree(hwRangesArr);
            cpssOsFree(hwRangesTypesArr);
            cpssOsFree(hwRangesBitLineArr);
            cpssOsFree(hwRangesTypeIndexArr);

            cpssOsFree(swTotalRangesArr);
            cpssOsFree(swTotalRangesTypesArr);
            cpssOsFree(swTotalRangesIsNonRealArr);

            CPSS_LOG_ERROR_AND_RETURN_MAC(status, "fail in getNumOfRangesFromHWSip6\n");
        }

        /* set HW address of the GONs in SW shadow */
        for (i=0;i<PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;i++)
        {
            nodeSize = nodeTotalLinesPerTypeArr[i][0]+nodeTotalLinesPerTypeArr[i][1]+nodeTotalLinesPerTypeArr[i][2];
            if ((nodeSize!=0)&&(nodeChildAddressesArr[i]!=0)&&(bucketPtr->hwGroupOffsetHandle[i]==0))
            {
                status = prvCpssDxChLpmRamMngHaSwPointerAllocateAccordingtoHwAddress(shadowPtr,
                                                                                     nodeChildAddressesArr[i],
                                                                                     nodeSize,/* the size of the gon */
                                                                                     protocol,
                                                                                     GT_FALSE, /* isRootBucket = GT_FALSE, we are now going over the GON pointers */
                                                                                     i,        /* gonIndex */
                                                                                     level,
                                                                                     prefixType,
                                                                                     bucketPtr);
                if (status!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(status, "error in swPointerAllocateAccordingtoHwAddress hwRootNodeAddr=%d",hwRootNodeAddr);
                }
            }
        }
    }
    else
    {
        /*  we are dealing with a non-root bucket */

        /* hwBucketDataArr contains the HW data of the node */
        status = getNumOfRangesFromHWSip6(protocol,
                                          level,
                                          hwNodeType,           /* the node type */
                                          &hwBucketDataArr[0],  /* given as a parameter to the function from the call to
                                                                   prvCpssDxChLpmRamDbgRangesHwShadowSyncValidityCheckSip6*/
                                          hwNodeOffset[0],      /* hwAddr is used in the fuction only for print incase of an error */
                                          nodeChildAddressesArr,
                                          nodeTotalChildTypesArr,
                                          nodeTotalLinesPerTypeArr,
                                          nodeTotalBucketPerTypesArr,
                                          &hwNumOfRanges,
                                          hwRangesArr,
                                          hwRangesTypesArr,
                                          hwRangesTypeIndexArr,
                                          hwRangesBitLineArr);
        if (status != GT_OK)
        {
            cpssOsFree(hwRangesArr);
            cpssOsFree(hwRangesTypesArr);
            cpssOsFree(hwRangesBitLineArr);
            cpssOsFree(hwRangesTypeIndexArr);

            cpssOsFree(swTotalRangesArr);
            cpssOsFree(swTotalRangesTypesArr);
            cpssOsFree(swTotalRangesIsNonRealArr);

            CPSS_LOG_ERROR_AND_RETURN_MAC(status, "fail in getNumOfRangesFromHWSip6\n");
        }
        /* set HW address of the GONs in SW shadow */
        for (i=0;i<PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;i++)
        {
            nodeSize = nodeTotalLinesPerTypeArr[i][0]+nodeTotalLinesPerTypeArr[i][1]+nodeTotalLinesPerTypeArr[i][2];
            if ((nodeSize!=0)&&(nodeChildAddressesArr[i]!=0)&&(bucketPtr->hwGroupOffsetHandle[i]==0))
            {
                status = prvCpssDxChLpmRamMngHaSwPointerAllocateAccordingtoHwAddress(shadowPtr,
                                                                                     nodeChildAddressesArr[i],
                                                                                     nodeSize,/* the size of the gon */
                                                                                     protocol,
                                                                                     GT_FALSE, /* isRootBucket = GT_FALSE */
                                                                                     i,        /* gonIndex */
                                                                                     level,
                                                                                     prefixType,
                                                                                     bucketPtr);
                if (status!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(status, "error in swPointerAllocateAccordingtoHwAddress hwRootNodeAddr=%d",hwRootNodeAddr);
                }
            }
        }
    }

    /* 3. check that the SW ranges is the same as HW bitVector values
          get the SW number of ranges including the ones hidden incase of regular node */

    status = prvCpssDxChLpmRamDbgBucketShadowGetNumOfTotalRangesNonRealIncluddedSip6(bucketPtr,
                                                                                    &swTotalNumOfRanges,
                                                                                    swTotalRangesArr,
                                                                                    swTotalRangesTypesArr,
                                                                                    swTotalRangesIsNonRealArr);

    if (status!=GT_OK)
    {
        cpssOsFree(hwRangesArr);
        cpssOsFree(hwRangesTypesArr);
        cpssOsFree(hwRangesBitLineArr);
        cpssOsFree(hwRangesTypeIndexArr);

        cpssOsFree(swTotalRangesArr);
        cpssOsFree(swTotalRangesTypesArr);
        cpssOsFree(swTotalRangesIsNonRealArr);

        CPSS_LOG_ERROR_AND_RETURN_MAC(status, "error in call to prvCpssDxChLpmRamDbgBucketShadowGetNumOfTotalRangesHiddenIncluddedSip6\n");
    }

    /* 4. check that the SW ranges values is the same as HW ranges values */
    tempRangePtr = bucketPtr->rangeList;
    for (i=0; i<hwNumOfRanges; i++)
    {
        if (swTotalRangesIsNonRealArr[i]==GT_TRUE)
        {
            nonRealRangeIndex++;
        }

        /* compare the range pointer data */
        hwPointerType = hwRangesTypesArr[i];/* HW type can be empty=0/leaf=1/regular=2/compressed=3
                                               SW type can be regular=0/route=3/compressed=6/
                                               embedded1=7/embedded2=8/embedded3=9/multipath=9 */

        /* If the pointerType is to a MC source and the MC source bucket is a regular bucket then it means that
           this is a root of MC source tree that points directly to a nexthop or ECMP/QoS entry */
        if ((swTotalRangesIsNonRealArr[i]==GT_FALSE)&&
            ((GT_U32)swTotalRangesTypesArr[i]==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E) &&
            (tempRangePtr->lowerLpmPtr.nextBucket->isImplicitGroupDefault==GT_FALSE))/* when isImplicitGroupDefault==GT_FALSE we are dealing with (G,*) */
            {
                status = prvCpssDxChLpmRamDbgGetLeafDataSip6(devNum,
                                        bucketPtr,
                                        hwBucketGonAddr,
                                        hwBucketDataArr,
                                        hwRangesTypesArr[i],
                                        hwRangesTypeIndexArr[i],
                                        nodeChildAddressesArr[hwRangesBitLineArr[i]],
                                        nodeTotalChildTypesArr[hwRangesBitLineArr[i]],
                                        GT_TRUE,/*returnOnFailure*/
                                        &nextNodeType,
                                        &nhAdditionalData,
                                        &nextBucketPointer,
                                        &nhPointer,
                                        &leafType,
                                        &entryType,
                                        &lpmOverEmPriority,
                                        &leafLineHwAddr,
                                        &offsetOfLeafInLine);
                if (status!=GT_OK)
                {
                    cpssOsFree(hwRangesArr);
                    cpssOsFree(hwRangesTypesArr);
                    cpssOsFree(hwRangesBitLineArr);
                    cpssOsFree(hwRangesTypeIndexArr);

                    cpssOsFree(swTotalRangesArr);
                    cpssOsFree(swTotalRangesTypesArr);
                    cpssOsFree(swTotalRangesIsNonRealArr);

                    CPSS_LOG_ERROR_AND_RETURN_MAC(status, "Error: no synchronization between HW and SW - prvCpssDxChLpmRamDbgGetLeafDataSip6\n");
                }
            }
            else
            {
                switch (hwPointerType)
                {
                case PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E:

                     /* only incase we are dealing with a non-hidden range we should continue
                        checking this leaf */
                    if (swTotalRangesIsNonRealArr[i]==GT_FALSE)
                    {
                        status = prvCpssDxChLpmRamDbgGetLeafDataSip6(devNum,
                                                                    bucketPtr,
                                                                    hwBucketGonAddr,
                                                                    hwBucketDataArr,
                                                                    hwRangesTypesArr[i],
                                                                    hwRangesTypeIndexArr[i],
                                                                    nodeChildAddressesArr[hwRangesBitLineArr[i]],
                                                                    nodeTotalChildTypesArr[hwRangesBitLineArr[i]],
                                                                    GT_TRUE,/*returnOnFailure*/
                                                                    &nextNodeType,
                                                                    &nhAdditionalData,
                                                                    &nextBucketPointer,
                                                                    &nhPointer,
                                                                    &leafType,
                                                                    &entryType,
                                                                    &lpmOverEmPriority,
                                                                    &leafLineHwAddr,
                                                                    &offsetOfLeafInLine);
                        if (status!=GT_OK)
                        {
                            cpssOsFree(hwRangesArr);
                            cpssOsFree(hwRangesTypesArr);
                            cpssOsFree(hwRangesBitLineArr);
                            cpssOsFree(hwRangesTypeIndexArr);

                            cpssOsFree(swTotalRangesArr);
                            cpssOsFree(swTotalRangesTypesArr);
                            cpssOsFree(swTotalRangesIsNonRealArr);

                            CPSS_LOG_ERROR_AND_RETURN_MAC(status, "Error: no synchronization between HW and SW - prvCpssDxChLpmRamDbgGetLeafDataSip6\n");
                        }

                        if((GT_U32)swTotalRangesTypesArr[i]==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
                        {
                            /* we are dealing with a range that points to a SRC tree that is not a NH (this was dealt with above) */

                            /* if entryType is trigger we need to set the hw value we got :
                               nextBucketPointer and HW offset of the Leaf */
                            tempRangePtr->lowerLpmPtr.nextBucket->nodeMemAddr=nextBucketPointer;
                            tempRangePtr->lowerLpmPtr.nextBucket->fifthAddress=(GT_U8)offsetOfLeafInLine;
                            tempRangePtr->lowerLpmPtr.nextBucket->pointingRangeMemAddr=leafLineHwAddr;

                            /* HW offset handle for MC is a special case that we use hwBucketOffsetHandle and not hwGroupOffsetHandle */
                            /* set HW address of the GONs in SW shadow */
                            if(nextNodeType==0)
                                nodeSize=PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;
                            else
                                nodeSize=PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS;

                            status = prvCpssDxChLpmRamMngHaSwPointerAllocateAccordingtoHwAddress(shadowPtr,
                                                                                                 nextBucketPointer,
                                                                                                 nodeSize,/* the size of the gon */
                                                                                                 protocol,
                                                                                                 GT_TRUE,  /* isRootBucket = GT_TRUE - Root of SRC */
                                                                                                 0,        /* gonIndex */
                                                                                                 0,        /* srcRoot always in octetIndex=0*/
                                                                                                 prefixType,
                                                                                                 tempRangePtr->lowerLpmPtr.nextBucket);
                            if (status!=GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(status, "error in swPointerAllocateAccordingtoHwAddress hwRootNodeAddr=%d",hwRootNodeAddr);
                            }
                        }
                    }
                    break;
                case PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E:
                case PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E:
                    /* nothing to check at this moment the node will be checked in the function
                       prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncRanges */
                    break;
                default:
                    cpssOsFree(hwRangesArr);
                    cpssOsFree(hwRangesTypesArr);
                    cpssOsFree(hwRangesBitLineArr);
                    cpssOsFree(hwRangesTypeIndexArr);

                    cpssOsFree(swTotalRangesArr);
                    cpssOsFree(swTotalRangesTypesArr);
                    cpssOsFree(swTotalRangesIsNonRealArr);

                    CPSS_LOG_ERROR_AND_RETURN_MAC(status, "Error: no synchronization between HW and SW - - error in HW Pointer Type\n");
                    break;
                }
            }

            /* update the HW is not needed --> change the flag */
            tempRangePtr->updateRangeInHw = GT_FALSE;

            /* if this is a hidden range it will not apear in the tempRangePtr,
               so need to continue with the same tempRangePtr */
            if ((swTotalRangesIsNonRealArr[i]==GT_FALSE)&&(tempRangePtr->next != NULL))
            {
                tempRangePtr = tempRangePtr->next;
            }
    }
    if(i != hwNumOfRanges)
    {
        cpssOsFree(hwRangesArr);
        cpssOsFree(hwRangesTypesArr);
        cpssOsFree(hwRangesBitLineArr);
        cpssOsFree(hwRangesTypeIndexArr);

        cpssOsFree(swTotalRangesArr);
        cpssOsFree(swTotalRangesTypesArr);
        cpssOsFree(swTotalRangesIsNonRealArr);

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - illegal ranges values\n");
    }
    /* 6. if the SW range point to a next bucket then recursively check the new bucket, stages 1-5  */
    if (bucketPtr->numOfRanges >= 2)
    {
        /* Ranges validity check */
        status = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncRanges(devNum,
                                                             shadowPtr,
                                                             vrId,
                                                             bucketPtr,
                                                             nodeTotalChildTypesArr,
                                                             level,
                                                             numOfMaxAllowedLevels,
                                                             prefixType,
                                                             protocol);
        if (status != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncRanges");
        }
    }
    else
    {
        /* error - a bucket can not have less then 2 ranges */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: a bucket can not have less then 2 ranges,"
                                               "fail in prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucket\n");
    }

    cpssOsFree(hwRangesArr);
    cpssOsFree(hwRangesTypesArr);
    cpssOsFree(hwRangesBitLineArr);
    cpssOsFree(hwRangesTypeIndexArr);

    cpssOsFree(swTotalRangesArr);
    cpssOsFree(swTotalRangesTypesArr);
    cpssOsFree(swTotalRangesIsNonRealArr);

    return status;
}


/**
* @internal getSwBucketInfo function
* @endinternal
*
* @brief   This function gets SW bucket info
*
* @param[in] bucketPtr                 - Pointer to the bucket.
* @param[in] newBucketType             - the bucket's new type
* @param[out] memSizePtr               - the memory size this bucket needs (in LPM lines)
* @param[out] writeOffsetPtr           - the write offset in the bucket where the ranges start
* @param[out] fifthStartAddrPtr        - the calculated fifth range address.
*
* @retval GT_OK    on success
*         GT_FAIL  otherwise
*/
static GT_STATUS getSwBucketInfo
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       newBucketType,
    OUT GT_U32                                    *memSizePtr,
    OUT GT_U32                                    *writeOffsetPtr,
    OUT GT_U8                                     *fifthStartAddrPtr
)
{
    switch (newBucketType)
    {
    case (CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E):
        *memSizePtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS + bucketPtr->numOfRanges;
        /* One word for the bit-vector + place for next pointers.   */
        *writeOffsetPtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS;
        break;

    case (CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E):
        *memSizePtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS + bucketPtr->numOfRanges;
        /* 2 words for the bit-vector + place for next pointers.   */
        *writeOffsetPtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS;

        /* this is the only place where the fifth address is valid - so retrive it:
                             range idx <0>      <1>   <2>   <3>   <4>   <5> */
        *fifthStartAddrPtr = bucketPtr->rangeList->next->next->next->next->next->startAddr;
        break;

    case (CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E):
        *memSizePtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS + bucketPtr->numOfRanges;
        /* 11 words for the bit-vector + place for next pointers.   */
        *writeOffsetPtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Fail in getBucketInfo: not a legal bucketType\n");
    }

    return (GT_OK);
}


/**
* @internal
*           getRangesArrayFromHwData function
* @endinternal
*
* @brief   return the array of ranges according to the HW data
*
* @param[in] hwNodeType             - the bucket type : regular/compress_1/compress_2
* @param[in] hwNumOfRanges          - number of ranges to return
* @param[in] hwAddr                 - hw address of the data
* @param[in] hwBucketData           - the hw data bit vector holding the ranges
* @param[in] hwBucketFifthStartAddr - in case of compress_2 this is the fifth address
*
* @param[out]hwRangesPtr            - (pointer to) an array holding the ranges
*
* @retval GT_FAIL                     - on failure
* @retval GT_OK                       - on success
*/
static GT_STATUS getRangesArrayFromHwData
(
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT hwNodeType,
    IN  GT_U32                              hwNumOfRanges,
    IN  GT_U32                              hwAddr,
    IN  GT_U32                              hwBucketData[PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS],
    IN  GT_U8                               hwBucketFifthStartAddr,
    OUT PRV_CPSS_DXCH_LPM_RANGES_STC        *hwRangesPtr
)
{
    GT_STATUS retVal=GT_OK;
    GT_U32    i=0;

    if (hwNodeType == CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E)
    {
        hwRangesPtr->ranges[0] = 0;
        hwRangesPtr->ranges[1] = hwBucketData[0] & 0xFF;
        hwRangesPtr->ranges[2] = (hwBucketData[0] >> 8) & 0xFF;
        hwRangesPtr->ranges[3] = (hwBucketData[0] >> 16) & 0xFF;
        hwRangesPtr->ranges[4] = (hwBucketData[0] >> 24) & 0xFF;
    }
    else
    {
        if(hwNodeType == CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)
        {
            /* Bitmap has 9 bytes; Word 1: [range_4_start ; range_3_start ; range_2_start ; range_1_start]
                                   Word 2: [range_9_start ; range_8_start ; range_7_start ; range_6_start]
                                    - range_5_start is carried from the pointer to this bucket
                                    - range_0_start is always 0 (so no need to add to bitmap) */
            hwRangesPtr->ranges[0] = 0;
            hwRangesPtr->ranges[1] =  hwBucketData[0] & 0xFF;
            hwRangesPtr->ranges[2] = (hwBucketData[0] >> 8) & 0xFF;
            hwRangesPtr->ranges[3] = (hwBucketData[0] >> 16) & 0xFF;
            hwRangesPtr->ranges[4] = (hwBucketData[0] >> 24) & 0xFF;
            hwRangesPtr->ranges[5] = hwBucketFifthStartAddr;


            /* for word 1: all ranges start must be != 0 (if one is 0, then it should be compressed 1);
               Range_5_start must be != 0 (if 0, then should use compressed 1) */
            if ((hwRangesPtr->ranges[1] == 0) || (hwRangesPtr->ranges[2] == 0) || (hwRangesPtr->ranges[3] == 0)
                || (hwRangesPtr->ranges[4] == 0) || (hwRangesPtr->ranges[5] == 0))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - At least 6 ranges should be defined!!!\n");
            }

            hwRangesPtr->ranges[6] =  hwBucketData[1*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS] & 0xFF;
            hwRangesPtr->ranges[7] = (hwBucketData[1*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS] >> 8) & 0xFF;
            hwRangesPtr->ranges[8] = (hwBucketData[1*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS] >> 16) & 0xFF;
            hwRangesPtr->ranges[9] = (hwBucketData[1*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS] >> 24) & 0xFF;

        }
        else/* CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E */
        {
            retVal = validateBitVectorOfRegularBucket(hwBucketData, hwAddr, 8);
            if (retVal != GT_OK)
            {
                return retVal;
            }
            for (i = 0; i < hwNumOfRanges; i++)
            {
                hwRangesPtr->ranges[i] = (GT_U8)getBaseAddrFromHW(CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E, i,hwBucketData, 0);
            }
        }
    }
    return retVal;
}

/**
* @internal readBitVectorData function
* @endinternal
*
* @brief   Read the bit vector data from HW.
*
* @param[in] devNum             - the device number
* @param[in] hwNodeAddr         - address of data to be read
* @param[in] hwNodeType         - com_1/com_2/regular
* @param[in] hwBitMapSize       - the number of consecutive entries to read
*
* @param[out] bucketData           - (pointer to) the data that will be read from the table
* @param[out] duplicateBucketData  - (pointer to) the duplicated data that will be read from the table
* @param[out] hwNumOfRangesPtr     - (pointer to) number of ranges in the bit vector
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS readBitVectorData
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  hwNodeAddr,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     hwNodeType,
    IN  GT_U32                                  hwBitMapSize,
    OUT GT_U32                                  *bucketData,
    OUT GT_U32                                  *duplicateBucketData,
    OUT GT_U32                                  *hwNumOfRangesPtr
)
{
    GT_STATUS retVal = GT_OK;

    retVal = prvCpssDxChReadTableMultiEntry(devNum,CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                            hwNodeAddr, hwBitMapSize, bucketData);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        /* read the bit vector */
        retVal = prvCpssDxChReadTableMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E,
                                         hwNodeAddr, hwBitMapSize, duplicateBucketData);
        if (retVal != GT_OK)
        {
            return retVal;
        }
    }

    *hwNumOfRangesPtr = getNumOfRangesFromHW(hwNodeType, bucketData, hwNodeAddr);

    return retVal;
}

/**
* @internal readBitVectorData function
* @endinternal
*
* @brief   Read the bit vector data from HW.
*
* @param[in] devNum             - the device number
* @param[in] hwNodeAddr         - address of data to be read
* @param[in] hwNodeType         - com_1/com_2/regular
* @param[in] hwBitMapSize       - the offset off the data
* @param[in] hwNumOfRanges      - number of consecutive entries to read,
*                                 number of ranges in the bucket
* @param[in] hwBucketFifthStartAddr - in case of compress_2 this is the fifth address
*
* @param[out] bucketData           - (pointer to) the data that will be read from the table
* @param[out] duplicateBucketData  - (pointer to) the duplicated data that will be read from the table
* @param[out] hwRangesPtr          - (pointer to) an array holding the ranges
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS readBucketData
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          hwNodeAddr,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             hwNodeType,
    IN  GT_U32                                          hwBitMapSize,
    IN  GT_U32                                          hwNumOfRanges,
    IN  GT_U8                                           hwBucketFifthStartAddr,
    OUT GT_U32                                          *bucketData,
    OUT GT_U32                                          *duplicateBucketData,
    OUT PRV_CPSS_DXCH_LPM_RANGES_STC                    *hwRangesPtr
)
{
    GT_STATUS retVal=GT_OK;

    retVal = prvCpssDxChReadTableMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                         hwNodeAddr + (hwBitMapSize), hwNumOfRanges,
                                         &bucketData[(hwBitMapSize)*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS]);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        /* read the ranges (the LPM lines after the bit vector) */
        retVal = prvCpssDxChReadTableMultiEntry(devNum,
                                         CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E,
                                         hwNodeAddr + (hwBitMapSize), hwNumOfRanges,
                                         &duplicateBucketData[(hwBitMapSize)*PRV_CPSS_DXCH_LPM_RAM_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS]);
        if (retVal != GT_OK)
        {
            return retVal;
        }

        retVal = compareBucket(devNum,hwNodeAddr,bucketData,duplicateBucketData,hwNumOfRanges,hwNodeType,GT_TRUE);

        if (retVal != GT_OK)
        {
            return retVal;
        }
    }

    retVal = getRangesArrayFromHwData(hwNodeType,
                                      hwNumOfRanges,
                                      hwNodeAddr,
                                      bucketData,
                                      hwBucketFifthStartAddr,/* for Root case this input is ignored*/
                                      hwRangesPtr);
    if (retVal!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "error in getRangesArrayFromHwData hwNodeAddr=%d",hwNodeAddr);
    }
    return retVal;
}


/**
* @internal checkMcRootSource function
* @endinternal
*
* @brief   check according to the HW data line if the value of
*          the NH pointer is equal to the value of the route
*          entry base memory address keept in the SW
*
* @param[in] bucketDataLine - (pointer to) the data line
* @param[in] nextHopEntry   - (pointer to) the SW NH data
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
static GT_STATUS checkMcRootSource
(
    IN GT_U32                                       bucketDataLine,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *nextHopEntry
)
{
    GT_U32      hwNhPointer;     /* This is the next hop pointer for accessing next hop table or
                                    the pointer for the ECMP table for fetching the ECMP attributes from ECMP table */

    /* If the pointerType is to a MC source and the MC source bucket is a regular bucket then it means that
       this is a root of MC source tree that points directly to a nexthop or ECMP/QoS entry */

     /* make sure the HW bit for Trigger IP MC S+G Lookup is on */
    if(((bucketDataLine >> 2) & 0x1)!=1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - Trigger IP MC S+G Lookup\n");
    }
    switch (nextHopEntry->routeEntryMethod)
    {
        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E:
        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E:
            /* check that if a SW ranges is of type ECMP/QOS then HW range should point to the same NH */
            hwNhPointer = ((bucketDataLine >> 7) & 0x00003FFF);
            if (nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - Pointer value\n");
            }
            break;
        case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E:
            /* check that if a SW ranges is of type NH then HW range should point to the same NH */
            hwNhPointer = ((bucketDataLine >> 7) & 0x00007FFF);
            if (nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - Pointer value\n");
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - error in routeEntryMethod\n");
            break;
    }
    return GT_OK;
}

/*
* @internal getHwNextBucketParameters function
* @endinternal
*
* @brief   check according to the HW next bucket type the sync
*          with the SW and return
*
*
* @param[in] bucketDataLine             - (pointer to) the data line
* @param[in] hwNextBucketType           - The next bucket type.
*                                         can be Regular/One_Compressed/Two_Compressed
* @param[in] swBucketFifthStartAddr     - in case of comp_2 this value should be verify with HW value
*
* @param[out] hwAddrPtr                 - (pointer to)next hw pointer in LPM entry(in LPM lines)
* @param[out] hwBitMapSizePtr           - (pointer to)
* @param[out] hwNodeTypePtr             - (pointer to)
* @param[out] hwBucketFifthStartAddrPtr - (pointer to)
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
static GT_STATUS getHwNextBucketParameters
(
    IN  GT_U32                                  bucketDataLine,
    IN  GT_U32                                  hwNextBucketType,
    IN  GT_U8                                   swBucketFifthStartAddr,
    OUT GT_U32                                  *hwAddrPtr,
    OUT GT_U32                                  *hwBitMapSizePtr,
    OUT CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     *hwNodeTypePtr,
    OUT GT_U8                                   *hwBucketFifthStartAddrPtr
)
{
    switch(hwNextBucketType)
    {
        case 0: /* CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E */
        case 1: /* CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E */
             /* hw pointer in LPM entry is in LPM lines*/
            *hwAddrPtr = ((bucketDataLine & 0x1FFFFFE0) >> 5);

            /* update the bit vector size of the next bucket */
            if(hwNextBucketType==0){
                *hwBitMapSizePtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS;
                *hwNodeTypePtr = CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E;
            }
            else
            {
                *hwBitMapSizePtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS;
                *hwNodeTypePtr = CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E;
            }
            break;
        case 2: /* CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E */
            *hwAddrPtr = (bucketDataLine >> 13);

            *hwBucketFifthStartAddrPtr = (bucketDataLine >> 5)& 0xFF;
            /* verify HW and SW sync */
            if (swBucketFifthStartAddr!= *hwBucketFifthStartAddrPtr)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - error in Fifth adress value \n");
            }
            /* update the bit vector size of the next bucket */
            *hwBitMapSizePtr = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS;
            *hwNodeTypePtr = CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - error in Next Bucket Pointer Type\n");
    }

    return GT_OK;
}

/**
* @internal
*           prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5_Sync function
* @endinternal
*
* @brief   Shadow and HW synchronization
*
* @param[in] devNum                   - The device number
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
*
* @retval GT_FAIL                     - on failure
* @retval GT_OK                       - on success
*/
GT_STATUS prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5_Sync
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol
)
{
    GT_STATUS                                       retVal= GT_OK;
    GT_U32                                          i = 0;
    GT_U32                                          level = 0;
    GT_BOOL                                         goDown;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         **currBucketPtr = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry);
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             **currBucketPtrType = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray);
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          **pRange;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             hwRootNodeType;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             hwNodeType;
    GT_U32                                          hwRootNodeAddr;
    GT_U32                                          *currHwBucketsAddressPtr = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketsAddressArray);
    GT_U32                                          *currHwBucketsNumberOfRangesPtr = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketsNumberOfRangesArray);
    GT_U32                                          *currHwBucketsNumberOfRangesHandledPtr = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketsNmberOfRangesHandledArray);
    PRV_CPSS_DXCH_LPM_RANGES_STC                    *currHwRangePtr;
    PRV_CPSS_DXCH_LPM_BUCKET_DATA_STC               *currHwBucketPtr;

    GT_U32                                          swBucketMemSize;
    GT_U32                                          swBucketWriteOffset;
    GT_U8                                           swBucketFifthStartAddr;
    GT_U8                                           hwBucketFifthStartAddr;

    GT_U32                                          hwNumOfRanges;
    GT_U32                                          hwBitMapSize;

    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *tempRangePtr;/* Current range pointer.   */
    GT_U32                                          hwPointerType;   /* The type of the next entry this entry points to. can be Leaf/Trigger */
    GT_U32                                          hwNextBucketType;/* The next bucket (point to by NextBucketPointer) type.
                                                                        can be Regular/One_Compressed/Two_Compressed */
    GT_U32                                          hwNhPointer;     /* This is the next hop pointer for accessing next hop table or
                                                                        the pointer for the ECMP table for fetching the ECMP
                                                                        attributes from ECMP table */
    GT_U32                                          hwAddr = 0;      /* The pointer to the next bucket entry in lines */
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             swRootBucketType; /* the 1st level bucket type  */

    GT_U32                                          tmpPrefixTypeRange;
    GT_U8                                           prefixTypeFirstRange, prefixTypeLastRange;
    GT_U8                                           prefixTypeSecondRange=0, prefixTypeSecondLastRange=0;


    swRootBucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol];

   /* 1.read the VR entry from HW in order to get the head of trie address
        (This address represent the start address of the first byte).
        The VR entry will also give us the Bucket-Type of the first bucket,
        it can be regular or compress_1.*/

    /* read the HW data directly from the VRF table
       input parameter bucketPtr hold the pointer to the root bucket
       this is our start point of the trees that need to be sync     */
    retVal = prvCpssDxChLpmHwVrfEntryRead(devNum, vrId, protocol,&hwRootNodeType, &hwRootNodeAddr);
    if (retVal != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "Error on reading Vrf table for vrfId=%d, and protocol=%d\n",vrId,protocol);
    }

    /* compare SW to HW bucket type */
    if (bucketPtr->bucketType!=hwRootNodeType)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "HW and SW are not Sync after HA "
                                                "bucketPtr->bucketType[%d]!=hwRootNodeType[%d]\n",
                                       bucketPtr->bucketType,hwRootNodeType);
    }

    /* 2.Keep in the hwBucketsAddressArray the address of the head of trie.
       This array will be dynamically updated as we go over the trie */
    PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketsAddressArray)[level]=hwRootNodeAddr; /* level=0 for the root */

    /* 3.If the bucket type is regular then we will have a 11 line of bit
       vector before the lines of the ranges. If the bucket type is
       compress_1 then only one line of bit vector (swBucketWriteOffset) */
    retVal = getSwBucketInfo(bucketPtr,bucketPtr->bucketType, &swBucketMemSize, &swBucketWriteOffset, &swBucketFifthStartAddr);
    if (retVal!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "Error in getBucketInfo \n");
    }

    /* 4.Read from the HW 1 line or 11 lines according to the bucket type and find
       out how many ranges we have in the bucket and what are those ranges.

       since we already checked that bucketPtr->bucketType==hwRootNodeType
       we can use the swBucketWriteOffset to read from the HW

       read the bit vector according to the address we got from the VRF entry*/
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketDataArr).bucketData, 0, PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS * sizeof(GT_U32));
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(duplicateBucketData),0,sizeof(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(duplicateBucketData)));

    /* read the bit vector */
    hwBitMapSize = swBucketWriteOffset;
    retVal = readBitVectorData(devNum,hwRootNodeAddr, hwRootNodeType,
                               hwBitMapSize, &(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketDataArr).bucketData[0]),
                               PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(duplicateBucketData),
                               &hwNumOfRanges);
    if (retVal != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "HW and SW are not Sync error in reading bit vector from HW, "
                                              "hwRootNodeAddr[%d]\n",hwRootNodeAddr);
    }

    /* check HW and SW sync */
    if (bucketPtr->numOfRanges!=hwNumOfRanges)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "HW and SW are not Sync after HA "
                                               "bucketPtr->numOfRanges[%d]!=hwNumOfRanges[%d]\n",
                                      bucketPtr->numOfRanges,hwNumOfRanges);
    }

    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketsNmberOfRangesHandledArray), 0, sizeof(GT_U32)*MAX_LPM_LEVELS_CNS*2);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketsNumberOfRangesArray), 0, sizeof(GT_U32)*MAX_LPM_LEVELS_CNS*2);

    PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketsNumberOfRangesArray)[level]=hwNumOfRanges;

    /* 5.Read from the HW the number of lines needed according to the number
       of ranges (each range occupy one line in HW) */

    /* read the ranges (the LPM lines after the bit vector) */
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges).ranges, 0, sizeof(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges).ranges));
    retVal = readBucketData(devNum,hwRootNodeAddr,hwRootNodeType,hwBitMapSize,
                            hwNumOfRanges,0,PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketDataArr).bucketData,
                            PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(duplicateBucketData),
                            &(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges)));
    if (retVal != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "HW and SW are not Sync error in reading ranges from HW, "
                                              "hwRootNodeAddr[%d]\n",hwRootNodeAddr);
    }

    /* check that the SW ranges values is the same as HW ranges values */
    tempRangePtr = bucketPtr->rangeList;
    for (i=0; (i<hwNumOfRanges && tempRangePtr!=NULL); i++)
    {
        if(tempRangePtr->startAddr != PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges).ranges[i])
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - ranges values\n");
        }
        else
        {
            /* range value is synchronized between HW and SW so now check that the SW range type
               is the same as HW range type, if it is the same the compare the range pointer data */
            hwPointerType = (PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketDataArr).bucketData[i+hwBitMapSize]) & 0x3;
            hwNextBucketType = (PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketDataArr).bucketData[i+hwBitMapSize] >> 3) & 0x3;

            if(((tempRangePtr->pointerType <= CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)&&(hwPointerType!=0))||
               ((tempRangePtr->pointerType==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)&&(hwPointerType!=1))||
               ((tempRangePtr->pointerType==CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E)&&(hwPointerType!=2))||
               ((tempRangePtr->pointerType==CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E)&&(hwPointerType!=3)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - range Pointer Type value\n");
            }
            else
            {
                /* If the pointerType is to a MC source and the MC source bucket is a regular bucket then it means that
                   this is a root of MC source tree that points directly to a nexthop or ECMP/QoS entry */
                if (((GT_U32)tempRangePtr->pointerType==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E) &&
                    (tempRangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E))
                {
                    retVal = checkMcRootSource(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketDataArr).bucketData[i+hwBitMapSize],
                                      tempRangePtr->lowerLpmPtr.nextBucket->rangeList->lowerLpmPtr.nextHopEntry);
                    if (retVal!=GT_OK)
                    {
                        return retVal;
                    }
                }
                else
                {
                    if ((GT_U32)tempRangePtr->pointerType==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
                    {
                        /* make sure the HW bit for Trigger IP MC S+G Lookup is on */
                        if(((PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketDataArr).bucketData[i+hwBitMapSize] >> 2) & 0x1)!=1)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - Trigger IP MC S+G Lookup\n");
                        }
                    }

                    switch (hwPointerType)
                    {
                        case 0:/* pointing to bucket */
                            /* check the bucket type: regular/comp_1/comp_2 */
                            retVal = getHwNextBucketParameters(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketDataArr).bucketData[i+hwBitMapSize],
                                                               hwNextBucketType,
                                                               tempRangePtr->lowerLpmPtr.nextBucket->fifthAddress,
                                                               &hwAddr,&hwBitMapSize,&hwNodeType,&hwBucketFifthStartAddr);
                            if (retVal!=GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "Error: getHwNextBucketParameters \n");
                            }
                            break;
                        case 0x1:/* CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E */
                            /* 5. check that if a SW ranges is of type NH then HW range should point to the same NH */
                            hwNhPointer = ((PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketDataArr).bucketData[i+hwBitMapSize] >> 7) & 0x00007FFF);
                            if (tempRangePtr->lowerLpmPtr.nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - Pointer value\n");
                            }
                            break;
                        case 0x2:/* CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E */
                        case 0x3:/* CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E */
                            /* 5. check that if a SW ranges is of type ECMP/QOS then HW range should point to the same NH */
                            hwNhPointer = ((PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketDataArr).bucketData[i+hwBitMapSize] >> 7) & 0x00003FFF);
                            if (tempRangePtr->lowerLpmPtr.nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - Pointer value\n");
                            }
                            break;
                        default:
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - error in HW Bucket Pointer Type\n");
                            break;
                    }
                }

                if (tempRangePtr->next != NULL)
                    tempRangePtr = tempRangePtr->next;
                else
                    break;
            }
        }
    }
    if(i != (hwNumOfRanges-1))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - ranges values\n");
    }

    /* zero the arrays */
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry),0,sizeof(PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC*)*MAX_LPM_LEVELS_CNS*2);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArray),0xFF, /*0xFFFFFFFF indicates untouched level*/
                 sizeof(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArray)));
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiBucketsDataArr),0,sizeof(PRV_CPSS_DXCH_LPM_BUCKET_DATA_STC*)*MAX_LPM_LEVELS_CNS*2);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiRangesArr),0,sizeof(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiRangesArr)));
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(duplicateBucketData),0,sizeof(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(duplicateBucketData)));

    /* set the initial values.*/
    PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry)[0] = bucketPtr;
    PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArray)[0] = &swRootBucketType;
    pRange = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArray);
    currHwRangePtr = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiRangesArr);
    currHwBucketPtr = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiBucketsDataArr);
    cpssOsMemCpy(&(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiRangesArr)[0]),
                 &(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges)),
                 sizeof(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges)));/* root bucket ranges */
    cpssOsMemCpy(&(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiBucketsDataArr)[0]),
                 &(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketDataArr)),sizeof(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwBucketDataArr)));/* root bucket data */

    /* go over all the lpm tree and update the shadow according to HW */
    while (currBucketPtr >= PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArry))
    {
         if (level == 0)
         {
             if (prefixType == CPSS_UNICAST_E)
             {
                 prefixTypeFirstRange = 0;
                 tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                     (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS - 1) :
                     (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS - 1);
                 prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

                 if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
                 {
                     prefixTypeSecondRange = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
                     prefixTypeSecondLastRange = PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
                 }
             }
             else /* CPSS_MULTICAST_E */
             {
                 tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                     PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                     PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
                 prefixTypeFirstRange = (GT_U8)tmpPrefixTypeRange;
                 tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                     PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                     PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS;
                 prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

                 prefixTypeSecondLastRange = prefixTypeLastRange;
             }
         }
         else
         {
             prefixTypeFirstRange = 0;
             prefixTypeLastRange = 255;
             prefixTypeSecondRange = 0;
             prefixTypeSecondLastRange = 255;

         }

        /* we start with the assumption we'll go up the bucket trie */
        goDown = GT_FALSE;

        /* if the range pointer reached NULL means that we finished with this level */
        if (*pRange != NULL)
        {
            /* check if we need to continue this level or this a brand new level (0xFFFFFFFF) */
            if (*pRange == (PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *)(~((GT_UINTPTR)0)))
            {
                *pRange=(*currBucketPtr)->rangeList;
            }
            do
            {
                /* we already checked that the HW and SW bucketType is sync so we can use the swBucketWriteOffset as the HW value */
                retVal = getSwBucketInfo((*currBucketPtr),(*currBucketPtr)->bucketType,
                                         &swBucketMemSize, &swBucketWriteOffset,
                                         &swBucketFifthStartAddr);
                if (retVal!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "Error in getBucketInfo \n");
                }

                /* read the bit vector */
                hwBitMapSize = swBucketWriteOffset;

                /* Skip unicast ranges for multicast sync or multicast ranges for unicast sync */
                if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
                {
                    if (((*pRange)->startAddr < prefixTypeFirstRange) ||
                        (((*pRange)->startAddr > prefixTypeLastRange) && ((*pRange)->startAddr<prefixTypeSecondRange))||
                         ((*pRange)->startAddr > prefixTypeSecondLastRange))
                    {
                        (*currHwBucketsNumberOfRangesHandledPtr)++; /* count the range as handled */
                        *pRange = (*pRange)->next;
                        continue;
                    }
                }
                else
                {
                    if (((*pRange)->startAddr < prefixTypeFirstRange) ||
                        ((*pRange)->startAddr > prefixTypeLastRange))
                    {
                        (*currHwBucketsNumberOfRangesHandledPtr)++; /* count the range as handled */
                        *pRange = (*pRange)->next;
                        continue;
                    }
                }

                /* set i to be the index of the current treated range in the array */
                i= *currHwBucketsNumberOfRangesHandledPtr;

                if((*pRange)->startAddr != currHwRangePtr->ranges[i])
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - ranges values\n");
                }
                else
                {
                    /* range value is synchronized between HW and SW so now check that the SW range type
                       is the same as HW range type, if it is the same then compare the range pointer data */
                    hwPointerType = (currHwBucketPtr->bucketData[i+hwBitMapSize]) & 0x3;
                    hwNextBucketType = (currHwBucketPtr->bucketData[i+hwBitMapSize] >> 3) & 0x3;
                }

                if((((*pRange)->pointerType <= CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)&&(hwPointerType!=0))||
                   (((*pRange)->pointerType==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)&&(hwPointerType!=1))||
                   (((*pRange)->pointerType==CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E)&&(hwPointerType!=2))||
                   (((*pRange)->pointerType==CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E)&&(hwPointerType!=3)))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - range Pointer Type value\n");
                }
                else
                {
                     /* We will dive deep into the tree until we get to the bucket that hold a leaf.
                        if the leaf is regular we need to check if the NH_pointer and the
                        “NH additional data” match the data kept in the shadow.
                        If the leaf is ECMP/QOS we need to check if the ECMP_pointer and the
                        “NH additional data” match the data kept in the shadow. */

                    if (((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                        ((*pRange)->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                        ((*pRange)->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
                        ((GT_U32)((*pRange)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                    {
                        /* need to check that SW is sync with HW */

                        /* If the pointerType is to a MC source and the MC source bucket is a regular bucket then it means that
                           this is a root of MC source tree that points directly to a nexthop or ECMP/QoS entry */
                        if (((GT_U32)(*pRange)->pointerType==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E) &&
                            ((*pRange)->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E))
                        {
                            retVal = checkMcRootSource(currHwBucketPtr->bucketData[i+hwBitMapSize],
                                                       (*pRange)->lowerLpmPtr.nextBucket->rangeList->lowerLpmPtr.nextHopEntry);
                            if (retVal!=GT_OK)
                            {
                                return retVal;
                            }

                            /* update a range was handle only in case this is a pointer to a next hop
                               other whise the counter will be updated when we finish investigation the entire bucket */
                            (*currHwBucketsNumberOfRangesHandledPtr)++;
                        }
                        else
                        {
                            switch (hwPointerType)
                            {
                                case 0:/* pointing to bucket - meaning MC pointing to SRC root */
                                    /* check the bucket type: regular/comp_1/comp_2 */
                                    retVal = getHwNextBucketParameters(currHwBucketPtr->bucketData[i+hwBitMapSize],
                                                                       hwNextBucketType,
                                                                       (*pRange)->lowerLpmPtr.nextBucket->fifthAddress,
                                                                       &hwAddr,&hwBitMapSize,&hwNodeType,&hwBucketFifthStartAddr);
                                    if (retVal!=GT_OK)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "Error: getHwNextBucketParameters \n");
                                    }

                                    /* we can not check the bucket address since we didnt update the shadow according to HW yet
                                       this will be done in the end after all ranges of the level will be sync
                                       for now keep the hwAdd to be updated later after DMM will be alloctaed */
                                    currHwBucketsAddressPtr[1]=hwAddr;

                                    /* Insert the addr. to the existing next bucket. */
                                    currBucketPtr[1] = (*pRange)->lowerLpmPtr.nextBucket;
                                    currBucketPtrType[1] = &((*pRange)->pointerType);

                                    /* calculate the next bucket ranges from HW*/

                                    /* read the bit vector */
                                    retVal = readBitVectorData(devNum, hwAddr, hwNodeType, hwBitMapSize,
                                                               &(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiBucketsDataArr)[level+1].bucketData[0]),
                                                               PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(duplicateBucketData), &hwNumOfRanges);
                                    if (retVal != GT_OK)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "HW and SW are not Sync error in reading bit "
                                                                              "vector from HW, hwRootNodeAddr[%d]\n",hwAddr);
                                    }

                                    currHwBucketsNumberOfRangesPtr[1]=hwNumOfRanges;

                                    /* check HW and SW sync */
                                    if (currBucketPtr[1]->numOfRanges!=hwNumOfRanges)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "HW and SW are not Sync after HA bucketPtr->numOfRanges[%d]!=hwNumOfRanges[%d]\n",bucketPtr->numOfRanges,hwNumOfRanges);
                                    }
                                    /* read the ranges (the LPM lines after the bit vector) */
                                    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges).ranges, 0, sizeof(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges).ranges));
                                    retVal = readBucketData(devNum,hwAddr,hwNodeType,hwBitMapSize,
                                                            hwNumOfRanges,hwBucketFifthStartAddr,
                                                            PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiBucketsDataArr)[level+1].bucketData,
                                                            PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(duplicateBucketData),
                                                            &(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges)));
                                    if (retVal != GT_OK)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "HW and SW are not Sync error in reading ranges from HW, "
                                                                              "hwAddr[%d]\n",hwAddr);
                                    }

                                    cpssOsMemCpy(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiRangesArr)[level+1].ranges,
                                                 PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges).ranges,
                                                 sizeof(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges).ranges));

                                    /* indicate we need to proceed down the bucket trie*/
                                    goDown = GT_TRUE;
                                   break;

                               case 0x1:/* CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E */
                                    /* 5. check that if a SW ranges is of type NH then HW range should point to the same NH */
                                    hwNhPointer = ((currHwBucketPtr->bucketData[i+hwBitMapSize] >> 7) & 0x00007FFF);
                                    if ((*pRange)->lowerLpmPtr.nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - Pointer value\n");
                                    }
                                    /* update a range was handle only in case this is a pointer to a next hop
                                    other whise the counter will be updated when we finish investigation the entire bucket */
                                    (*currHwBucketsNumberOfRangesHandledPtr)++;
                                    break;
                                case 0x2:/* CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E */
                                case 0x3:/* CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E */
                                    /* 5. check that if a SW ranges is of type ECMP/QOS then HW range should point to the same NH */
                                    hwNhPointer = ((currHwBucketPtr->bucketData[i+hwBitMapSize] >> 7) & 0x00003FFF);
                                    if ((*pRange)->lowerLpmPtr.nextHopEntry->routeEntryBaseMemAddr != hwNhPointer)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - Pointer value\n");
                                    }
                                    /* update a range was handle only in case this is a pointer to a next hop
                                    other whise the counter will be updated when we finish investigation the entire bucket */
                                    (*currHwBucketsNumberOfRangesHandledPtr)++;
                                    break;
                              default:
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - error in HW Bucket Pointer Type\n");
                                    break;
                            }
                        }
                    }
                    else
                    {
                        /* If we reach a bucket and not a leaf we continue deeper in the tree.
                           if the bucket is “trigger for MC source group” then we should continue
                           deeper in the tree to deal with the source. if the bucket is compressed_2
                           we should look at the “two compressed last offset” field in order to
                           calculate the number of ranges of the next bucket and we will use
                           “two compressed Next Bucket Pointer”.
                           If the bucket is regular or compressed_1 we will use
                           “Next bucket pointer” field to reach the next bucket. */

                        switch (hwPointerType)
                        {
                            case 0:/* pointing to bucket */
                                /* check the bucket type: regular/comp_1/comp_2 */
                                retVal = getHwNextBucketParameters(currHwBucketPtr->bucketData[i+hwBitMapSize],
                                                                   hwNextBucketType,
                                                                   (*pRange)->lowerLpmPtr.nextBucket->fifthAddress,
                                                                   &hwAddr,&hwBitMapSize,&hwNodeType,&hwBucketFifthStartAddr);
                                if (retVal!=GT_OK)
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "Error: getHwNextBucketParameters \n");
                                }

                                /* we can not check the bucket address since we didnt update the shadow according to HW yet
                                   this will be done in the end after all ranges of the level will be sync
                                   for now keep the hwAdd to be updated later after DMM will be alloctaed */
                                currHwBucketsAddressPtr[1]=hwAddr;
                                break;
                            case 0x1:/* CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E */
                            case 0x2:/* CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E */
                            case 0x3:/* CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E */
                            default:
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - error in HW Bucket Pointer Type\n");
                                break;

                        }

                        /* Insert the addr. to the existing next bucket. */
                        currBucketPtr[1] = (*pRange)->lowerLpmPtr.nextBucket;
                        currBucketPtrType[1] = &((*pRange)->pointerType);

                        /* calculate the next bucket ranges from HW*/

                        /* read the bit vector */
                        retVal = readBitVectorData(devNum, hwAddr, hwNodeType, hwBitMapSize,
                                                   &(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiBucketsDataArr)[level+1].bucketData[0]),
                                                   PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(duplicateBucketData),&hwNumOfRanges);
                        if (retVal != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "HW and SW are not Sync error in reading bit "
                                                                  "vector from HW, hwRootNodeAddr[%d]\n",hwAddr);
                        }

                        currHwBucketsNumberOfRangesPtr[1]=hwNumOfRanges;

                        /* check HW and SW sync */
                        if (currBucketPtr[1]->numOfRanges!=hwNumOfRanges)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "HW and SW are not Sync after HA bucketPtr->numOfRanges[%d]!=hwNumOfRanges[%d]\n",bucketPtr->numOfRanges,hwNumOfRanges);
                        }
                        /* read the ranges (the LPM lines after the bit vector) */
                        cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges).ranges, 0,
                                     sizeof(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges).ranges));
                        retVal = readBucketData(devNum,hwAddr,hwNodeType,hwBitMapSize,
                                                hwNumOfRanges,hwBucketFifthStartAddr,
                                                PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiBucketsDataArr)[level+1].bucketData,
                                                PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(duplicateBucketData),
                                                &(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges)));
                        if (retVal != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "HW and SW are not Sync error in reading ranges from HW, "
                                                                  "hwAddr[%d]\n",hwAddr);
                        }

                        cpssOsMemCpy(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiRangesArr)[level+1].ranges,
                                     PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges).ranges,
                                     sizeof(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwRanges).ranges));

                        /* indicate we need to proceed down the bucket trie*/
                        goDown = GT_TRUE;
                    }
                }

                 /* update the HW is not needed --> change the flag */
                (*pRange)->updateRangeInHw = GT_FALSE;
                *pRange = (*pRange)->next;

            }while (((*pRange) != NULL) &&
                    (goDown == GT_FALSE));
        }

        /* check what direction we go in the tree */
        if (goDown == GT_TRUE)
        {
            /* we're going down. so advance in the pointer arrays */
            currBucketPtr++;
            currBucketPtrType++;
            currHwBucketsNumberOfRangesPtr++;
            currHwBucketsNumberOfRangesHandledPtr++;
            pRange++;
            currHwBucketPtr++;
            currHwRangePtr++;
            currHwBucketsAddressPtr++;
            level++;
        }
        else
        {
            /* we're going up, meaning we finished with the current level.
               write it to the HW and move back the pointers in the arrays*/

            /* When the counter reached the number of ranges defined we will
               allocated the bucket in the DMM and we will reset the element
                in the arrays since all its ranges were handled */

            /* check we went over all ranges */
            if (*currHwBucketsNumberOfRangesPtr != *currHwBucketsNumberOfRangesHandledPtr)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - not all ranges were handel\n");
            }

            /* if the range pointer reached NULL means that we finished with this level */
            /* need to allocate DMM and update hwBucketOffsetHandle*/

            retVal = getSwBucketInfo((*currBucketPtr),(*currBucketPtr)->bucketType, &swBucketMemSize, &swBucketWriteOffset, &swBucketFifthStartAddr);
            if (retVal!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "Error in getBucketInfo \n");
            }

            /* if hwBucketOffsetHandle was already sync no need to allocate DMM again
               this can happen in case of MC Root sync after UC Root sync */
            if ((*currBucketPtr)->hwBucketOffsetHandle == 0)
            {
                retVal = prvCpssDxChLpmRamMngHaSwPointerAllocateAccordingtoHwAddressSip5(shadowPtr,
                                                                                         *currHwBucketsAddressPtr,
                                                                                         swBucketMemSize,
                                                                                         protocol, level, prefixType, (*currBucketPtr));
                if (retVal!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "Error in prvCpssDxChLpmRamMngHaSwPointerAllocateAccordingtoHwAddressSip5 \n");
                }
            }

            /* before moving back in the array we need to clean the counter for next use */
            (*currHwBucketsNumberOfRangesHandledPtr)=0;
            (*currHwBucketsNumberOfRangesPtr)=0;
            /* move back in the array */
            currHwBucketsNumberOfRangesHandledPtr--;
            currHwBucketsNumberOfRangesPtr--;
            /* after moving back in the array we need to update that we finish to handle one range */
            (*currHwBucketsNumberOfRangesHandledPtr)++;

            /* move back in the array - after it we have:
               the address we need to update the pointingRangeMemAddr
               and the type of the pointing bucket to calculate the offset */
            currHwBucketsAddressPtr--;
            currBucketPtrType--;

            if (level!=0)
            {

                /* according to the number of ranges keept in the
                   array we can calculate the offset of the bucket */
                if ((*currHwBucketsNumberOfRangesPtr)<=MAX_NUMBER_OF_COMPRESSED_1_RANGES_CNS)
                {
                    swBucketWriteOffset = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_1_CNS;
                }
                else
                {
                    if ((*currHwBucketsNumberOfRangesPtr)<=MAX_NUMBER_OF_COMPRESSED_2_RANGES_CNS)
                    {
                        swBucketWriteOffset = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_COMPRESSED_2_CNS;
                    }
                    else
                    {
                        swBucketWriteOffset = PRV_CPSS_DXCH_LPM_RAM_BUCKET_BIT_VEC_SIZE_REGULAR_CNS;
                    }
                }

                /* update pointingRangeMemAddr - need to do the update from previous level */
                (*currBucketPtr)->pointingRangeMemAddr =
                    *currHwBucketsAddressPtr + (swBucketWriteOffset + (*currHwBucketsNumberOfRangesHandledPtr) - 1);
            }
            else
            {
                /* pointingRangeMemAddr is always 0 for Root bucket */
                (*currBucketPtr)->pointingRangeMemAddr = 0;
            }

            /* move back in the arrays*/
            currBucketPtr--;
            currHwBucketPtr--;
            currHwRangePtr--;

            /* before we go up , indicate this level is finished, and ready
               for a fresh level if needed in the future */
            *pRange = (PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *)(~((GT_UINTPTR)0));
            pRange--;
            /* before moving back in the array we need to reset arrays for next use */
            cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiRangesArr)[level].ranges, 0, sizeof(GT_U32)*256);
            cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(hwMultiBucketsDataArr)[level].bucketData, 0, sizeof(GT_U32)*PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS);
            level--;
        }
    }

    return GT_OK;
}

/**
* @internal
*           prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5_Clean function
* @endinternal
*
* @brief   in case of a fail - clean the hwOffsetHandle already allocated and set in the
*
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
*
* @retval GT_FAIL                     - on failure
* @retval GT_OK                       - on success
*/
GT_STATUS prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5_Clean
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol
)
{
    GT_U32                                          level = 0;
    GT_BOOL                                         goDown;

    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         **currBucketPtr = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArr);
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             **currBucketPtrType = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArr);
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          **pRange;

    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             swRootBucketType; /* the 1st level bucket type  */

    GT_U32                                          tmpPrefixTypeRange;
    GT_U8                                           prefixTypeFirstRange, prefixTypeLastRange;
    GT_U8                                           prefixTypeSecondRange=0, prefixTypeSecondLastRange=0;


    swRootBucketType = shadowPtr->vrRootBucketArray[vrId].rootBucketType[protocol];

    /* zero the arrays */
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArr),0,sizeof(PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC*)*MAX_LPM_LEVELS_CNS*2);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArr),0xFF, /*0xFFFFFFFF indicates untouched level*/
                 sizeof(PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArr)));

    /* set the initial values.*/
    PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArr)[0] = bucketPtr;
    PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(pointerTypePtrArr)[0] = &swRootBucketType;
    pRange = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArr);

    /* go over all the lpm tree and update the shadow according to HW */
    while (currBucketPtr >= PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(bucketPtrArr))
    {
         if (level == 0)
         {
             if (prefixType == CPSS_UNICAST_E)
             {
                 prefixTypeFirstRange = 0;
                 tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                     (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS - 1) :
                     (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS - 1);
                 prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

                 if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
                 {
                     prefixTypeSecondRange = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
                     prefixTypeSecondLastRange = PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
                 }
             }
             else /* CPSS_MULTICAST_E */
             {
                 tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                     PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                     PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
                 prefixTypeFirstRange = (GT_U8)tmpPrefixTypeRange;
                 tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                     PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                     PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS;
                 prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

                 prefixTypeSecondLastRange = prefixTypeLastRange;
             }
         }
         else
         {
             prefixTypeFirstRange = 0;
             prefixTypeLastRange = 255;
             prefixTypeSecondRange = 0;
             prefixTypeSecondLastRange = 255;

         }

        /* we start with the assumption we'll go up the bucket trie */
        goDown = GT_FALSE;

        /* if the range pointer reached NULL means that we finished with this level */
        if (*pRange != NULL)
        {
            /* check if we need to continue this level or this a brand new level (0xFFFFFFFF) */
            if (*pRange == (PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *)(~((GT_UINTPTR)0)))
            {
                *pRange=(*currBucketPtr)->rangeList;
            }
            do
            {
               /* Skip unicast ranges for multicast sync or multicast ranges for unicast sync */
                if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
                {
                    if (((*pRange)->startAddr < prefixTypeFirstRange) ||
                        (((*pRange)->startAddr > prefixTypeLastRange) && ((*pRange)->startAddr<prefixTypeSecondRange))||
                         ((*pRange)->startAddr > prefixTypeSecondLastRange))
                    {
                        *pRange = (*pRange)->next;
                        continue;
                    }
                }
                else
                {
                    if (((*pRange)->startAddr < prefixTypeFirstRange) ||
                        ((*pRange)->startAddr > prefixTypeLastRange))
                    {
                        *pRange = (*pRange)->next;
                        continue;
                    }
                }

                 /* We will dive deep into the tree until we get to the bucket that hold a leaf.
                    if the leaf is regular we need to check if the NH_pointer and the
                     NH additional data” match the data kept in the shadow.
                    If the leaf is ECMP/QOS we need to check if the ECMP_pointer and the
                     NH additional data” match the data kept in the shadow. */

                if (((*pRange)->pointerType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) ||
                    ((*pRange)->pointerType == CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E) ||
                    ((*pRange)->pointerType == CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E) ||
                    ((GT_U32)((*pRange)->pointerType) == (GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                {
                    /* need to check that SW is sync with HW */

                    /* If the pointerType is to a MC source and the MC source bucket is a regular bucket then it means that
                       this is a root of MC source tree that points directly to a nexthop or ECMP/QoS entry */
                    if (((GT_U32)(*pRange)->pointerType==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E) &&
                        ((*pRange)->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E))
                    {
                        /* nothing to clear */
                    }
                    else
                    {
                        if (((*pRange)->pointerType<=CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E)||
                            ((GT_U32)(*pRange)->pointerType==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E))
                        {
                            /* Insert the addr. to the existing next bucket. */
                                currBucketPtr[1] = (*pRange)->lowerLpmPtr.nextBucket;
                                currBucketPtrType[1] = &((*pRange)->pointerType);

                                /* indicate we need to proceed down the bucket trie*/
                                goDown = GT_TRUE;
                        }
                        else
                        {
                             /* nothing to clear */
                        }
                    }
                }
                else
                {
                     /* If we reach a bucket and not a leaf we continue deeper in the tree.
                       if the bucket is “trigger for MC source group” then we should continue
                       deeper in the tree to deal with the source. if the bucket is compressed_2
                       we should look at the “two compressed last offset” field in order to
                       calculate the number of ranges of the next bucket and we will use
                        two compressed Next Bucket Pointer”.
                       If the bucket is regular or compressed_1 we will use
                        Next bucket pointer” field to reach the next bucket. */

                    /* Insert the addr. to the existing next bucket. */
                    currBucketPtr[1] = (*pRange)->lowerLpmPtr.nextBucket;
                    currBucketPtrType[1] = &((*pRange)->pointerType);

                    /* indicate we need to proceed down the bucket trie*/
                    goDown = GT_TRUE;
                }


                 /* update the HW is not needed --> change the flag */
                (*pRange)->updateRangeInHw = GT_FALSE;
                *pRange = (*pRange)->next;

            }while (((*pRange) != NULL) &&
                    (goDown == GT_FALSE));
        }

        /* check what direction we go in the tree */
        if (goDown == GT_TRUE)
        {
            /* we're going down. so advance in the pointer arrays */
            currBucketPtr++;
            currBucketPtrType++;
            pRange++;
            level++;
        }
        else
        {
            /* we're going up, meaning we finished with the current level.
               write it to the HW and move back the pointers in the arrays*/

            /* due to a fail in the sync operation we need to clear all
               hwBucketOffsetHandle in the SW tree */

            /* ******** THIS IS THE PURPOSE OF THE FUNCTION ***************************** */
            if ((*currBucketPtr)->hwBucketOffsetHandle !=0 )
            {
                prvCpssDmmFree((*currBucketPtr)->hwBucketOffsetHandle);
            }
            (*currBucketPtr)->hwBucketOffsetHandle = 0;
           /* ************************************************************************** */

            /* move back in the array - after it we have:
               the address we need to update the pointingRangeMemAddr
               and the type of the pointing bucket to calculate the offset */
            currBucketPtrType--;

            /* ******** reset pointingRangeMemAddr for all levels ********************** */
            (*currBucketPtr)->pointingRangeMemAddr = 0;
            /* ************************************************************************* */

            /* move back in the arrays*/
            currBucketPtr--;

            /* before we go up , indicate this level is finished, and ready
               for a fresh level if needed in the future */
            *pRange = (PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *)(~((GT_UINTPTR)0));
            pRange--;

            level--;
        }
    }

    return GT_OK;
}

/**
* @internal
*           prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5 function
* @endinternal
*
* @brief  Update SW Shadow with relevant data from HW, and
*         allocate DMM memory according to HW memory for a
*         specific vrId. Relevant for HA process
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] shadowPtr             - the shadow to work on.
* @param[in] vrId                  - The virtual's router ID.
* @param[in] protocolStack         - type of ip protocol stack to work on.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
*/
static GT_STATUS prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    IN GT_U32                               vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocolStack
)
{
    GT_STATUS                                rc=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr;

    /* read the HW data directly from the VRF table and compare it to the SW data in the shadow
       since LPM shadow has a shared device list - we will take the first device in the list
       to be used in calling prvCpssDxChLpmHwVrfEntryRead API */

    if(shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protocolStack]==GT_TRUE)
    {
        bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];

        rc = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5_Sync(shadowPtr->shadowDevList.shareDevs[0],
                                                                            shadowPtr,
                                                                            vrId,
                                                                            bucketPtr,
                                                                            CPSS_UNICAST_E,
                                                                            protocolStack);

        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /* FCOE does not support Multicast */
    if ((protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)||(protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E))
    {
        if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protocolStack] == GT_TRUE)
        {
            bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];

            rc = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5_Sync(shadowPtr->shadowDevList.shareDevs[0],
                                                                                shadowPtr,
                                                                                vrId,
                                                                                bucketPtr,
                                                                                CPSS_MULTICAST_E,
                                                                                protocolStack);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return rc;
}

/**
* @internal
*           prvCpssDxChLpmRamCleanSwAfterHaFailPerVrIdAndProtocolSip5 function
* @endinternal
*
* @brief  Clean SW tree after HA fail
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] shadowPtr             - the shadow to work on.
* @param[in] vrId                  - The virtual's router ID.
* @param[in] protocolStack         - type of ip protocol stack to work on.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
*/
static GT_STATUS prvCpssDxChLpmRamCleanSwAfterHaFailPerVrIdAndProtocolSip5
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    IN GT_U32                               vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocolStack
)
{
    GT_STATUS                                rc=GT_OK;
    GT_STATUS                                rc1=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr;

    if(shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protocolStack]==GT_TRUE)
    {
        bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];

       rc = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5_Clean(shadowPtr,
                                                                            vrId,
                                                                            bucketPtr,
                                                                            CPSS_UNICAST_E,
                                                                            protocolStack);

        if (rc != GT_OK)
        {
            /* must free the allocated/bound RAM memory - done in previous stages */
            rc1 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFreeForHa(shadowPtr,protocolStack);
            if (rc1!=GT_OK)
            {
                cpssOsPrintf("failed to free allocated/bound RAM memory \n");
            }
            /* in case of fail we will need to recondtruct to the status we had before the call to swPointerAllocateAccordingtoHwAddress */
            cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

            cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

            cpssOsPrintf("fail to clean SW UNICAST tree\n");
            return rc;
        }
    }
    /* FCOE does not support Multicast */
    if ((protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)||(protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E))
    {
        if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protocolStack] == GT_TRUE)
        {
            bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];

            rc = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5_Clean(shadowPtr,
                                                                                vrId,
                                                                                bucketPtr,
                                                                                CPSS_MULTICAST_E,
                                                                                protocolStack);
            if (rc != GT_OK)
            {
                /* must free the allocated/bound RAM memory - done in previous stages */
                rc1 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFreeForHa(shadowPtr,protocolStack);
                if (rc1!=GT_OK)
                {
                    cpssOsPrintf("failed to free allocated/bound RAM memory \n");
                }
                /* in case of fail we will need to recondtruct to the status we had before the call to swPointerAllocateAccordingtoHwAddress */
                cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                             sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

                cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

                cpssOsPrintf("fail to clean SW MULTICAST tree\n");
                return rc;
            }
        }
    }

    /* must free the allocated/bound RAM memory - done in previous stages */
    rc = prvCpssDxChLpmRamMngAllocatedAndBoundMemFreeForHa(shadowPtr,protocolStack);
    if (rc!=GT_OK)
    {
        cpssOsPrintf("failed to free allocated/bound RAM memory \n");
    }
    /* in case of fail we will need to recondtruct to the status we had before the call to swPointerAllocateAccordingtoHwAddress */
    cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

    cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

    return rc;
}

/**
* @internal
*           prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol
*           function
* @endinternal
*
* @brief  Update SW Shadow with relevant data from HW, and
*         allocate DMM memory according to HW memory for a
*         specific vrId. Relevant for HA process
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] shadowPtr             - the shadow to work on.
* @param[in] vrId                  - The virtual's router ID.
* @param[in] protocolStack         - type of ip protocol stack to work on.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
*/
static GT_STATUS prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    IN GT_U32                               vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocolStack
)
{
    GT_STATUS                                rc=GT_OK;
    GT_U32                                   hwBucketDataArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS*PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
    GT_U32                                   gonNodeAddr=0; /* Node address to read the HW data from */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr;
    GT_U8                                    numOfMaxAllowedLevels;

    /* read the HW data directly from the VRF table and compare it to the SW data in the shadow
       since LPM shadow has a shared device list - we will take the first device in the list
       to be used in calling prvCpssDxChLpmHwVrfEntryReadSip6 API */

    if(shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protocolStack]==GT_TRUE)
    {
        bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];
        switch(protocolStack)
        {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            numOfMaxAllowedLevels = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            numOfMaxAllowedLevels = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            numOfMaxAllowedLevels = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "protocol type not supported for HA sync");

        }

        cpssOsMemSet(&hwBucketDataArr[0], 0, sizeof(hwBucketDataArr));

        /* keep values in case reconstruct is needed */
        cpssOsMemCpy(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,shadowPtr->lpmRamOctetsToBlockMappingPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

        rc = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucket(shadowPtr->shadowDevList.shareDevs[0],
                                                                            shadowPtr,
                                                                            vrId,
                                                                            bucketPtr,
                                                                            gonNodeAddr,    /* not used in the first call */
                                                                            &hwBucketDataArr[0],
                                                                            0,              /* level */
                                                                            numOfMaxAllowedLevels,
                                                                            CPSS_UNICAST_E,
                                                                            protocolStack,
                                                                            GT_TRUE);       /* isRootBucket */

        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /* FCOE does not support Multicast */
    if ((protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)||(protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E))
    {
        if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protocolStack] == GT_TRUE)
        {
            bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];
            switch(protocolStack)
            {
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
                numOfMaxAllowedLevels = 2*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
                numOfMaxAllowedLevels = 2*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "protocol type not supported for HA sync");

            }
            /* keep values in case reconstruct is needed */
            cpssOsMemCpy(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,shadowPtr->lpmRamOctetsToBlockMappingPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

            rc = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucket(shadowPtr->shadowDevList.shareDevs[0],
                                                                                shadowPtr,
                                                                                vrId,
                                                                                bucketPtr,
                                                                                gonNodeAddr,    /* not used in the first call */
                                                                                &hwBucketDataArr[0],
                                                                                0,              /* level */
                                                                                numOfMaxAllowedLevels,
                                                                                CPSS_MULTICAST_E,
                                                                                protocolStack,
                                                                                GT_TRUE);       /* isRootBucket */
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return rc;
}
/**
* @internal prvCpssDxChLpmRamSyncSwHwForHa function
* @endinternal
*
* @brief  Update SW Shadow with relevant data from HW, and
*         allocate DMM memory according to HW memory.
*         Relevant for HA process
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5;
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
*
*/
GT_STATUS prvCpssDxChLpmRamSyncSwHwForHa
(
    GT_VOID
)
{
    GT_STATUS       rc,rc1;                                             /* return code */
    GT_U32          lpmDbId = 0;                                    /* LPM DB index */
    GT_UINTPTR      slIterator;                                     /* Skip List iterator */
    PRV_CPSS_DXCH_LPM_SHADOW_STC            *lpmDbPtr,tmpLpmDb;     /* pointer to and temp instance of LMP DB entry */
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *ipShadowPtr;           /* pointer to shadow entry */
    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC    *ipShadowsPtr;          /* pointer to shadows DB */

    GT_U32                                  shadowIdx;              /* shadow index */
    GT_U32                                  i;                      /* loop iterator */
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT        shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT              protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT      memoryCfg;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    currProtocolStack;

    cpssOsMemSet(&memoryCfg,0,sizeof(memoryCfg));
    if (PRV_SHARED_PORT_DIR_IP_LPM_SRC_GLOBAL_VAR_GET(lpmDbSL) == NULL)
    {
        CPSS_LOG_INFORMATION_MAC("in prvCpssDxChLpmRamSyncSwHwForHa- prvCpssDxChIplpmDbSL not initialized");
        return GT_OK;
    }
    slIterator = 0;
    while (1)
    {
        rc =  prvCpssDxChIpLpmDbIdGetNext(&lpmDbId,&slIterator);
        if (rc == GT_NO_MORE)
        {
            break;
        }
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChLpmRamSyncSwHwForHa-prvCpssDxChIpLpmDbIdGetNext");
        }

        rc = cpssDxChIpLpmDBConfigGet(lpmDbId,&shadowType,&protocolStack,&memoryCfg);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChLpmRamSyncSwHwForHa-cpssDxChIpLpmDBConfigGet");
        }
        if ((shadowType != CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E)&&
            (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E))
        {
            return GT_OK;
        }
        /* check parameters */
        tmpLpmDb.lpmDbId = lpmDbId;
        lpmDbPtr = prvCpssSlSearch(PRV_SHARED_PORT_DIR_IP_LPM_SRC_GLOBAL_VAR_GET(lpmDbSL),&tmpLpmDb);
        if (lpmDbPtr == NULL)
        {
            /* can't find the lpm DB */
            return GT_OK;
        }

        ipShadowsPtr = lpmDbPtr->shadow;

        for (shadowIdx = 0; shadowIdx < ipShadowsPtr->numOfShadowCfg; shadowIdx++)
        {
            ipShadowPtr = &ipShadowsPtr->shadowArray[shadowIdx];
            if (ipShadowPtr->shadowDevList.shareDevNum==0)
            {
                /* There is no added devices */
                CPSS_LOG_INFORMATION_MAC("in prvCpssDxChLpmRamSyncSwHwForHa- no added devices: shareDevNum=%d", ipShadowPtr->shadowDevList.shareDevNum);
                return GT_OK;
            }
            for(i = 0; i < ipShadowPtr->vrfTblSize; i++)
            {
                for (currProtocolStack = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
                      currProtocolStack < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
                      currProtocolStack++)
                {
                    if ((isProtocolSetInBitmap(ipShadowsPtr->protocolBitmap, currProtocolStack) == GT_FALSE) ||
                        (ipShadowPtr->isProtocolInitialized[currProtocolStack] == GT_FALSE))
                    {
                        continue;
                    }
                    /* go over all valid VR , if it's initilized for this protocol */
                    if ((ipShadowPtr->vrRootBucketArray[i].valid == GT_TRUE) &&
                        (ipShadowPtr->vrRootBucketArray[i].rootBucket[currProtocolStack] != NULL))
                    {
                        if (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)
                        {
                            /* keep values in case reconstruct is needed */
                            cpssOsMemCpy(ipShadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,ipShadowPtr->lpmRamOctetsToBlockMappingPtr,
                                         sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*ipShadowPtr->numOfLpmMemories);

                            rc = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5(ipShadowPtr, i, currProtocolStack);
                            if (rc!=0)
                            {
                                rc1 = prvCpssDxChLpmRamCleanSwAfterHaFailPerVrIdAndProtocolSip5(ipShadowPtr, i, currProtocolStack);
                                if (rc1!=0)
                                {
                                    /* reset pending array for future use */
                                    cpssOsMemSet(ipShadowPtr->pendingBlockToUpdateArr,0,sizeof(ipShadowPtr->pendingBlockToUpdateArr));

                                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5 and "
                                                                      "prvCpssDxChLpmRamCleanSwAfterHaFailPerVrIdAndProtocolSip5");
                                }
                            }
                            cpssOsMemSet(ipShadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr,0,sizeof(ipShadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));
                            cpssOsMemSet(ipShadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(ipShadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

                        }
                        else
                        {
                            rc = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol(ipShadowPtr, i, currProtocolStack);
                        }
                        if (rc != GT_OK)
                        {
                            /* reset pending array for future use */
                            cpssOsMemSet(ipShadowPtr->pendingBlockToUpdateArr,0,sizeof(ipShadowPtr->pendingBlockToUpdateArr));

                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChLpmRamSyncSwHwForHa-prvCpssDxChLpmRamSyncSwHwForHaCheckSingleVrId");
                        }

                        rc = prvCpssDxChLpmRamUpdateBlockUsageCounters(ipShadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                ipShadowPtr->pendingBlockToUpdateArr,
                                                                ipShadowPtr->protocolCountersPerBlockArr,
                                                                ipShadowPtr->pendingBlockToUpdateArr,
                                                                currProtocolStack,
                                                                ipShadowPtr->numOfLpmMemories);
                        if (rc!=GT_OK)
                        {
                            /* reset pending array for future use */
                            cpssOsMemSet(ipShadowPtr->pendingBlockToUpdateArr,0,sizeof(ipShadowPtr->pendingBlockToUpdateArr));

                            return rc;
                        }
                    }
                }
            }
        }
    }
    return GT_OK;
}
