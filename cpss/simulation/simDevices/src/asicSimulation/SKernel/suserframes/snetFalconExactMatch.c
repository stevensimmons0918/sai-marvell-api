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
* @file snetFalconExactMatch.c
*
* @brief SIP6 Falcon Exact Match
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>
#include <asicSimulation/SLog/simLogInfoTypeDevice.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahPcl.h>
#include <asicSimulation/SKernel/suserframes/snetFalconExactMatch.h>
#include <asicSimulation/SKernel/suserframes/snetHawkExactMatch.h>
#include <asicSimulation/SKernel/cheetahCommon/sregFalcon.h>
#include <asicSimulation/SKernel/suserframes/snetLion3Tcam.h>
#include <common/Utils/PresteraHash/smacHashExactMatch.h>
#include <asicSimulation/SKernel/cheetahCommon/sregHawk.h>

static GT_CHAR* tcamClientName[SIP5_TCAM_CLIENT_LAST_E + 1] =
{
    STR(SIP5_TCAM_CLIENT_TTI_E   ),
    STR(SIP5_TCAM_CLIENT_IPCL0_E ),
    STR(SIP5_TCAM_CLIENT_IPCL1_E ),
    STR(SIP5_TCAM_CLIENT_IPCL2_E ),
    STR(SIP5_TCAM_CLIENT_EPCL_E  ),

    NULL
};

#define SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_NAME                       \
     STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_VALID                    )\
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE               )\
     /*EM ACTION*/                                                    \
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_AGE            )\
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_SIZE       )\
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_LOOKUP_NUMBER  )\
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_31_0       )\
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_39_32      )\
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_31_0    )\
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_63_32   )\
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_67_64   )\
     /*EM KEY*/                                                       \
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_31_0          )\
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_63_32         )\
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_95_64         )\
    ,STR(SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_111_96        )

static char * sip6ExactMatchFieldsTableNames[SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6ExactMatchTableFieldsFormat[SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS___LAST_VALUE___E] =
{
    /*SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_VALID*/
    STANDARD_FIELD_MAC(1)
    /*SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE*/
    ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_AGE*/
        ,{FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE}
    /*SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_SIZE*/
    ,STANDARD_FIELD_MAC(2)
   /*   SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_LOOKUP_NUMBER*/
        ,STANDARD_FIELD_MAC(1)
    /*    SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_31_0*/
        ,STANDARD_FIELD_MAC(32)
    /*    SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_39_32*/
        ,STANDARD_FIELD_MAC(8)
    /*    SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_31_0*/
        ,STANDARD_FIELD_MAC(32)
    /*    SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_63_32*/
        ,STANDARD_FIELD_MAC(32)
    /*    SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_67_64*/
        ,STANDARD_FIELD_MAC(4)

    /*    SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_31_0*/
        ,{FIELD_SET_IN_RUNTIME_CNS,
         32,
         SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE}
    /*    SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_63_32*/
        ,STANDARD_FIELD_MAC(32)
    /*    SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_95_64*/
        ,STANDARD_FIELD_MAC(32)
    /*    SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_111_96*/
        ,STANDARD_FIELD_MAC(16)

};

/**
* @internal snetExactMatchTablesFormatInit function
* @endinternal
*
* @brief init the format of Exact Match tables.
*
* @param[in] devObjPtr      - pointer to device object.
*/
void snetExactMatchTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
)
{

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_EXACT_MATCH_ENTRY_E,
            sip6ExactMatchTableFieldsFormat, sip6ExactMatchFieldsTableNames);
    }
}

