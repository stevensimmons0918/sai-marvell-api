/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssUnitGenDrv.h
*
* DESCRIPTION:
*       prvCpssUnitGenDrv.c.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * typedef: struct PRV_CPSS_DRV_FLD_ARR_DEF_STC
 *
 * Description: general driver field definiton structure
 *
 * Fields:
 *       isSupported           - whether the field is supported
 *       regAddrOffsInStruct   - offset of field's register address in some structure
 *       offs;                 - offset of field in register
 *       len;                  - len of field in register
 *       arrsize               - thereis array of reguisters of same type
 *       minValue              - min value of field
 *       maxValue              - max value of field
 *       namePtr               - field name (for debugging purpose)
 */

typedef struct PRV_CPSS_DRV_FLD_ARR_DEF_STC
{
    GT_BOOL  isSupported;
    GT_U32   regAddrOffsInStruct;
    GT_U32   offs;
    GT_U32   len;
    GT_U32   arrsize;    /* thereis array of reguisters of same type */
    GT_U32   minValue;
    GT_U32   maxValue;
    GT_CHAR *namePtr;
}PRV_CPSS_DRV_FLD_ARR_DEF_STC;


/*
 * typedef: struct PRV_CPSS_DXCH_ARR_DRV_STC
 *
 * Description: general driver structure, contains info about fields and qeuery
 *
 * Fields:
 *       fldDefListPtr         - pointer to table of all defined registers
 *       totalFldNum           - number of entries in the table
 *       query                 - qeuery related information
 */
typedef struct PRV_CPSS_DXCH_ARR_DRV_STC
{
    PRV_CPSS_DRV_FLD_ARR_DEF_STC   *fldDefListPtr;
    GT_U32                          totalFldNum;
}PRV_CPSS_DXCH_ARR_DRV_STC;


/*
 * typedef: struct PRV_CPSS_DRV_FLD_QEUERY_STC
 *
 * Description: general driver qeuery contain info from where main paraemeters shall be taken
 *              for consequent read/write operations
 *
 * Fields:
 *       devNum                - device number
 *       portGroupId           - port group id
 *       regStructBaseAddrPtr  - address of structure of field's register address
 */
typedef struct PRV_CPSS_DRV_FLD_ARR_QEUERY_STC
{
    PRV_CPSS_DXCH_ARR_DRV_STC * drvPtr;
    GT_U8   devNum;
    GT_U32  portGroupId;
    GT_U32 *regStructBaseAddrPtr;
}PRV_CPSS_DRV_FLD_ARR_QEUERY_STC;


/*
 * typedef: struct PRV_CPSS_DRV_FLD_ARR_DEF_STC
 *
 * Description: record in list of supported structure, is used for initialization of corresponding table in driver
 *
 * Fields:
 *       regAddrOffsInStruct   - offeset of field's register address in some structure
 *       offs;                 - offset of field in register
 *       len;                  - len of field in register
 *       minValue              - min value of field
 *       maxValue              - max value of field
 *       namePtr               - field name (for debugging purpose)
 */
typedef struct PRV_CPSS_DRV_FLD_ARR_INIT_STCT
{
    GT_U32   fldId;
    struct REG_ARR_DEF_STC
    {
        GT_U32   regAddrOffsInStruct;
        GT_U32   offs;
        GT_U32   len;
    }regDef;
    GT_U32   arrSize;
    struct ARR_FLD_BND_STC
    {
        GT_U32   minValue;
        GT_U32   maxValue;
    }fldBnd;
    GT_CHAR *namePtr;
}PRV_CPSS_DRV_FLD_ARR_INIT_STC;

/*
 * typedef: PRV_BAD_VAL
 *
 * Description: in list of suppored fields this value is used to indicate end of list
 */
 #define PRV_BAD_VAL (GT_U32)(~0)


/**
* @internal prvCpssArrDrvInit function
* @endinternal
*
* @brief   Init general field arr oriented driver
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] drvPtr                   - pointer to driver description
* @param[in] fldDefListPtr            - pointer to field declaration table
* @param[in] fldInitListPtr           - pointer to array of all supported list
* @param[in] totalFldNum              - total number of fields in driver (both spuurted and not) = table size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on hardware error
*/
GT_STATUS prvCpssArrDrvInit
(
    INOUT PRV_CPSS_DXCH_ARR_DRV_STC     *drvPtr,
    IN    PRV_CPSS_DRV_FLD_ARR_DEF_STC  *fldDefListPtr, /* memory where field definition shall be stored */
    IN    const PRV_CPSS_DRV_FLD_ARR_INIT_STC *fldInitListPtr,
    IN    GT_U32                     totalFldNum
);

