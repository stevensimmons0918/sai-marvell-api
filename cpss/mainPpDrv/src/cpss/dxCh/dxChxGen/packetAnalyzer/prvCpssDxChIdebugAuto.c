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
* @file prvCpssDxChIdebugAutoTypes.c
*
* @brief Automatic Packet Analyzer APIs for CPSS.
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChIdebug.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**************************** Constants definitions *****************************/

/**************************** Private macro definitions ***********************/

/****************************** Enum definitions ********************************/

#define SIP6_TBD_MARK

#define PRV_IDEBUG_TOKEN_VAL_LOGIC_INFO_START   "validityLogic"
#define PRV_IDEBUG_VAL_LOGIC_VALUE_EXIST        "="
#define PRV_IDEBUG_VAL_LOGIC_INFO_VAL_PREFIX    "![CDATA["
#define PRV_IDEBUG_ENUM_INFO_START              "Enums"
#define PRV_IDEBUG_ENUM_INFO_VAL                "Value"
#define PRV_IDEBUG_ENUM_INFO_NAME               "name"
#define PRV_IDEBUG_ENUM_ATTR_VAL                "value"
#define PRV_IDEBUG_ENUM_ATTR_AND_OP             "and"
#define PRV_IDEBUG_ENUM_ATTR_OR_OP              "or"
#define PRV_IDEBUG_ENUM_ATTR_AND_OP_WITH_WS     " and"
#define PRV_IDEBUG_ENUM_ATTR_OR_OP_WITH_WS      " or"


#define PRV_IDEBUG_MAX_LEVEL_CNS     6


/**
* @internal prvCpssDxChIdebugFieldDynamicValidityLogicUpdate function
* @endinternal
*
* @brief   update field validity logic dynamicly .
*  Read the required logic from XMLdb in runtime. Check if required values match received values
*
* @note   APPLICABLE DEVICES:      Aldrin; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X.
*
* @param[in] devNum                        device number
* @param[in] interfaceName            interface name
* @param[in] fieldName                - field name
* @param[in] numOfFields              - number of interface fields
* @param[in,out] fieldsValueArr       interface fields
*                                       values array
*
* @retval GT_OK                    - on success
*
*/
GT_STATUS prvCpssDxChIdebugFieldDynamicValidityLogicUpdate
(
    IN  GT_U8                                          devNum,
    IN  const GT_CHAR_PTR                              interfaceName,
    IN  const GT_CHAR_PTR                              fieldName,
    IN  GT_U32                                         numOfFields,
    INOUT PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC     fieldsValueArr[]
);


/********************************* Structures definitions ***********************/

/********************************* private APIs *********************************/

static GT_U32 getParam
(
    IN  GT_CHAR_PTR                                 fieldName,
    IN  GT_U32                                      numOfFields,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC    fieldsValueArr[]
)
{
    GT_U32 i;

    for(i=0; i<numOfFields; i++)
    {
        if (cpssOsStrCmp(fieldsValueArr[i].fieldName,fieldName) == 0)
        {
            return fieldsValueArr[i].data[0];
        }
    }

    return 0;
}

static GT_VOID setParam
(
    IN  GT_CHAR_PTR                                 fieldName,
    IN  GT_U32                                      numOfFields,
    INOUT  PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC fieldsValueArr[],
    IN  GT_BOOL                                     isValid
)
{
    GT_U32 i;

    for(i=0; i<numOfFields; i++)
    {
        if (cpssOsStrCmp(fieldsValueArr[i].fieldName,fieldName) == 0)
        {
            fieldsValueArr[i].isValid = isValid;
        }
    }

    return;
}

/**
    SIP6_TBD_MARK

    SIP6_TBD_MARK

* @internal prvCpssDxChIdebugFieldValidityLogicUpdate function
* @endinternal
*
* @brief   update field validity logic
*
* @note   APPLICABLE DEVICES:      Aldrin; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X.
*
* @param[in] interfaceName            - interface name
* @param[in] fieldName                - field name
* @param[in] numOfFields              - number of interface
*                                       fields
* @param[in,out] fieldsValueArr       - interface fields
*                                       values array
*
* @retval GT_OK                    - on success
*
*/
GT_STATUS prvCpssDxChIdebugFieldValidityLogicUpdate
(
    IN  GT_U8                                       devNum,
    IN  GT_CHAR_PTR                                 interfaceName,
    IN  GT_CHAR_PTR                                 fieldName,
    IN  GT_U32                                      numOfFields,
    INOUT PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC  fieldsValueArr[]
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_AC3X_E );

    /* Check if the interface applicable for the device */
    rc = prvCpssDxChIdebugInterfaceDeviceCheck(devNum, interfaceName);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Check if the field is applicable for the device */
    rc = prvCpssDxChIdebugFieldDeviceCheck(devNum, fieldName);
    if (rc != GT_OK)
    {
        return rc;
    }

    return  prvCpssDxChIdebugFieldDynamicValidityLogicUpdate(devNum,
        interfaceName,
        fieldName,
         numOfFields,fieldsValueArr);

}