/**
* @internal snetFalconExactMatchProfileIdGet function
* @endinternal
*
* @brief   Gets Exact Match Lookup Profile ID1/ID2 for TTI
*          PCL and EPCL packet type.
*
* @param[in] devObjPtr                      - pointer to device object.
* @param[in] client                         - tcam client (TTI,PCL,EPCL)
* @param[in] index                          - keyType in case of TTI
*                                             or index for Exact match Profile-ID mapping table
*                                             in case of PCL/EPCL
* @param[out] exactMatchProfileIdArr        - Exact Match Lookup
*                                             Profile ID Array
*/
GT_VOID snetFalconExactMatchProfileIdGet
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr ,
    IN  SIP5_TCAM_CLIENT_ENT               client,
    IN  GT_U32                             index,
    OUT GT_U32                             exactMatchProfileIdArr[]
)
{
    DECLARE_FUNC_NAME(snetFalconExactMatchProfileIdGet);
    GT_U32  regAddr,profileId1=0,profileId2=0;
    GT_U32 * regPtr;/* register entry pointer */

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        switch (client)
        {
        case SIP5_TCAM_CLIENT_TTI_E:
            /* keyIndex: 0-19 */
            if(index <= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_15_E)
            {
                GT_U32 regVal;
                /* get profileId1, profileId2 */
                regAddr = SMEM_SIP6_TTI_PACKET_TYPE_EXACT_MATCH_PROFILE_ID_REG(devObjPtr, (index)/2);
                smemRegGet(devObjPtr,regAddr , &regVal);

                /* 4 bits for profileId1 (bits 0-3 for even index, bits 8-11 for odd index)
                   4 bits for profileId2 (bits 4-7 for even index, bits 12-15 for odd index)*/
                profileId1 = SMEM_U32_GET_FIELD(regVal,8*(index%2), 4);
                profileId2 = SMEM_U32_GET_FIELD(regVal,4 + 8*(index%2), 4);

            }
            break;

        case SIP5_TCAM_CLIENT_IPCL0_E:
        case SIP5_TCAM_CLIENT_IPCL1_E:
        case SIP5_TCAM_CLIENT_IPCL2_E:
            /* sip6    : keyIndex: 0-127 */
            /* sip6_10 : keyIndex: 0-255 */
            if(index < devObjPtr->numofIPclProfileId)
            {
                GT_U32 regVal;
                /* get profileId1, profileId2 */
                regAddr = SMEM_SIP6_PCL_EXACT_MATCH_PROFILE_ID_MAP_REG(devObjPtr, index);

                smemRegGet(devObjPtr, regAddr, &regVal);

               /* 4 bits for profileId1 (bits 0-3 )
                  4 bits for profileId2 (bits 4-7 )
               */
                profileId1 = SMEM_U32_GET_FIELD(regVal,0, 4);
                profileId2 = SMEM_U32_GET_FIELD(regVal,4, 4);
            }
            break;

        case SIP5_TCAM_CLIENT_EPCL_E:
            /* sip6    : keyIndex: 0-127 */
            /* sip6_10 : keyIndex: 0-255 */
            if(index < devObjPtr->numofEPclProfileId)
            {
                /* get profileId1, profileId2 */
                regAddr = SMEM_SIP6_EPCL_EXACT_MATCH_PROFILE_ID_MAP_TBL_MEM(devObjPtr, index);
                regPtr = smemMemGet(devObjPtr, regAddr);

               /* 4 bits for profileId1 (bits 0-3 )
                  4 bits for profileId2 (bits 4-7 )
               */
                profileId1 = snetFieldValueGet(regPtr, 0, 4);
                profileId2 = snetFieldValueGet(regPtr, 4, 4);
            }
            break;
        default:
            __LOG(("invalid client\n"));
            return;
        }
    }

    __LOG_PARAM(profileId1);
    __LOG_PARAM(profileId2);

    if(client == SIP5_TCAM_CLIENT_TTI_E && SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && descrPtr)
    {
        GT_U32  port_profileId1,port_profileId2;
        GT_U32  index_profileId1,index_profileId2;

        port_profileId1 = SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
            SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_LOOKUP_PROFILE_ID1);
        port_profileId2 = SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
            SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_LOOKUP_PROFILE_ID2);

        __LOG_PARAM(port_profileId1);
        __LOG_PARAM(port_profileId2);

        switch(SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
            SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_PROFILE_ID_MODE))
        {
            case 1:  /*Per Port Assignment*/
                profileId1 = port_profileId1;
                profileId2 = port_profileId2;
                __LOG(("EM TTI : profiles per Port Assignment :profileId1[%d],profileId2[%d] \n",
                    profileId1,profileId2));
                break;
            case 2: /*  Per <Port, Packet type> Assignment */
                /* Bits [6:3]= Packet Type */
                /* Bits [2:0]= Source Physical Port table<Exact Match Lookup Profile-ID1>[2:0] */
                index_profileId1 = ((index & 0xF) << 3) | (port_profileId1 & 0x7);
                index_profileId2 = ((index & 0xF) << 3) | (port_profileId2 & 0x7);
                __LOG_PARAM(index_profileId1);
                __LOG_PARAM(index_profileId2);

                smemRegGet(devObjPtr,
                    SMEM_SIP6_10_TTI_EM_PROFILE_ID_1_MAPPING_TBL_MEM(devObjPtr,index_profileId1),
                    &profileId1);
                smemRegGet(devObjPtr,
                    SMEM_SIP6_10_TTI_EM_PROFILE_ID_2_MAPPING_TBL_MEM(devObjPtr,index_profileId2),
                    &profileId2);

                __LOG(("EM TTI : profiles per <Port, Packet type> Assignment :profileId1[%d],profileId2[%d] \n",
                    profileId1,profileId2));
                break;
            default:/*0/3 no change*/
                __LOG(("EM TTI : profiles per packet type :profileId1[%d],profileId2[%d] \n",
                    profileId1,profileId2));
        }
    }

    exactMatchProfileIdArr[0] = profileId1;
    exactMatchProfileIdArr[1] = profileId2;

    __LOG(("exactMatchProfileId client  = %d \n", client));
    __LOG(("exactMatchProfileId table index = %d \n", index));

    if (exactMatchProfileIdArr[0] == 0)
    {
        __LOG(("exactMatchProfileId1 set to 0 , lookup0 is disabled for client [%d]\n", client));
    }
    else
    {
        __LOG(("exactMatchProfileId1 = %d \n", exactMatchProfileIdArr[0]));
    }

    if (exactMatchProfileIdArr[1] == 0)
    {
        __LOG(("exactMatchProfileId2 set to 0 , lookup1 is disabled for client [%d]\n", client));
    }
    else
    {
        __LOG(("exactMatchProfileId2 = %d \n", exactMatchProfileIdArr[1]));
    }


    return;
}



/**
* @internal snetFalconExactMatchmProfileTableDefaultActionEntryGet function
* @endinternal
*
* @brief   return the Profile Table default action according to the ProfileID
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] exactMatchProfileId      - index to the exact match
*                                       profiles table
* @param[out]defaultActionPtr         - pointer to Exact Match
*                                       Default action
*
*/
static GT_U32 snetFalconExactMatchmProfileTableDefaultActionEntryGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                      exactMatchProfileId,
    OUT GT_U32                      *defaultActionPtr
)
{
    GT_U32  regAddr=0,wordNum=0;

    /* 256 bits of data for each default entry */
    for (wordNum = 0;wordNum < SIP6_EXACT_MATCH_FULL_ACTION_SIZE_WORD_CNS ;wordNum++)
    {
        regAddr = SMEM_FALCON_EXACT_MATCH_PROFILE_TABLE_DEFAULT_ACTION_DATA_REG(devObjPtr, exactMatchProfileId, wordNum);
        smemRegFldGet(devObjPtr, regAddr, 0, 32, &defaultActionPtr[wordNum]);
    }

    return GT_OK;
}

/**
* @internal snetFalconExactMatchProfileTableControlGet function
* @endinternal
*
* @brief   return Profile Table Control fields according to the ProfileID
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] exactMatchProfileId      - index to the exact match
*                                       profiles table
* @param[out]keySizePtr               - pointer to key size
* @param[out]keyStartPtr              - pointer to start of EM
*                                       Search key in the TCAM key
* @param[out]enableDefaultPtr         - pointer to enable
*                                       default action use,
*                                       in case no match in EM
*                                       lookup or in TCAM lookup
*/
GT_U32 snetFalconExactMatchProfileTableControlGet
(
    IN  SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN  GT_U32                            exactMatchProfileId,
    OUT SIP6_EXACT_MATCH_KEY_SIZE_ENT    *keySizePtr,
    OUT GT_U32                           *keyStartPtr,
    OUT GT_U32                           *enableDefaultPtr
)
{
    GT_U32  regAddr=0;
    GT_U32  value=0;

    regAddr = SMEM_FALCON_EXACT_MATCH_PROFILE_TABLE_CONTROL_REG(devObjPtr, exactMatchProfileId);
    smemRegFldGet(devObjPtr, regAddr, 0, 2, &value);
    smemRegFldGet(devObjPtr, regAddr, 4, 6, keyStartPtr);
    smemRegFldGet(devObjPtr, regAddr, 17, 1, enableDefaultPtr);

    switch(value)
    {
    case 0:
        *keySizePtr=SIP6_EXACT_MATCH_KEY_SIZE_5B_E;
        break;
    case 1:
        *keySizePtr=SIP6_EXACT_MATCH_KEY_SIZE_19B_E;
        break;
    case 2:
        *keySizePtr=SIP6_EXACT_MATCH_KEY_SIZE_33B_E;
        break;
    case 3:
        *keySizePtr=SIP6_EXACT_MATCH_KEY_SIZE_47B_E;
        break;
    default:
        break;
    }

    return GT_OK;
}

