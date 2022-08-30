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
* @file prvCpssPhy.h
*
* @brief Definitions and enumerators for port Core Serial Management
* Interface facility.
*
* @version   18
********************************************************************************
*/
#ifndef __prvCpssPhyh
#define __prvCpssPhyh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/phy/cpssGenPhySmi.h>

/* use in accessing Twist-D-XG's SMI control Reg */
#define PRV_CPSS_PHY_XENPAK_DEV_ADDRESS_CNS   0x05 /* DTE device address */
#define PRV_CPSS_PHY_XENPAK_PORT_ADDRESS_CNS  0x01 /* DTE port address   */

/* default values to indicate no SMI configuration */
#define PRV_CPSS_PHY_SMI_NO_PORT_ADDR_CNS   0xFFFF
#define PRV_CPSS_PHY_SMI_NO_CTRL_ADDR_CNS   0xFFFFFFFF
#define PRV_CPSS_PHY_INVALID_SMI_INSTANCE_CNS  0xFFFF

/* macro to get the port's PHY SMI address -- not relevant to XG ports */
#define PRV_CPSS_PHY_SMI_PORT_ADDR_MAC(devNum,portNum)  \
    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].smiIfInfo.smiPortAddr

/* macro to get the port's PHY SMI port group -- not relevant to XG ports */
#define PRV_CPSS_PHY_SMI_GROUP_PORT_MAC(devNum,portNum)  \
    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].smiIfInfo.portGroupHostingSmiInterface

#define PRV_CPSS_PHY_SMI_INSTANCE_MAC(devNum,portNum)  \
    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].smiIfInfo.smiInterface


/* macro to get the port's PHY XSMI interface */
#define PRV_CPSS_PHY_XSMI_PORT_INTERFACE_MAC(devNum,portNum)  \
    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].xsmiInterface

/**
* @struct PRV_CPSS_PHY_PORT_SMI_IF_INFO_STC
 *
 * @brief Holds information about the Smi interface to be used for a given
 * port.
*/
typedef struct{

    /** relation of port to SMI */
    GT_U16 smiInterface;

    /** SMI port address. */
    GT_U16 smiPortAddr;


    /** the port group that hosts the SMI to access the port (PHY) */
    GT_U32 portGroupHostingSmiInterface;

} PRV_CPSS_PHY_PORT_SMI_IF_INFO_STC;

/*******************************************************************************
* PRV_CPSS_SMI_PHY_MNG_REGISTERS_ADDR_GET_FUN
*
* DESCRIPTION:
*       Get addresse of control SMI register.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Puma2; Puma3; ExMx; Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - The PP to read from.
*       xsmiInterface - XSMI instance
*
* OUTPUTS:
*       ctrlRegAddrPtr - (ptr to) address of SMI control register
*
* RETURNS:
*       GT_OK   - on success
*       GT_BAD_PARAM - bad devNum
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_SMI_PHY_MNG_REGISTERS_ADDR_GET_FUN)
(
    IN GT_U8    devNum,
    IN  CPSS_PHY_SMI_INTERFACE_ENT smiInterface,
    OUT GT_U32  *ctrlRegAddrPtr
);
/**
* @struct PRV_CPSS_SMI_GEN_BIND_FUNC_STC
 *
 * @brief A structure to hold common SMI functions for PP Family needed
 * in CPSS
 * INFO "PER DEVICE FAMILY"
*/
typedef struct{

    /** get SMI management register address */
    PRV_CPSS_SMI_PHY_MNG_REGISTERS_ADDR_GET_FUN cpssPhySmiRegsAddrGet;
} PRV_CPSS_SMI_GEN_BIND_FUNC_STC;


/*******************************************************************************
* PRV_CPSS_XSMI_PHY_MNG_REGISTERS_ADDR_GET_FUN
*
* DESCRIPTION:
*       Get addresses of control and address XSMI registers.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx; Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum      - The PP to read from.
*       xsmiInterface - XSMI instance
*
* OUTPUTS:
*       ctrlRegAddrPtr - (ptr to) address of XSMI control register
*       addrRegAddrPtr - (ptr to) address of XSMI address register
*
* RETURNS:
*       GT_OK   - on success
*       GT_BAD_PARAM - bad devNum
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_XSMI_PHY_MNG_REGISTERS_ADDR_GET_FUN)
(
    IN GT_U8    devNum,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    OUT GT_U32  *ctrlRegAddrPtr,
    OUT GT_U32  *addrRegAddrPtr
);



/*
 * @enum, PRV_CPSS_SMI_BUS_TYPE_ENT
 * @brief, Bus types to be muxed between
 */
typedef enum {
    PRV_CPSS_SMI_BUS_TYPE_SMI_E,
    PRV_CPSS_SMI_BUS_TYPE_XSMI_E
} PRV_CPSS_SMI_BUS_TYPE_ENT;

/**
* @internal prvCpssSmiXsmiMppMuxSet function
* @endinternal
*
* @brief  Mux MPPs according to parameters
*         Currently implements only AC5/5x SMI/XSMI mux
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number
* @param[in] busNum                - bus number to be used
* @param[in] busType               - bus type to be used
*
* @param[out] resTypePtr           - (pointer to) resource type
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
*/
GT_STATUS prvCpssSmiXsmiMppMuxSet
(
    IN GT_U8    devNum,
    IN GT_U32   busNum,
    IN PRV_CPSS_SMI_BUS_TYPE_ENT busType
);


/**
* @struct PRV_CPSS_XSMI_GEN_BIND_FUNC_STC
 *
 * @brief A structure to hold common XSMI functions for PP Family needed
 * in CPSS
 * INFO "PER DEVICE FAMILY"
*/
typedef struct{

    /** get addresses of control and address XSMI registers; */
    PRV_CPSS_XSMI_PHY_MNG_REGISTERS_ADDR_GET_FUN cpssPhyXSmiRegsAddrGet;

} PRV_CPSS_XSMI_GEN_BIND_FUNC_STC;

/**
* @struct PRV_CPSS_PHY_INFO_STC
 *
 * @brief Holds info regarding virtual functions
 * port.
*/
typedef struct{

    /** structure of pointers to VCT service routines */
    CPSS_VCT_GEN_BIND_FUNC_STC genVctBindFunc;

    /** structure of pointers to SMI service routines */
    PRV_CPSS_SMI_GEN_BIND_FUNC_STC genSmiBindFunc;

    /** structure of pointers to XSMI service routines */
    PRV_CPSS_XSMI_GEN_BIND_FUNC_STC genXSmiBindFunc;

} PRV_CPSS_PHY_INFO_STC;

/**
* @internal prvCpssVctDbRelease function
* @endinternal
*
* @brief   private (internal) function to release the DB of the VCT.
*         NOTE: function 'free' the allocated memory.
*         and restore DB to 'pre-init' state
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssVctDbRelease
(
    void
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPhyh */


