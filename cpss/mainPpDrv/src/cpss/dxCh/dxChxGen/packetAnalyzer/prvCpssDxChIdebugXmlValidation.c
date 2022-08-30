/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChIdebugXmlValidation.c
*
* @brief Logical validation of XML file used for iDebug
*
* @version   1
********************************************************************************
*/


#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChIdebug.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChPacketAnalyzer.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChIdebugAuto.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define CPSS_DXCH_MAX_INTERFACE_NUM_IN_BUS_CNS  10
#define PRV_IDEBUG_STRING_BUFFER_SIZE_CNS       1024
#define PRV_IDEBUG_XML_VERSION_LOOKUP_LINES_CNS 10


/**
* @internal prvCpssDxChIdebugXmlValidatorCompareNames function
* @endinternal
*
* @brief   For specific instance check that all interfaces has unique name.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2 .
*
* @param[in] internalNames                -                    (pointer to)array of internal names present on instance.
* @param[in] uniqueNames                -                    (pointer to)array of unique names  present on instance.
* @param[in] instanceName                -                  name of the instance that is being validated.
* @param[in] internalNameDuplicationsAnalyze   whether or not to check internal name uniqueness.
* @param[in] size                                                       number of interfaces connected to instance.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
static GT_STATUS prvCpssDxChIdebugXmlValidatorCompareNames
(
    IN const GT_CHAR * internalNames[],
    IN const GT_CHAR * uniqueNames[],
    IN const GT_CHAR * instanceName,
    IN GT_U32          *placeInMux,
    IN GT_BOOL         internalNameDuplicationsAnalyze,
    IN GT_U32          size
)
{
      GT_BOOL         internalNameDuplicationFound[CPSS_DXCH_MAX_INTERFACE_NUM_IN_BUS_CNS];
      GT_BOOL         uniquelNameDuplicationFound[CPSS_DXCH_MAX_INTERFACE_NUM_IN_BUS_CNS];
      GT_U32          i,j;
      GT_BOOL         firstDup;

      cpssOsMemSet(internalNameDuplicationFound,GT_FALSE,CPSS_DXCH_MAX_INTERFACE_NUM_IN_BUS_CNS*sizeof(internalNameDuplicationFound[0]));
      cpssOsMemSet(uniquelNameDuplicationFound,GT_FALSE,CPSS_DXCH_MAX_INTERFACE_NUM_IN_BUS_CNS*sizeof(uniquelNameDuplicationFound[0]));

      if(GT_TRUE==internalNameDuplicationsAnalyze)
      {

          for(i=0;i<size;i++)
          {
            firstDup = GT_FALSE;
            if(internalNameDuplicationFound[i]==GT_FALSE)
            {
                 for(j=i+1;j<size;j++)
                 {
                    if(0==cpssOsStrCmp(internalNames[i],internalNames[j]))
                    {
                        internalNameDuplicationFound[i]=GT_TRUE;
                        internalNameDuplicationFound[j]=GT_TRUE;
                        if(GT_FALSE==firstDup)
                        {
                            cpssOsPrintf("Internal name duplication found on interface %s\n",instanceName);
                            firstDup = GT_TRUE;
                         }
                        cpssOsPrintf("  Index %d and %d are the same\n",i,j);
                    }
                 }
            }
          }
       }

      for(i=0;i<size;i++)
      {
        firstDup = GT_FALSE;
        if(uniquelNameDuplicationFound[i]==GT_FALSE)
        {
             for(j=i+1;j<size;j++)
             {
                if((0==cpssOsStrCmp(uniqueNames[i],uniqueNames[j]))&&
                    placeInMux[i]==placeInMux[j])
                {
                    uniquelNameDuplicationFound[i]=GT_TRUE;
                    uniquelNameDuplicationFound[j]=GT_TRUE;
                    if(GT_FALSE==firstDup)
                    {
                        cpssOsPrintf("Unique name duplication found on interface %s\n",instanceName);
                        firstDup = GT_TRUE;
                    }
                    cpssOsPrintf("  Index %d and %d are the same.Name %s\n",i,j,uniqueNames[i]);
                    cpssOsPrintf("   place in mux [%d]=%d \n",i,placeInMux[i]);
                    cpssOsPrintf("   place in mux [%d]=%d \n",j,placeInMux[j]);

                }
             }
        }
      }

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugXmlValidator function
* @endinternal
*
* @brief   Check logic validity of iDEbug XML.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2 .
* @param[in] devNum                   - device number
* @param[in] internalNameDuplicationsAnalyze   whether or not to check internal name uniqueness.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
GT_STATUS prvCpssDxChIdebugXmlValidator
(
    IN GT_U8           devNum,
    IN GT_BOOL         internalNameDuplicationsAnalyze
)
{
    mxml_node_t *node, *instance, *head,*uniqueNameNode = NULL;
    const GT_CHAR  *attr,*currentBusName;
    const GT_CHAR * internalNames[CPSS_DXCH_MAX_INTERFACE_NUM_IN_BUS_CNS];
    const GT_CHAR * uniqueNames[CPSS_DXCH_MAX_INTERFACE_NUM_IN_BUS_CNS];
    GT_U32          placeInMux[CPSS_DXCH_MAX_INTERFACE_NUM_IN_BUS_CNS];
    GT_CHAR         instanceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32          internalNamesSize;
    GT_STATUS       rc;
    GT_U32          i,numOfInterfaces;
    GT_CHAR_PTR     *interfacesArrDbPtr = NULL;
    GT_U32          dummy;


    if(GT_FALSE==PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        cpssOsPrintf("Only SIP6 devices are supported\n");
        return GT_OK;
    }

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*1.check that there is no duplications on same instance*/

    cpssOsPrintf("Check that there is no duplications on same instance:\n");

    node = prvCpssMxmlFindElement(head, head, "Instances", NULL, NULL, MXML_DESCEND);
    if (!node)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    /* find first instance */
    instance = node = node->child;
    if (!node)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    while (instance != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(instance)

        internalNamesSize=0;


        currentBusName = prvCpssMxmlElementGetAttr(instance,"name");
        node = prvCpssMxmlFindElement(instance, instance, "Interfaces", NULL, NULL, MXML_DESCEND);
        if (!node)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        /* find first interface */
        node = node->child;
        if (!node)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

        while(node != NULL)
        {
            PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

            attr = prvCpssMxmlElementGetAttr(node,"internalName");
            if(cpssOsStrlen(attr) != 0)
            {
                internalNames[internalNamesSize]=attr;
            }

            attr = prvCpssMxmlElementGetAttr(node,"placeInMux");
            if(cpssOsStrlen(attr) != 0)
            {
                placeInMux[internalNamesSize]=cpssOsStrTo32(attr);
            }


            uniqueNameNode = prvCpssMxmlFindElement(node, node, "uniqueName", NULL, NULL, MXML_DESCEND);
            if (!node || uniqueNameNode == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }


             while(uniqueNameNode->type!=MXML_ELEMENT)
            {
                uniqueNameNode=uniqueNameNode->next;
            }

            uniqueNameNode=uniqueNameNode->child;

            while(uniqueNameNode->type!=MXML_ELEMENT)
            {
                uniqueNameNode=uniqueNameNode->next;
            }

            attr = uniqueNameNode->value.element.name; /* uniqueName */

            if(cpssOsStrlen(attr) != 0)
            {
                uniqueNames[internalNamesSize++]=attr;
            }

            node = node->next;
        }

        rc = prvCpssDxChIdebugXmlValidatorCompareNames(internalNames,uniqueNames,currentBusName,placeInMux,
            internalNameDuplicationsAnalyze,internalNamesSize);

        if (rc!=GT_OK)
        {
           return rc;
        }

        instance = instance->next;
    }

     /*2.check that all defined interfaces are connected*/

    cpssOsPrintf("Check that all defined interfaces are connected:\n");

     /* get pointer to interfaces array in iDebug DB */
      rc = prvCpssDxChIdebugDbInterfaceArrayGet(devNum,&numOfInterfaces,&interfacesArrDbPtr);
      if (rc != GT_OK)
      {
          return rc;
      }

      for (i=0; i<numOfInterfaces; i++)
      {
         node = mxmlInstanceInfoGet(devNum,interfacesArrDbPtr[i],0,instanceId,&dummy,
                &dummy,&dummy,&dummy);
        if(!node)
        {
            cpssOsPrintf("Interface %s is not connected \n",interfacesArrDbPtr[i]);
        }
      }

    return GT_OK;
}


/**
* @internal prvCpssDxChIdebugXmlVersionGet function
* @endinternal
*
* @brief  Get XML iDebug version
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2 .
* @param[in] devNum                   - device number
* @param[in] internalNameDuplicationsAnalyze   whether or not to check internal name uniqueness.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
GT_STATUS prvCpssDxChIdebugXmlVersionGet
(
    IN GT_U8           devNum
)
{
    CPSS_OS_FILE_TYPE_STC * filePtr;
    static char buffer[PRV_IDEBUG_STRING_BUFFER_SIZE_CNS];
    GT_U32 i=0;

    filePtr = prvCpssDxChIdebugFileFpGet(devNum);
    if(filePtr==NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    cpssOsRewind(filePtr->fd);

    while (cpssOsFgets(buffer, 1024, filePtr->fd) != 0)
    {
        if(cpssOsStrStr(buffer,"<!--Version"))
        {
            cpssOsPrintf("\n%s\n",buffer);
            break;
        }

        i++;

        if(i==PRV_IDEBUG_XML_VERSION_LOOKUP_LINES_CNS)
        {
            cpssOsPrintf("\nVersion not found.\n");
            break;
        }
    }

    return GT_OK;
}