/**
* @internal sip6ExactMatchProfileTableMaskDataGet function
* @endinternal
*
* @brief   return Profile Table Mask Data according to the ProfileID
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] exactMatchProfileId      - index to the exact match
*                                       profiles table
* @param[out]maskDataPtr              - pointer to Exact
*                                       Match Profile table mask
*
*/
static GT_U32 sip6ExactMatchProfileTableMaskDataGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                       exactMatchProfileId,
    OUT GT_U32                      *maskDataPtr
)
{
    GT_U32  regAddr = 0,wordNum = 0;

    /* 384 bits of data for each entry mask */
    for (wordNum = 0;wordNum < SIP6_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS ;wordNum++)
    {
        regAddr = SMEM_FALCON_EXACT_MATCH_PROFILE_TABLE_MASK_DATA_REG(devObjPtr, exactMatchProfileId, wordNum);
        smemRegFldGet(devObjPtr, regAddr, 0, 32, &maskDataPtr[wordNum]);
    }

    return GT_OK;
}


/**
* @internal sip6ExactMatchBuildNewKey function
* @endinternal
*
* @brief   return the new Exact match Key build from the original
*          TCAM-Key, according to the ProfileID
*
* @param[in] devObjPtr                   - (pointer to) the device object
* @param[in] exactMatchProfileId         - index to the exact
*                                          match profiles table
* @param[in] keyDataPtr                  - TCAM-Key data
* @param[in] keySize                     - TCAM-Key size
* @param[in] exactMatchNewKeyDataPtr     - EM-Key data
* @param[in] exactMatchNewKeySizePtr     - EM-Key size
*
*/
static GT_U32 sip6ExactMatchBuildNewKey
(
    IN  SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN  GT_U32                            exactMatchProfileId,
    IN  GT_U32                           *tcamKeyDataPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT            tcamKeySize,
    OUT GT_U32                           *exactMatchNewKeyDataPtr,
    OUT SIP6_EXACT_MATCH_KEY_SIZE_ENT    *exactMatchNewKeySizePtr
)
{
    GT_U32  exactMatchProfileTableEnableDefault;
    GT_U32  exactMatchProfileTableKeyStart;
    GT_U32  exactMatchProfileTableMaskDataArray[SIP6_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS];/* 376 bits for the mask */
    GT_U32  ii,offset= 0,exactMatchKeysize=0;

    DECLARE_FUNC_NAME(sip6ExactMatchBuildNewKey);
    __LOG(("sip6ExactMatchBuildNewKey :  makes new exact match key from tcam key \n"));

    snetFalconExactMatchProfileTableControlGet(devObjPtr,
                                               exactMatchProfileId,
                                               exactMatchNewKeySizePtr,
                                               &exactMatchProfileTableKeyStart,
                                               &exactMatchProfileTableEnableDefault);

    /*getting the mask*/
    sip6ExactMatchProfileTableMaskDataGet(devObjPtr,exactMatchProfileId,&exactMatchProfileTableMaskDataArray[0]);

    __LOG(("sip6ExactMatchBuildNewKey :  exactMatchProfileId [%d] exactMatchNewKeySizePtr[%d] exactMatchProfileTableKeyStart[%d] \n",
           exactMatchProfileId,*exactMatchNewKeySizePtr,exactMatchProfileTableKeyStart));

    exactMatchKeysize = SMEM_FALCON_EXACT_MATCH_KEY_SIZE_TO_BYTE_MAC(*exactMatchNewKeySizePtr);
    exactMatchProfileTableKeyStart*=8;

    /*shift and mask the Tcam key*/
    for (ii=0;ii<=(exactMatchKeysize/4);ii++)
    {
        /* calculate the bit offset were to start the copy of the needed bits */
        offset=(exactMatchProfileTableKeyStart+(ii*32));

        /* if the current offset plus 32 bits is less or equal to the total number
           of bits needed shifted to the correct location, it means that
           we can read 32 bits to the new exactMatchKeyData
           else we need to read only the leftovers (8,16,24 bits) */
        if ((offset+32) <= ((GT_U32)(exactMatchKeysize*8)+exactMatchProfileTableKeyStart))
        {
            exactMatchNewKeyDataPtr[ii] = snetFieldValueGet(tcamKeyDataPtr, offset, 32);
        }
        else
        {
            if ((offset+24) <= ((GT_U32)(exactMatchKeysize*8)+exactMatchProfileTableKeyStart))
            {
                exactMatchNewKeyDataPtr[ii] = snetFieldValueGet(tcamKeyDataPtr, offset, 24);
            }
            else
            {
                if ((offset+16) <= ((GT_U32)(exactMatchKeysize*8)+exactMatchProfileTableKeyStart))
                {
                    exactMatchNewKeyDataPtr[ii] = snetFieldValueGet(tcamKeyDataPtr, offset, 16);
                }
                else
                {
                    exactMatchNewKeyDataPtr[ii] = snetFieldValueGet(tcamKeyDataPtr, offset, 8);
                }
            }
        }
        /*masking the key*/
        exactMatchNewKeyDataPtr[ii] &=exactMatchProfileTableMaskDataArray[ii];
    }

    return GT_OK;
}

/**
* @internal snetFalconExactMatchParseEntry function
* @endinternal
*
* @brief   Parses Exact match entry to the struct
*
* @param[in] devObjPtr                    - (pointer to) the device object
* @param[in] exactMatchEntryPtr           - (pointer to) Exact match entry
* @param[in] entryIndex                   -  entry index (hashed index)
* @param[out] keySize                     - (pointer to) Exact match entry parsed into fields
*
*/
GT_VOID snetFalconExactMatchParseEntry
(
    IN    SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN    GT_U32                               *exactMatchEntryPtr,
    IN    GT_U32                                entryIndex,
    OUT   SNET_SIP6_EXACT_MATCH_ENTRY_INFO     *exactMatchEntryInfoPtr
)
{
    DECLARE_FUNC_NAME(snetFalconExactMatchParseEntry);

#define SIP6_GET_VALUE(structFieldName, tableFieldName)\
    exactMatchEntryInfoPtr->structFieldName = SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_GET(devObjPtr,exactMatchEntryPtr, entryIndex, tableFieldName)

    SIP6_GET_VALUE(valid                  , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_VALID                     );
    if(exactMatchEntryInfoPtr->valid == 0)
    {
        /* do not parse the rest of the entry ... not relevant ... save some time in Exact match scanning */
        return;
    }
    SIP6_GET_VALUE(entry_type             , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE            );
    __LOG(("snetFalconExactMatchParseEntry found valid entry type : %d \n", exactMatchEntryInfoPtr->entry_type));

    if(exactMatchEntryInfoPtr->entry_type==0)  /*entry is key and action*/
    {
        SIP6_GET_VALUE(key_size           , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_SIZE               );
        SIP6_GET_VALUE(age                , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_AGE                    );
        SIP6_GET_VALUE(lookup_number      , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_LOOKUP_NUMBER          );
        SIP6_GET_VALUE(key_31_0           , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_31_0               );
        SIP6_GET_VALUE(key_39_32          , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_39_32              );
        SIP6_GET_VALUE(action_31_0        , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_31_0            );
        SIP6_GET_VALUE(action_63_32       , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_63_32           );
        SIP6_GET_VALUE(action_67_64       , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_67_64           );
    }
    else /*entry is key only */
    {
        SIP6_GET_VALUE(keyOnly_31_0       , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_31_0           );
        SIP6_GET_VALUE(keyOnly_63_32      , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_63_32          );
        SIP6_GET_VALUE(keyOnly_95_64      , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_95_64          );
        SIP6_GET_VALUE(keyOnly_111_96     , SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_111_96         );
    }
}