/**
* @internal prvCpssDxChIdebugFieldValidityLogicEnumParse function
* @endinternal
*
* @brief   Convert form enum name to enum integer value
*
* @note   APPLICABLE DEVICES:      Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X;Aldrin; Bobcat3;
*
* @param[in] devNum                        device number
* @param[in] interfaceName            interface name
* @param[in] fieldName                - field name
* @param[in] fieldValueInStr              - field name in string format
* @param[out] fieldValuePtr              -(pointer to)field name in integer format
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                    - on failure
*
*/
GT_STATUS prvCpssDxChIdebugFieldValidityLogicEnumParse
(
    IN   GT_U8                                          devNum,
    IN   const GT_CHAR_PTR                              interfaceName,
    IN   const GT_CHAR_PTR                              fieldName,
    IN  const GT_CHAR_PTR                               fieldValueInStr,
    OUT  GT_U32                                         *fieldValuePtr
)
{
    const GT_CHAR  *attr;
    mxml_node_t    *node;
    node = mxmlFindInterfaceFirstFieldNode(devNum,interfaceName);

    if(node)
    {

        while (node != NULL)
        {
            PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

            attr = prvCpssMxmlElementGetAttr(node,"internalName");
            if(cpssOsStrCmp(attr,fieldName)==0)
            {
                  node = prvCpssMxmlFindElement(node, node, PRV_IDEBUG_ENUM_INFO_START, NULL, NULL, MXML_DESCEND);
                  if (!node)
                  {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
                  }
                  node = prvCpssMxmlFindElement(node, node, PRV_IDEBUG_ENUM_INFO_VAL,
                        PRV_IDEBUG_ENUM_INFO_NAME, fieldValueInStr, MXML_DESCEND);
                  if (!node)
                  {
                    /*This is a number not an enum*/
                    *fieldValuePtr = cpssOsStrToU32(fieldValueInStr, NULL, 0);
                  }
                  else

                  {
                    attr = prvCpssMxmlElementGetAttr(node,PRV_IDEBUG_ENUM_ATTR_VAL);
                    if(!attr)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
                    }
                    *fieldValuePtr = cpssOsStrToU32(attr, NULL, 0);
                  }
                  return GT_OK;
            }
            node=node->next;
         }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}