/**
* @internal prvCpssArrDrvFldAttributeArrSize function
* @endinternal
*
* @brief   Get field attribute size
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] drvPtr                   - (pointer to) driver description
* @param[in] fldId                    - field id
*
* @param[out] sizePtr                  - (pointer to) attribute size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong fldId
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssArrDrvFldAttributeArrSize
(
     IN   PRV_CPSS_DXCH_ARR_DRV_STC   *drvPtr,
     IN   GT_U32                       fldId,
     OUT  GT_U32                      *sizePtr
);

/**
* @internal prvCpssArrDrvQeueryInit function
* @endinternal
*
* @brief   Init qeuery
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] queryPtr                 - pointer to query description
* @param[in] drvPtr                   - pointer to driver description
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] regStructBaseAddrPtr     - base address of structure from where addresses of appropriate registers will be taken
* @param[in,out] queryPtr                 - pointer to query description
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssArrDrvQeueryInit
(
    INOUT PRV_CPSS_DRV_FLD_ARR_QEUERY_STC *queryPtr,
    IN    PRV_CPSS_DXCH_ARR_DRV_STC       *drvPtr,
    IN    GT_U8                            devNum,
    IN    GT_U32                           portGroupId,
    IN    GT_U32                          *regStructBaseAddrPtr
);


/**
* @internal prvCpssArrDrvQeueryFldAddrGet function
* @endinternal
*
* @brief   Get reg address of field
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] queryPtr                 - pointer to query description
* @param[in] fldId                    - field id
* @param[in] fldIdx                   - index of field in array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not supoorted on specific implementation of driver
* @retval GT_OUT_OF_RANGE          - id is valid, field is suppprted, but value is put of range
*/
GT_STATUS prvCpssArrDrvQeueryFldAddrGet
(
    IN  PRV_CPSS_DRV_FLD_ARR_QEUERY_STC *queryPtr,
    IN  GT_U32                       fldId,
    IN  GT_U32                       fldIdx,
    OUT GT_U32                      *regAddrPtr
);


/**
* @internal prvCpssArrDrvQeueryFldAddrValidCheck function
* @endinternal
*
* @brief   check reg address of field
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] queryPtr                 - pointer to query description
* @param[in] fldId                    - field id
* @param[in] fldIdx                   - index of field in array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not supoorted on specific implementation of driver
* @retval GT_OUT_OF_RANGE          - id is valid, field is suppprted, but value is put of range
* @retval GT_NOT_INITIALIZED       - reg addr is not initialized
*/
GT_STATUS prvCpssArrDrvQeueryFldAddrValidCheck
(
    IN  PRV_CPSS_DRV_FLD_ARR_QEUERY_STC *queryPtr,
    IN  GT_U32                       fldId,
    IN  GT_U32                       fldIdx
);


/**
* @internal prvCpssArrDrvQeueryFldSet function
* @endinternal
*
* @brief   Set field
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] queryPtr                 - pointer to query description
* @param[in] fldId                    - field id
* @param[in] fldIdx                   - index of field in array
* @param[in] fldValue                 - field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not spuurted on specific implementation of driver
* @retval GT_OUT_OF_RANGE          - id is valid, field is suppprted, but value is put of range
*/
GT_STATUS prvCpssArrDrvQeueryFldSet
(
    IN  PRV_CPSS_DRV_FLD_ARR_QEUERY_STC *queryPtr,
    IN  GT_U32                       fldId,
    IN  GT_U32                       fldIdx,
    IN  GT_U32                       fldValue
);

/**
* @internal prvCpssArrDrvQeueryFldGet function
* @endinternal
*
* @brief   Get the field
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] queryPtr                 - pointer to query
* @param[in] fldId                    - field id
* @param[in] fldIdx                   - index of field in array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not spuurted on specific implementation of driver
*/
GT_STATUS prvCpssArrDrvQeueryFldGet
(
    IN  PRV_CPSS_DRV_FLD_ARR_QEUERY_STC *queryPtr,
    IN  GT_U32                       fldId,
    IN  GT_U32                       fldIdx,
    OUT GT_U32                      *fldValuePtr
);

/**
* @internal prvCpssArrDrvQeueryFldCheck function
* @endinternal
*
* @brief   Check field
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] queryPtr                 - pointer to query
* @param[in] fldId                    - field id
* @param[in] fldIdx                   - index of field in array
* @param[in] fldValue                 - field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on field id is out of scope of total field
* @retval GT_NOT_SUPPORTED         - on field is not spuurted on specific implementation of driver
*/
GT_STATUS prvCpssArrDrvQeueryFldCheck
(
    IN   PRV_CPSS_DRV_FLD_ARR_QEUERY_STC *queryPtr,
    IN   GT_U32                           fldId,
    IN   GT_U32                           fldIdx,
    OUT  GT_U32                           fldValue
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