/**
* @internal exactMatchKeyComparison function
* @endinternal
*
* @brief   return GT_TRUE incase the exact match key (part or all of it )
*          match the key in the exact match bank
*
* @param[in] devObjPtr                       - (pointer to) the device object
* @param[in] exactMatchNewKeyDataPtr         - (pointer to) EM-Key data
* @param[in] exactMatchNewKeySize            -  EM-Key size
* @param[in] exactMatchParsedEntryPtr        - (pointer to) Exact match Parsed Entry
* @param[in] hitNum                          -  exact matchhit number (0,1)
* @param[in] relativeBankOffset              -  the relative bank Offset from the first bank (0..3)
*
*/
static GT_BOOL exactMatchKeyComparison
(
    IN  SKERNEL_DEVICE_OBJECT                 *devObjPtr,
    IN  GT_U32                                *exactMatchNewKeyDataPtr,
    IN  GT_U32                                 exactMatchNewKeySize,
    IN  SNET_SIP6_EXACT_MATCH_ENTRY_INFO      *exactMatchParsedEntryPtr,
    IN  GT_U32                                 hitNum,
    IN  GT_U32                                 relativeBankOffset
)
{
    DECLARE_FUNC_NAME(exactMatchKeyComparison);
    GT_U32 originalKey[4]; /*exact matchkey (per bank ) is max 112 bit*/
    GT_U32 globleBitOffset=0;

    __LOG(("exactMatchKeyComparison : start comparison of Exact Match entry and Exact Match key \n"));

    if (!exactMatchParsedEntryPtr->valid) /* not valid */
    {
        if (relativeBankOffset == 0)
        {
            __LOG(("exactMatchKeyComparison : no hit , entry is not valid \n"));
        }
        else
            skernelFatalError("exactMatchKeyComparison : illegal state ,relativeBankOffset [%d] have valid bit 0 \n",
                              relativeBankOffset);
        return GT_FALSE;
    }

    if(relativeBankOffset == 0) /*we are in the first bank */
    {
        /*EM key and action can be only in relativeBankOffset 0 */
        if ( (exactMatchParsedEntryPtr->entry_type!=0)           ||
             (exactMatchParsedEntryPtr->lookup_number != hitNum) ||
             (exactMatchParsedEntryPtr->key_size != exactMatchNewKeySize) )
        {
            __LOG(("exactMatchKeyComparison : no hit One or more of these parameters does not match search data"
                   "entry_type [%d] d hitNum [%d] keySize [%d] \n",
                   exactMatchParsedEntryPtr->entry_type,exactMatchParsedEntryPtr->lookup_number,exactMatchParsedEntryPtr->key_size));
            return GT_FALSE;
        }
        originalKey[0] = snetFieldValueGet(exactMatchNewKeyDataPtr,0, 32);
        originalKey[1] = snetFieldValueGet(exactMatchNewKeyDataPtr,32, 8);

        if ((originalKey[0]== exactMatchParsedEntryPtr->key_31_0  ) &&
            (originalKey[1]== exactMatchParsedEntryPtr->key_39_32 )    )
        {
            __LOG(("exactMatchKeyComparison : match in relative bank [%d],hitNum [%d] \n",
                   relativeBankOffset,hitNum));
            return GT_TRUE;
        }
        __LOG(("exactMatchKeyComparison : no match in relative bank [%d] \n",
               relativeBankOffset));
    }
    else
    {
        /*EM key can be only in relativeBankOffset > 0 */
        if (exactMatchParsedEntryPtr->entry_type !=1 )
        {
            skernelFatalError("exactMatchKeyComparison : entry_type [%d] was not expected when relativeBankOffset [%d] \n",
                               exactMatchParsedEntryPtr->entry_type,relativeBankOffset);
            return GT_FALSE;
        }
        globleBitOffset= 40+((relativeBankOffset-1)* SIP6_EXACT_MATCH_KEY_ONLY_FIELD_SIZE_CNS ); /*40 for the first bank and 112 per bank */

        originalKey[0] = snetFieldValueGet(exactMatchNewKeyDataPtr, globleBitOffset+0  ,32);
        originalKey[1] = snetFieldValueGet(exactMatchNewKeyDataPtr, globleBitOffset+32 ,32);
        originalKey[2] = snetFieldValueGet(exactMatchNewKeyDataPtr, globleBitOffset+64 ,32);
        originalKey[3] = snetFieldValueGet(exactMatchNewKeyDataPtr, globleBitOffset+96 ,16);
        if ((originalKey[0] == exactMatchParsedEntryPtr->keyOnly_31_0  )&&
            (originalKey[1] == exactMatchParsedEntryPtr->keyOnly_63_32 )&&
            (originalKey[2] == exactMatchParsedEntryPtr->keyOnly_95_64 )&&
            (originalKey[3] == exactMatchParsedEntryPtr->keyOnly_111_96)  )
        {
            __LOG(("exactMatchKeyComparison : match in relative bank [%d],hitNum [%d] \n",
            relativeBankOffset,hitNum));
            return GT_TRUE;
        }
        __LOG(("exactMatchKeyComparison : no match in relative bank [%d] \n",
               relativeBankOffset));
    }
    return GT_FALSE;
}