/**
* @internal prvCpssDxChIdebugFieldValidityFindSubExperession function
* @endinternal
*
* @brief   Find sub-expression in expression
*
* @note   APPLICABLE DEVICES:      Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X;Aldrin; Bobcat3;
*
* @param[in] expression                    expression to evaluate
* @param[out] startPtr               (pointer to)start index  of sub-expression
* @param[out] stopPtr              -(pointer to)stop index  of sub-expression
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                    - on failure
*
*/
static GT_STATUS prvCpssDxChIdebugFieldValidityFindSubExperession
(
    IN  GT_CHAR_PTR                                     expression,
    IN GT_U32                                           *startPtr,
    IN GT_U32                                           *stopPtr
)
{
        GT_U32 i,balance=0;
        GT_U32 length = cpssOsStrlen(expression);

        *startPtr=length;
        *stopPtr = length;

        for(i=0;i<length;i++)
        {
            if(expression[i]=='(')
            {
                if(balance==0)
                {
                    *startPtr = i;
                }
                balance++;
            }

            if(expression[i]==')')
            {
                balance--;
                if(balance==0)
                {
                    *stopPtr = i;
                    break;
                }
            }
        }

        if(*startPtr==length||*stopPtr==length||*startPtr>*stopPtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        /*elliminate' (' and ' )' */
        (*startPtr)++;
        (*stopPtr)--;

        return GT_OK;

}

/**
* @internal prvCpssDxChIdebugFieldValidityEvaluateExperession function
* @endinternal
*
* @brief   Evaluate given expression
*
* @note   APPLICABLE DEVICES:      Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X;Aldrin; Bobcat3;
*
* @param[in] devNum                        device number
* @param[in] interfaceName            interface name
* @param[in] expression                    expression to evaluate

* @param[in] numOfFields              - number of interface fields
* @param[in,out] fieldsValueArr       interface fields
*                                       values array
* @param[out] resultPtr              -(pointer to)expression result
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                    - on failure
*
*/
static GT_STATUS prvCpssDxChIdebugFieldValidityEvaluateExperession
(
    IN  GT_U8                                          devNum,
    IN  const GT_CHAR_PTR                              interfaceName,
    IN  GT_CHAR_PTR                                    expression,
    IN  GT_U32                                         numOfFields,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC       fieldsValueArr[],
    IN  GT_U32                                         nestedLevel,
    OUT GT_BOOL                                        *resultPtr
)
{
    GT_U32  i,cnt=0,start,stop;
    GT_CHAR         *logicFieldStop;
    GT_CHAR         validityLogicField[64];
    GT_U32          fieldValueReceived,fieldValueRequired;
    GT_CHAR         fieldValueStr[64]={0};
    GT_CHAR         buffer[320]={0};
    GT_STATUS       rc;
    GT_BOOL         reverseLogic = GT_FALSE,tmpResult,andOperator=GT_FALSE;
    GT_BOOL         finish = GT_FALSE,firstExpression = GT_TRUE,evaluationRequired=GT_TRUE;

    nestedLevel++;

    /*Stack overflow protection*/
    if(nestedLevel>PRV_IDEBUG_MAX_LEVEL_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Condition parsing  failure");
    }

    *resultPtr = GT_TRUE;

    for(i=0;i<cpssOsStrlen(expression);i++)
    {
        if(expression[i]=='(')
        {
            cnt++;
            /*we have comlicate expression, no need to continue*/
            if(cnt>2)
            {
                break;
            }
        }
    }

    if(cnt<2)
    {
        if(cnt==1)
        {
            if(expression[0]=='(')
            {
                expression++;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            /*in case after '(' we have space */
            while (expression[0]==' ') expression++ ;
        }

        logicFieldStop=cpssOsStrStr(expression,PRV_IDEBUG_VAL_LOGIC_VALUE_EXIST);
        if(!logicFieldStop)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        cpssOsStrNCpy(validityLogicField,expression,logicFieldStop-expression+1);
        validityLogicField[logicFieldStop-expression]='\0';

        if(validityLogicField[logicFieldStop-expression-1]=='!')
        {
            validityLogicField[logicFieldStop-expression-1]='\0';
            reverseLogic=GT_TRUE;
        }

        /*transform to  field name format*/
        for(i=0;i<cpssOsStrlen(validityLogicField);i++)
        {
            if(validityLogicField[i]==' ')
            {
                validityLogicField[i]='_';
            }
            else if(validityLogicField[i]>='A'&&validityLogicField[i]<='Z')

            {
                validityLogicField[i]+=('a'-'A');
            }
        }

         /*handle trailing */
         for(i=cpssOsStrlen(validityLogicField)-1;i>0;i--)
         {
            if(validityLogicField[i]=='_')
            {
                validityLogicField[i]='\0';
            }
            else

            {
                break;
            }
         }

          /*2.Get field value*/
          fieldValueReceived = getParam(validityLogicField,numOfFields,fieldsValueArr);
          while((*logicFieldStop ==' ')||(*logicFieldStop=='=')||(*logicFieldStop=='"'))
          {
            logicFieldStop++;
          }

          i=0;
          while(logicFieldStop[i]!='"')
          {
              fieldValueStr[i]=logicFieldStop[i];
              i++;
          }

          /*3.Translate required  to integer*/
          rc = prvCpssDxChIdebugFieldValidityLogicEnumParse(devNum,interfaceName,validityLogicField,
            fieldValueStr,&fieldValueRequired);

          if(rc!=GT_OK)
          {
            return rc;
          }

          if(fieldValueReceived==fieldValueRequired)
          {
            *resultPtr = (!reverseLogic);
          }
          else
          {
            *resultPtr = reverseLogic;
          }
    }
    else
    {
        do
        {
            evaluationRequired = GT_TRUE;
            if(GT_FALSE==firstExpression)
            {
                if(cpssOsStrNCmp(expression,PRV_IDEBUG_ENUM_ATTR_AND_OP,3)==0||
                    cpssOsStrNCmp(expression,PRV_IDEBUG_ENUM_ATTR_AND_OP_WITH_WS,4)==0)
                {
                    andOperator = GT_TRUE;
                    /*Short circuit - let's save time*/
                    if(GT_FALSE==*resultPtr)
                    {
                        evaluationRequired = GT_FALSE;
                        tmpResult = GT_FALSE;
                    }
                }
                else if(cpssOsStrNCmp(expression,PRV_IDEBUG_ENUM_ATTR_OR_OP,2)==0||
                    cpssOsStrNCmp(expression,PRV_IDEBUG_ENUM_ATTR_OR_OP_WITH_WS,3)==0)
                {
                    andOperator = GT_FALSE;
                     /*Short circuit - let's save time*/
                    if(GT_TRUE==*resultPtr)
                    {
                        evaluationRequired = GT_FALSE;
                        tmpResult = GT_TRUE;
                    }
                }
                else
                {
                    finish = GT_TRUE;
                    continue;
                }
            }

            rc = prvCpssDxChIdebugFieldValidityFindSubExperession(expression,&start,&stop);
            if(rc!=GT_OK)
            {
                return rc;
            }
            if(GT_TRUE==evaluationRequired)
            {
                cpssOsStrNCpy(buffer,expression+start,stop-start+1);
                buffer[stop-start+1]='\0';

                rc= prvCpssDxChIdebugFieldValidityEvaluateExperession(devNum,interfaceName,
                    buffer,numOfFields,fieldsValueArr,nestedLevel,&tmpResult);

                if(rc!=GT_OK)
                {
                    return rc;
                }
            }
            expression+=(stop+2);

            if(GT_FALSE==firstExpression)
            {
                if(andOperator==GT_TRUE)
                {
                    *resultPtr=(*resultPtr)&&tmpResult;
                }
                else
                {
                    *resultPtr=(*resultPtr)||tmpResult;
                }
            }
            else
            {
                *resultPtr= tmpResult;
                 firstExpression= GT_FALSE;
            }

         }
         while(GT_FALSE==finish);
    }

    return GT_OK;

}

/**
* @internal prvCpssDxChIdebugFieldDynamicValidityLogicUpdate function
* @endinternal
*
* @brief   update field validity logic dynamicly .
*  Read the required logic from XMLdb in runtime. Check if required values match received values
*
* @note   APPLICABLE DEVICES:      Aldrin; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X.
*
* @param[in] devNum                        device number
* @param[in] interfaceName            interface name
* @param[in] fieldName                - field name
* @param[in] numOfFields              - number of interface fields
* @param[in,out] fieldsValueArr       interface fields
*                                       values array
*
* @retval GT_OK                    - on success
*
*/
GT_STATUS prvCpssDxChIdebugFieldDynamicValidityLogicUpdate
(
    IN  GT_U8                                          devNum,
    IN  const GT_CHAR_PTR                              interfaceName,
    IN  const GT_CHAR_PTR                              fieldName,
    IN  GT_U32                                         numOfFields,
    INOUT PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC     fieldsValueArr[]
)
{
    mxml_node_t *node;
    const GT_CHAR  *attr;
    GT_U32          prefixLen = cpssOsStrlen(PRV_IDEBUG_VAL_LOGIC_INFO_VAL_PREFIX);
    GT_CHAR         *logicFieldStop;
    GT_STATUS       rc;
    GT_BOOL         expressionValue;
    mxml_node_t     *tmp;


    node = mxmlFindInterfaceFirstFieldNode(devNum,interfaceName);

    if(node)
    {
        while (node != NULL)
        {
            PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

            attr = prvCpssMxmlElementGetAttr(node,"internalName");
            if(cpssOsStrCmp(attr,fieldName)==0)
            {
                  node = prvCpssMxmlFindElement(node, node, PRV_IDEBUG_TOKEN_VAL_LOGIC_INFO_START, NULL, NULL, MXML_DESCEND);
                  if (!node)
                  {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
                  }
                  else
                  {
                    tmp = node->child;

                    while(tmp->type!=MXML_ELEMENT)
                    {
                        tmp=tmp->next;
                    }

                    attr = tmp->value.element.name;
                    attr += prefixLen;

                   logicFieldStop=cpssOsStrStr(attr,PRV_IDEBUG_VAL_LOGIC_VALUE_EXIST);

                    if(logicFieldStop !=NULL)
                    {
                         /*There is   validity logic*/
                        /*1.Get required field name*/
                        /*ignore white spaces*/
                        while(*attr==' ')attr++;

                        rc = prvCpssDxChIdebugFieldValidityEvaluateExperession
                            (devNum,interfaceName,(GT_CHAR  *)attr,numOfFields,fieldsValueArr,0,&expressionValue);
                        if(rc!=GT_OK)
                        {
                            return rc;
                        }
                       setParam(fieldName,numOfFields,fieldsValueArr,expressionValue);
                    }
                    else
                    {
                        setParam(fieldName,numOfFields,fieldsValueArr,GT_TRUE);
                    }
                    return GT_OK;
                  }
            }
            node = node->next;
        }
     }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