/**
* @internal sip6ExactMatchIsHit function
* @endinternal
*
* @brief   return GT_TRUE incase there was a hit in the Exact match table
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] hitNum                     - hit number (0,1)
* @param[in] exactMatchNewKeyDataPtr    - EM-Key data
* @param[in] exactMatchNewKeySizePtr    - EM-Key size
* @param[out] matchIndexPtr             - pointer to match Index
*/
static GT_BOOL sip6ExactMatchIsHit
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN  GT_U32                                hitNum,
    IN  GT_U32                               *exactMatchNewKeyDataPtr,
    IN  SIP6_EXACT_MATCH_KEY_SIZE_ENT         exactMatchNewKeySize,
    OUT GT_U32                               *matchIndexPtr
)
{
    GT_U32 numOfValidBanks = 0;
    GT_U32 multiHashIndexArr[SIP6_EXACT_MATCH_MAX_NUM_BANKS_CNS] = {0};
    GT_U32 bank=0 ,relativeBank=0;
    GT_U32 regAddr , ageEn =0 ;
    GT_BOOL hit = GT_FALSE;
    GT_U32 * entryPtr , * firstBankEntryPtr=NULL ;                  /* Exact Match table entry pointer */
    SNET_SIP6_EXACT_MATCH_ENTRY_INFO exactMatchEntryInfo ;


    DECLARE_FUNC_NAME(sip6ExactMatchIsHit);
    __LOG(("start sip6ExactMatchIsHit  \n"));

    smacHashExactMatchMultiHashResultsCalc(devObjPtr,exactMatchNewKeyDataPtr,exactMatchNewKeySize,&multiHashIndexArr[0],&numOfValidBanks);

    regAddr = SMEM_FALCON_EXACT_MATCH_GLOBAL_CONFIGURATION_REG(devObjPtr,emGlobalConfiguration1);
    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        smemRegFldGet(devObjPtr, regAddr, 5, 1, &ageEn);
    else
        smemRegFldGet(devObjPtr, regAddr, 20, 1, &ageEn);

    for (bank = 0;bank < numOfValidBanks ;bank+=(exactMatchNewKeySize+1) )
    {
        for ( relativeBank = 0 ;relativeBank <= (GT_U32)exactMatchNewKeySize ;relativeBank ++ )
        {
            memset(&exactMatchEntryInfo, 0, sizeof(SNET_SIP6_EXACT_MATCH_ENTRY_INFO));
            /* Get entryPtr according to entry index */
            entryPtr = SMEM_SIP6_HIDDEN_EXACT_MATCH_PTR(devObjPtr, multiHashIndexArr[bank+relativeBank]);
            snetFalconExactMatchParseEntry(devObjPtr,entryPtr,multiHashIndexArr[bank+relativeBank],&exactMatchEntryInfo);

            if (relativeBank == 0)
            {
                firstBankEntryPtr = entryPtr ;
            }

            hit = exactMatchKeyComparison(devObjPtr,exactMatchNewKeyDataPtr,(GT_U32) exactMatchNewKeySize,
                                  &exactMatchEntryInfo, hitNum, relativeBank /*relative bank ofset from the "n" bank*/);

            if (!hit)
            {
                __LOG(("sip6ExactMatchIsHit :no hit bank index [0x%x] \n",
                       multiHashIndexArr[bank+relativeBank]));
                break;
            }

            if (relativeBank ==(GT_U32) exactMatchNewKeySize) /*hit is contuinuous GT_TRUE in all checked bank */
            {
                *matchIndexPtr=multiHashIndexArr[bank];
                if (ageEn)
                {
                    if(firstBankEntryPtr == NULL)
                    {
                        skernelFatalError("firstBankEntryPtr == NULL \n");
                        return GT_FALSE;
                    }

                    SMEM_SIP6_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, firstBankEntryPtr, multiHashIndexArr[bank],
                                 SMEM_SIP6_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_AGE,1);
                }
                __LOG(("sip6ExactMatchIsHit :hit bank index [0x%x] \n",
                       multiHashIndexArr[bank]));
                return GT_TRUE;
            }
        }/*end for ralativeBank loop*/
    }/*end for bank loop*/

    return GT_FALSE;
}

/**
* @internal sip6ExactMatchDoLookup function
* @endinternal
*
* @brief   sip6 function that does sequential lookup on all banks
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] tcamClient                 - Exact match client
* @param[in] keyArrayPtr                - key data array
* @param[in] keySize                    - size of the key
* @param[in] exactMatchProfileIdArr     - Exact Match lookup profile ID array
* @param[out] exactMatchClientMatchArr  - if client is configured as the client of the Exact match lookup
* @param[out] resultArr                 - Exact Match hits number of results
*/
static GT_U32 sip6ExactMatchDoLookup
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  SIP5_TCAM_CLIENT_ENT         tcamClient,
    IN  GT_U32                      *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT       keySize,
    IN  GT_U32                       exactMatchProfileIdArr[],
    OUT GT_BOOL                      exactMatchClientMatchArr[],
    OUT GT_U32                       resultArr[]
)
{
    DECLARE_FUNC_NAME(sip6ExactMatchDoLookup);

    GT_BOOL gotHit = GT_FALSE;
    GT_U32  hitNum;
    GT_U32  matchIndex=0;
    GT_U32  bankNum = 0; /* the bank of the matching index */
    GT_U32  numberOfBanks=0;
    GT_U32  autoLearnMatchIndex=0;/* the index to set into ipfix table, matchIndex+bankNum*/
    GT_U32  hitsCounter = 0;
    GT_U32  exactMatchNewKeyArray[SIP6_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS] = {0};/* the new key that will be used for Exact match table - 376 bits*/
    SIP6_EXACT_MATCH_KEY_SIZE_ENT  exactMatchNewKeySize;

    GT_U32  pipeId, localPipeId, regAddr;
    GT_U32  exactMatchLookupClient[SIP6_EXACT_MATCH_MAX_CLIENT_CNS];
    GT_BOOL checkFail;

    /* Auto Learn parameters for sip6_10*/
    GT_BOOL autoLearnEn = GT_FALSE;
    GT_BOOL gotFreeIndex=GT_FALSE;
    GT_BOOL gotFreeFlowId=GT_FALSE;
    GT_U32  freeFlowId=0;
    GT_BOOL newEntryLearned=GT_FALSE;

    pipeId = smemGetCurrentPipeId(devObjPtr);
    localPipeId = smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile(devObjPtr,pipeId,NULL);

    regAddr = SMEM_FALCON_EXACT_MATCH_PORT_MAPPING_REG(devObjPtr, localPipeId);
    smemRegFldGet(devObjPtr, regAddr, 0, 4, &exactMatchLookupClient[0]);
    smemRegFldGet(devObjPtr, regAddr, 4, 4, &exactMatchLookupClient[1]);

    for(hitNum = 0; hitNum < SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS; hitNum++)
    {
        checkFail=GT_FALSE;

        __LOG(("start Exact match lookup for hitNum [%d] \n", hitNum));

        /* need to reset the New Key calculated between each lookup */
        memset(exactMatchNewKeyArray, 0, sizeof(exactMatchNewKeyArray));

        if (exactMatchProfileIdArr[hitNum]==0)/* Do not trigger Exact match lookup */
        {
            /*init for the caller that no match for this lookup*/
            resultArr[hitNum] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
            __LOG(("for Exact match hitNum [%d]: NO hit, The TCAM/EM client do not trigger the TCAM lookup with 'EM Profile ID' != 0. \n", hitNum));

            continue;
        }

        /* in falcon the IPCL0 is not valid;(supports only IPCL1,2) */
        if(devObjPtr->isIpcl0NotValid)
        {
            if ((((tcamClient==SIP5_TCAM_CLIENT_TTI_E)&& exactMatchLookupClient[hitNum]!=0))||
                (((tcamClient==SIP5_TCAM_CLIENT_IPCL1_E)&& exactMatchLookupClient[hitNum]!=1))||
                (((tcamClient==SIP5_TCAM_CLIENT_IPCL2_E)&& exactMatchLookupClient[hitNum]!=2))||
                (((tcamClient==SIP5_TCAM_CLIENT_EPCL_E)&& exactMatchLookupClient[hitNum]!=3)))
                checkFail = GT_TRUE;
        }
        else
        {
            if ((((tcamClient==SIP5_TCAM_CLIENT_TTI_E)&& exactMatchLookupClient[hitNum]!=0))||
                (((tcamClient==SIP5_TCAM_CLIENT_IPCL0_E)&& exactMatchLookupClient[hitNum]!=1))||
                (((tcamClient==SIP5_TCAM_CLIENT_IPCL1_E)&& exactMatchLookupClient[hitNum]!=2))||
                (((tcamClient==SIP5_TCAM_CLIENT_IPCL2_E)&& exactMatchLookupClient[hitNum]!=3))||
                (((tcamClient==SIP5_TCAM_CLIENT_EPCL_E)&& exactMatchLookupClient[hitNum]!=4)))
                checkFail = GT_TRUE;
        }

        if (checkFail)
        {
                /*init for the caller that no match for this lookup*/
                resultArr[hitNum] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
                __LOG(("for Exact match hitNum [%d]: NO hit. The TCAM/EM client is not configured as the client of the Exact match lookup. \n", hitNum));
                exactMatchClientMatchArr[hitNum]= GT_FALSE ;
                continue;
        }
        exactMatchClientMatchArr[hitNum]= GT_TRUE ;
        sip6ExactMatchBuildNewKey(devObjPtr,exactMatchProfileIdArr[hitNum], keyArrayPtr, keySize, &exactMatchNewKeyArray[0], &exactMatchNewKeySize);
        __LOG(("[sip6ExactMatchBuildNewKey] exactMatchNewKeySize [%d] \n", exactMatchNewKeySize));
        __LOG(("[sip6ExactMatchBuildNewKey] exactMatchNewKeyArray_0 [0x%X],exactMatchNewKeyArray_1 [0x%X],exactMatchNewKeyArray_2 [0x%X] \n", exactMatchNewKeyArray[0],exactMatchNewKeyArray[1],exactMatchNewKeyArray[2]));
        __LOG(("[sip6ExactMatchBuildNewKey] exactMatchNewKeyArray_3 [0x%X],exactMatchNewKeyArray_4 [0x%X],exactMatchNewKeyArray_5 [0x%X] \n", exactMatchNewKeyArray[3],exactMatchNewKeyArray[4],exactMatchNewKeyArray[5]));
        __LOG(("[sip6ExactMatchBuildNewKey] exactMatchNewKeyArray_6 [0x%X],exactMatchNewKeyArray_7 [0x%X],exactMatchNewKeyArray_8 [0x%X] \n", exactMatchNewKeyArray[6],exactMatchNewKeyArray[7],exactMatchNewKeyArray[8]));
        __LOG(("[sip6ExactMatchBuildNewKey] exactMatchNewKeyArray_9 [0x%X],exactMatchNewKeyArray_10 [0x%X],exactMatchNewKeyArray_11 [0x%X] \n", exactMatchNewKeyArray[9],exactMatchNewKeyArray[10],exactMatchNewKeyArray[11]));

        /* use the new key to get a match in the Exact match entry */
        gotHit = sip6ExactMatchIsHit(devObjPtr,hitNum,&exactMatchNewKeyArray[0], exactMatchNewKeySize, &matchIndex);

        if(gotHit)
        {
            resultArr[hitNum] = matchIndex;
            hitsCounter++;
            __LOG(("for Exact match hitNum [%d]: got hit: matchIndex [%d] \n", hitNum,matchIndex));
        }
        else
        {
            /* for HAWK - if Auto Learn is enabled, a new entry will be learned and added to the Exact Match */
            if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
            {
                /* check if Auto Learn is enabled on hitNum */
                autoLearnEn = sip6_10_ExactMatchAutoLearnHitNumEnable(devObjPtr,hitNum);
                if (autoLearnEn==GT_FALSE)/* no auto learning enabled for this hit */
                {
                     /*init for the caller that no match for this lookup*/
                    resultArr[hitNum] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
                    __LOG(("for Exact match hitNum [%d]: NO hit - auto learn lookup not enabled  \n", hitNum));
                }
                else
                {
                    /* continue to check Auto Learn configuration */

                    /* check if Auto Learn is enabled on the exactMatchProfileIndex  */
                    autoLearnEn = sip6_10_ExactMatchAutoLearnProfileIndexEnable(devObjPtr,exactMatchProfileIdArr[hitNum]);
                    if (autoLearnEn==GT_FALSE)/* no auto learning enabled for this profile */
                    {
                         /*init for the caller that no match for this lookup*/
                        resultArr[hitNum] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
                        __LOG(("for Exact match hitNum [%d]: NO hit - auto learn not enabled \n", hitNum));
                    }
                    else
                    {
                        /* check if there is a free space for the learned new entry
                           use the key built above */

                        /* use the new key to find a free index match in the Exact match entry */
                        gotFreeIndex = sip6_10_ExactMatchFreeIndex(devObjPtr,hitNum,&exactMatchNewKeyArray[0],
                                                                   exactMatchNewKeySize, &matchIndex, &bankNum, &numberOfBanks);

                        if (gotFreeIndex==GT_FALSE)
                        {
                            /*init for the caller that no match for this lookup*/
                            resultArr[hitNum] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
                            __LOG(("for Exact match hitNum [%d]: NO hit - auto learn - no free index \n", hitNum));
                        }
                        else
                        {
                            /* find a free flowId */
                            gotFreeFlowId = sip6_10_ExactMatchFreeFlowId(devObjPtr,&freeFlowId);
                            if (gotFreeFlowId==GT_FALSE)
                            {
                                 /*init for the caller that no match for this lookup*/
                                resultArr[hitNum] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
                                __LOG(("for Exact match hitNum [%d]: NO hit - auto learn - no free flowId \n", hitNum));
                            }
                            else
                            {
                                /* build the entry learned, update the flowId in the reduced entry
                                   and add the new learned entry in the matchIndex */
                                newEntryLearned = sip6_10_ExactMatchBuildAutoLearnEntry(devObjPtr,hitNum,exactMatchProfileIdArr[hitNum],
                                                                                        &exactMatchNewKeyArray[0], exactMatchNewKeySize,
                                                                                        freeFlowId, matchIndex);

                                if (newEntryLearned==GT_FALSE)
                                {
                                     /*init for the caller that no match for this lookup*/
                                    resultArr[hitNum] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
                                    __LOG(("for Exact match hitNum [%d]: NO hit - auto learn entry "
                                           "was not added correctly \n", hitNum));
                                }
                                else
                                {
                                    /* remove from the match index the bank number that is located in the last bits*/
                                    switch (numberOfBanks)
                                    {
                                        case 4:
                                            autoLearnMatchIndex = (matchIndex  >> 0x2);/* bank number takes 2 bits */
                                            break;
                                        case 8:
                                            autoLearnMatchIndex = (matchIndex >> 0x3);/* bank number takes 3 bits */
                                            break;
                                        case 16:
                                            autoLearnMatchIndex = (matchIndex >> 0x4);/* bank number takes 4 bits */
                                            break;
                                        default:
                                            __LOG(("illegal numberOfBanks [%d] \n", numberOfBanks));
                                            autoLearnMatchIndex=matchIndex;
                                            break;
                                    }

                                    sip6_10_ExactMatchAutoLearnEntryIndexSet(devObjPtr, freeFlowId, autoLearnMatchIndex, bankNum);

                                    resultArr[hitNum] = matchIndex;
                                    hitsCounter++;
                                    __LOG(("for Exact match hitNum [%d]: got hit: matchIndex [%d] \n", hitNum,matchIndex));
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                /*init for the caller that no match for this lookup*/
                resultArr[hitNum] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
                __LOG(("for Exact match hitNum [%d]: NO hit \n", hitNum));
            }
        }
    }

    return hitsCounter;
}

/**
* @internal snetFalconExactMatchLookupByGroupId function
* @endinternal
*
* @brief  do lookup in Exact Match for given key and fill the results array
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] groupId                    - Exact match client group Id
* @param[in] tcamClient                 - Exact match client
* @param[in] keyArrayPtr                - key array (size up to 47 bytes)
* @param[in] keySize                    - size of the key number of hits found
* @param[in] exactMatchProfileIdArr     - Exact Match lookup profile ID Array
* @param[out] exactMatchClientMatchArr  - if client is configured as the client of the Exact match lookup
* @param[out] resultArr                 - Exact Match hits
*/
GT_U32 snetFalconExactMatchLookupByGroupId
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  GT_U32                    groupId,
    IN  SIP5_TCAM_CLIENT_ENT      tcamClient,
    IN  GT_U32                   *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize,
    IN  GT_U32                    exactMatchProfileIdArr[],
    OUT GT_BOOL                   exactMatchClientMatchArr[],
    OUT GT_U32                    resultArr[]
)
{
    DECLARE_FUNC_NAME(snetFalconExactMatchLookupByGroupId);

    GT_U32      numOfHits;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TCAM_E);

    if((exactMatchProfileIdArr[0]==0 )&&(exactMatchProfileIdArr[1]==0))
    {
        __LOG(("groupId [%d]: ProfileId1 and ProfileId2 are 0 \n",
            groupId));
        numOfHits = 0;
        resultArr[0] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
        resultArr[1] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
        goto exitCleanly_lbl;
    }

    /* sequential lookup on all segments */
    numOfHits = sip6ExactMatchDoLookup(devObjPtr, tcamClient, keyArrayPtr, keySize, exactMatchProfileIdArr,
                                       exactMatchClientMatchArr,resultArr);

   __LOG_PARAM(resultArr[0]);
   __LOG_PARAM(resultArr[1]);
   __LOG_PARAM(exactMatchClientMatchArr[0]);
   __LOG_PARAM(exactMatchClientMatchArr[1]);

exitCleanly_lbl:
    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E);

    return numOfHits;
}

/**
* @internal snetFalconExactMacthLookup function
* @endinternal
*
* @brief   do lookup in Exact match for given key and
*          fill the results array
*
* @param[in] devObjPtr                 - (pointer to) the device object
* @param[in] tcamClient                - Exact match client
* @param[in] keyArrayPtr               - key array (size up to 47
*                                        bytes)
* @param[in] keySize                   - size of the key
*                                        number of hits found
* @param[in] exactMatchProfileIdArr    - Exact Match lookup
*                                        profile ID array
* @param[out] exactMatchClientMatchArr - if client is configured as the client of the Exact match lookup
* @param[out] resultArr                - Exact Match hits
*
*/
GT_U32 snetFalconExactMacthLookup
(
    IN  SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN  SIP5_TCAM_CLIENT_ENT      tcamClient,
    IN  GT_U32                   *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT    keySize,
    IN  GT_U32                    exactMatchProfileIdArr[],
    OUT GT_BOOL                   exactMatchClientMatchArr[],
    OUT GT_U32                    resultArr[]
)
{
    DECLARE_FUNC_NAME(snetFalconExactMacthLookup);

    GT_STATUS   st; /* return status */
    GT_U32      groupId = 0;
    GT_U32      numOfHits;
    GT_CHAR*    clientNamePtr = ((tcamClient < SIP5_TCAM_CLIENT_LAST_E) ? tcamClientName[tcamClient] : "unknown");

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TCAM_E);

    __LOG(("Start Exact Match check for [%s] \n",
        clientNamePtr));

    /* get group id */
    st = snetLion3TcamGetTcamGroupId(devObjPtr, tcamClient, &groupId);
    if(GT_OK != st)
    {
        __LOG(("client [%s] disabled", clientNamePtr));
        numOfHits = 0;
        simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E);
    }
    else
    {
        numOfHits = snetFalconExactMatchLookupByGroupId(devObjPtr, groupId, tcamClient, keyArrayPtr,
                                                        keySize, exactMatchProfileIdArr,
                                                        exactMatchClientMatchArr,resultArr);

    }
    return numOfHits;
}

/**
* @internal sip6ExactMatchActionAssignmentGet function
* @endinternal
*
* @brief   return origin bit and data from the exact match table
*
* @param[in] devObjPtr                        - (pointer to) the device object
* @param[in] expandedActionIndex              - index to action assignment table row (0..15)
* @param[in] byte                             - the byte offset in the row  (0..30)
* @param[out]OriginPtr                        - pointer to origin bit : 0- take from dataptr
*                                                                       1- take value from exact matchaction
* @param[out]DataPtr                          - pointer to data Depending on origin value
*                                               origin==0 - default value
*                                               origin==1 - byte offset in the exact match action
*/
static GT_VOID sip6ExactMatchActionAssignmentGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                       expandedActionIndex,
    IN  GT_U32                       byte,
    OUT GT_U32                      *originPtr,
    OUT GT_U32                      *dataPtr
)
{
    GT_U32  regAddr=0;

    regAddr = SMEM_FALCON_EXACT_MATCH_ACTION_ASSIGNMENT_REG(devObjPtr,expandedActionIndex,byte);
    smemRegFldGet(devObjPtr, regAddr, 8, 1, originPtr);
    if (*originPtr==0)
    {
        smemRegFldGet(devObjPtr, regAddr, 0, 8, dataPtr);
    }
    else
    {
        smemRegFldGet(devObjPtr, regAddr, 0, 3, dataPtr);
    }
}

/**
* @internal snetFalconExactMatchActionExpander function
* @endinternal
*
* @brief   receives reduced action from the Exact match engine
*          and expands them fully-sized TCAM Action
*
* @param[in] devObjPtr                  - (pointer to) device object.
* @param[in] reducedActionPtr           - (pointer to) reduced Exact match action
* @param[in] expandedActionIndex        -  index to Exact match expander action row (0..15)
* @param[out] fullActionPtr             - (pointer to) full Action after expanding
*/
static GT_VOID snetFalconExactMatchActionExpander
(
    IN  SKERNEL_DEVICE_OBJECT              *devObjPtr,
    IN GT_U32                              *reducedActionPtr,
    IN GT_U32                               expandedActionIndex,
    OUT GT_U32                             *fullActionPtr
)
{

   GT_U32 byte=0 ,origin=0,dataOrOffset=0;
   GT_U32 writeOffset=0 ,byteFromeReducedAction=0;
   GT_U32 maxNumOfBytes=0;

   if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
   {
       maxNumOfBytes = 32;
   }
   else
   {
       maxNumOfBytes = 31;
   }
   for (byte=0; byte < maxNumOfBytes; byte++)
   {
       sip6ExactMatchActionAssignmentGet(devObjPtr, expandedActionIndex, byte, &origin, &dataOrOffset);
       if (origin==0)
       {
           /*write the value from dataOrOffset into fullActionPtr */
           snetFieldValueSet(fullActionPtr,writeOffset,8,dataOrOffset);
       }
       else
       {   /*write the value from reducedActionPtr with the offset dataOrOffset into fullActionPtr */
           byteFromeReducedAction = snetFieldValueGet(reducedActionPtr,dataOrOffset*8,8);
           snetFieldValueSet(fullActionPtr,writeOffset,8,byteFromeReducedAction);
       }
       writeOffset+=8;
   }
}

/**
* @internal snetFalconExactMatchActionGet function
* @endinternal
*
* @brief   Get the action entry from the Exact Match table.
*
* @param[in] devObjPtr                       - pointer to device object.
* @param[in] descrPtr                        - pointer to frame descriptor.
* @param[in] matchIndex                      - index to the action table .
* @param[in] Client                          - tcam client (TTI,PCL,EPCL)
* @param[in] exactMatchProfileIdTableIndex   - keyType in case of TTI or
*                                              index for Exact match Profile-ID mapping table
*                                              in case of PCL/EPCL
* @param[out]actionDataPtr                   - (pointer to) action data
*/
GT_VOID snetFalconExactMatchActionGet
(
    IN SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr  ,
    IN GT_U32                               matchIndex,
    IN SIP5_TCAM_CLIENT_ENT                 client,
    IN GT_U32                               exactMatchProfileIdTableIndex,
    OUT GT_U32                             *actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetFalconExactMatchActionGet);
    /*for the reduced action needed 64 bit */
    GT_U32  reducedAction[2]={0} ;
    GT_U32  exactMatchProfileIdArr[SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS]={0} ;
    GT_U32  expandedActionIndex ;
    GT_U32 *entryPtr ;                 /* MAC table entry pointer */
    GT_U32  profileId =0;
    SNET_SIP6_EXACT_MATCH_ENTRY_INFO exactMatchEntryInfo ;

    if ((matchIndex == SNET_SIP6_EXACT_MATCH_DEFAULT_INDEX_LOOKUP0_CNS) ||
        (matchIndex == SNET_SIP6_EXACT_MATCH_DEFAULT_INDEX_LOOKUP1_CNS)   )
    {
        snetFalconExactMatchProfileIdGet(devObjPtr,descrPtr,client,exactMatchProfileIdTableIndex,&exactMatchProfileIdArr[0]);
        profileId = (matchIndex == SNET_SIP6_EXACT_MATCH_DEFAULT_INDEX_LOOKUP0_CNS ) ? exactMatchProfileIdArr[0] : exactMatchProfileIdArr[1];

        if (profileId!=0)
        {
            __LOG(("snetFalconExactMatchActionGet : action taken from exact match default action entry [%d] \n" ,
                   profileId));
            snetFalconExactMatchmProfileTableDefaultActionEntryGet(devObjPtr, profileId, &actionDataPtr[0]);
        }
        else
        {
             __LOG(("profileId [%s] not legal - profile is disabled", profileId));
             simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E);
        }
    }
    else
    {
        __LOG(("snetFalconExactMatchActionGet : action taken from exact match reduced action , matchIndex entry [%d] \n" ,
               matchIndex));
        entryPtr = SMEM_SIP6_HIDDEN_EXACT_MATCH_PTR(devObjPtr, matchIndex);
        snetFalconExactMatchParseEntry(devObjPtr,entryPtr,matchIndex,&exactMatchEntryInfo);
        reducedAction[0]=exactMatchEntryInfo.action_31_0;
        reducedAction[1]=exactMatchEntryInfo.action_63_32;
        expandedActionIndex=exactMatchEntryInfo.action_67_64;

        snetFalconExactMatchActionExpander(devObjPtr,&reducedAction[0],expandedActionIndex,&actionDataPtr[0]);
    }
}

/**
* @internal snetFalconTtiGetIndexForKeyType function
* @endinternal
*
* @brief   Gets the index used in the HW for a specific key type.
*
* @param[in] keyType                   - the key type
* @param[out] indexPtr                 - (pointer to) the index of the key type
*
*/
GT_VOID snetFalconTtiGetIndexForKeyType
(
    IN  SKERNEL_CHT3_TTI_KEY_TYPE_ENT       keyType,
    OUT GT_U32                              *indexPtr
)
{   switch (keyType)
    {
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E:
        *indexPtr = 0;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_1_E:
        *indexPtr = 1;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_2_E:
        *indexPtr = 2;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_3_E:
        *indexPtr = 3;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_4_E:
        *indexPtr = 4;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_5_E:
        *indexPtr = 5;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_6_E:
        *indexPtr = 6;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_7_E:
        *indexPtr = 7;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_8_E:
        *indexPtr = 8;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_9_E:
        *indexPtr = 9;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_10_E:
        *indexPtr = 10;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_11_E:
        *indexPtr = 11;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_12_E:
        *indexPtr = 12;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_13_E:
        *indexPtr = 13;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_14_E:
        *indexPtr = 14;
        break;
    case SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_15_E:
        *indexPtr = 15;
        break;
    case SKERNEL_CHT3_TTI_KEY_IPV4_E:
        *indexPtr = 16;
        break;
    case SKERNEL_CHT3_TTI_KEY_MPLS_E:
        *indexPtr = 17;
        break;
    case SKERNEL_XCAT_TTI_KEY_MIM_E:
        *indexPtr = 18;
        break;
    case SKERNEL_CHT3_TTI_KEY_ETH_E:
        *indexPtr = 19;
        break;
    default:
        skernelFatalError("Wrong key type format given\n");
    }
}
