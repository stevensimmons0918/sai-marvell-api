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
* @file prvCpssDxChCfg.h
*
* @brief private DxCh initialization PPs functions.
*
* @version   3
********************************************************************************
*/
#ifndef __prvCpssDxChCfgh
#define __prvCpssDxChCfgh


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/generic/extMac/cpssExtMacDrv.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/* number of SIP6 PBR entries in lpm line */
#define PRV_CPSS_DXCH_CFG_SIP6_NUM_PBR_ENTRIES_IN_LPM_LINE_CNS 5

/**
* @internal prvCpssDxChCfgPort88e1690Bind function
* @endinternal
*
* @brief   Creation and Bind PHY-MAC for PHY '88E1690' to the DX device
*         on specific portNum.
*         NOTE:
*         The portNum should be the 'physical port num' of the DX port connection
*         to the PHY-MAC device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] infoPtr                  - (pointer to) The needed info about the remote device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCfgPort88e1690Bind
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   *infoPtr
);

/*******************************************************************************
* prvCpssDxChCfgPort88e1690ObjInit
*
* DESCRIPTION:
*       create 88e1690 object. (singleton)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* INPUTS:
*       void
*
* OUTPUTS:
*
* RETURNS:
*       pointer to the '88e1690' object.
*       NULL - on GT_OUT_OF_CPU_MEM error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
CPSS_MACDRV_OBJ_STC* prvCpssDxChCfgPort88e1690ObjInit(void);


/*******************************************************************************
* prvCpssDxChCfgPort88e1690RemotePhysicalPortInfoGet
*
* DESCRIPTION:
*       get info about 88e1690 that hold the DX remote physical port.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       remotePhysicalPortNum  - the remote physical port number
*
* OUTPUTS:
*       portIndexPtr - (pointer to) the port index in the array of remotePortInfoArr[]
* RETURNS:
*       pointer to the '88e1690' info.
*       NULL - if 'remotePhysicalPortNum' not found
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC*  prvCpssDxChCfgPort88e1690RemotePhysicalPortInfoGet(
    IN GT_U8    devNum,
    IN GT_U32   remotePhysicalPortNum,
    OUT GT_U32  *portIndexPtr
);

/*******************************************************************************
* prvCpssDxChCfgPort88e1690CascadePortToInfoGet
*
* DESCRIPTION:
*       get the 88e1690 info that is on the DX device on the cascade port.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum   - physical device number
*       physicalPort  - the physical port number (of the DX cascade port)
*
* OUTPUTS:
*       None.
* RETURNS:
*       pointer to the '88e1690' info.
*       NULL - if not found
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC*  prvCpssDxChCfgPort88e1690CascadePortToInfoGet(
    IN GT_U8    devNum,
    IN GT_U32 physicalPort
);

/**
* @internal prvCpssDxChCfgPort88e1690InitEvents function
* @endinternal
*
* @brief   Init interrupt and events for the 88e1690
*
* @param[in] devNum                   - The Pp device number at which the Gpp device is connected.
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device
* @param[in] remotePhyMacInfoPtr      - (pointer to) the specific 88e1690 device info
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChCfgPort88e1690InitEvents
(
    IN GT_U8           devNum,
    IN GT_U32   cascadePortNum,
    IN PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr
);

/**
* @internal prvCpssDxChCfg88e1690RemoteFcModeSet function
* @endinternal
*
* @brief   Set Flow Control or HOL system mode for Remote ports on the specified device.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device resides
* @param[in] modeFcHol                - CPSS_DXCH_PORT_FC_E  : set Flow Control mode
*                                      CPSS_DXCH_PORT_HOL_E : set HOL system mode
* @param[in] profileSet               - the associated Drop Profile Set (Relevant for CPSS_DXCH_PORT_FC_E mode).
* @param[in] tcBitmap                 - bit map of CN Aware TCs (Relevant for CPSS_DXCH_PORT_FC_E mode):
*                                      bit#n set if traffic class #n is CN aware, bit#n is clear otherwise.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChCfg88e1690RemoteFcModeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    IN  CPSS_DXCH_PORT_HOL_FC_ENT           modeFcHol,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                              tcBitmap
);

/**
* @internal prvCpssDxChCfg88e1690RemoteFcModeGet function
* @endinternal
*
* @brief  Get Flow Control or HOL system mode for Remote ports on the specified device.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device resides
* @param[in] profileSet               - the associated Drop Profile Set (Relevant for CPSS_DXCH_PORT_FC_E mode).
* @param[in] tcBitmap                 - bit map of CN Aware TCs (Relevant for CPSS_DXCH_PORT_FC_E mode):
*                                      bit#n set if traffic class #n is CN aware, bit#n is clear otherwise.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChCfg88e1690RemoteFcModeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                cascadePortNum,
    OUT CPSS_PORT_TX_DROP_PROFILE_SET_ENT  *profileSetPtr,
    OUT GT_U32                             *tcBitmapPtr,
    OUT CPSS_DXCH_PORT_HOL_FC_ENT          *modeFcHolPtr
);

/**
* @internal prvCpssDxChCfgPort88e1690CascadePfcParametersSet function
* @endinternal
*
* @brief   Set PFC parameters per remoting cascade port.
*         Function is relevant for AC3X systems after cpssDxChCfgRemoteFcModeSet(),
*         Function should be used for advanced customization configuration only.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device resides
* @param[in] xOffThreshold            - xOff threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] xOnThreshold             - xOn threshold in buffers (APPLICABLE RANGES: 0..120)
* @param[in] timer                    - PFC  (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] tcBitmap                 - list of TCs to include at PFC message.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note PFC is sent in uplink direction from remote port.
*
*/
GT_STATUS prvCpssDxChCfgPort88e1690CascadePfcParametersSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    cascadePortNum,
    IN  GT_U32                  xOffThreshold,
    IN  GT_U32                  xOnThreshold,
    IN  GT_U32                  timer,
    IN  GT_U32                  tcBitmap
);

/**
* @internal prvCpssDxChCfgPort88e1690CascadePfcParametersGet function
* @endinternal
*
* @brief   Get PFC parameters per remoting cascade port.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
* @param[in] cascadePortNum           - the DX cascade port on which the 88e1690 device resides
*
* @param[out] xOffThresholdPtr         - xOff threshold in buffers
* @param[out] xOnThresholdPtr          - xOn threshold in buffers
* @param[out] timerPtr                 - PFC timer
* @param[out] tcBitmapPtr              - list of TCs to include at PFC message.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDxChCfgPort88e1690CascadePfcParametersGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    cascadePortNum,
    OUT GT_U32                  *xOffThresholdPtr,
    OUT GT_U32                  *xOnThresholdPtr,
    OUT GT_U32                  *timerPtr,
    OUT GT_U32                  *tcBitmapPtr
);

/**
* @internal prvCpssDxChCfg88e1690RemotePortsMacCountersByPacketParse function
* @endinternal
*
* @brief   Parses packet containing Ethernet MAC counters.
*         The result is stored at shadow buffers, and it can be retrieved
*         by function cpssDxChPortMacCountersCaptureOnPortGet()
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] numOfBuff                - Num of used buffs in packetBuffsArrPtr.
* @param[in] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[in] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
* @param[in] rxParamsPtr              - (pointer to)information parameters of received packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The input parameters are obtained by function cpssDxChNetIfSdmaRxPacketGet()
*       The output parameter are the parsed port number and MAC counter values.
*
*/
GT_STATUS prvCpssDxChCfg88e1690RemotePortsMacCountersByPacketParse
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          numOfBuff,
    IN  GT_U8                          *packetBuffsArrPtr[],
    IN  GT_U32                          buffLenArr[],
    IN  CPSS_DXCH_NET_RX_PARAMS_STC    *rxParamsPtr
);

/**
* @internal prvCpssDxChCfgAreRemotePortsBound function
* @endinternal
*
* @brief   Check if remote ports were bound to device
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] remotePortBoundPtr       - GT_TRUE - was bound, GT_FALSE - wasn't bound
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChCfgAreRemotePortsBound
(
    IN  GT_U8                               devNum,
    OUT GT_BOOL                            *remotePortBoundPtr
);

/**
* @internal prvCpssDxChCfg88e1690ReDevNum function
* @endinternal
*
* @brief   Replace the ID of a device from old device number
*         to a new device number.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] oldDevNum                - old device num
* @param[in] newDevNum                - new device num
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on error
*
* @note This function is called under ISR lock.
*
*/
GT_STATUS prvCpssDxChCfg88e1690ReDevNum
(
    IN  GT_U8 oldDevNum,
    IN  GT_U8 newDevNum
);

/**
* @internal prvCpssDxChCfgSharedTablesType2BlockTableIdGet function
* @endinternal
*
* @brief   function gets Id of table using the given SMB Type2 block.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X, Bobcat3; Aldrin2.
*
* @param[in]
*            devNum           - the device number
*            smbBlockIndex    - index of SMB block
*            smbBlockPort     - port of SMB block - 0 or 1
*
* @param[out]
*            tableIdPtr       - (pointer to) Id of the table using the given SMB Type2 block.
*            clientOffsetPtr  - (pointer to) Id of the client's bank using the given SMB Type2 block.
*
* @retval GT_OK               - success.
* @retval GT_BAD_PARAM        - on wrong parameters.
* @retval GT_BAD_STATE        - port select values that cannot be converted.
* @retval GT_HW_ERROR         - on hardware error.
*
*/
GT_STATUS prvCpssDxChCfgSharedTablesType2BlockTableIdGet
(
    IN   GT_U8                           devNum,
    IN   GT_U32                          smbBlockIndex,
    IN   GT_U32                          smbBlockPort,
    OUT  CPSS_DXCH_TABLE_ENT             *tableIdPtr,
    OUT  GT_U32                          *clientOffsetPtr
);

/**
* @internal prvCpssDxChCfgSharedTablesArpSmbIndexGet function
* @endinternal
*
* @brief   function returns serial number of ARP SMB.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X, Bobcat3; Aldrin2.
*
* @param[in]
*            devNum           - the device number
*            smbNum           - number of ARP's SMB to calculate serial number
*
* @retval serial number of SMB between all ARP SMBs with number less than smbNum.
*         0xFFFFFFFF - in case of wrong input parameters
*
*/
GT_U32 prvCpssDxChCfgSharedTablesArpSmbIndexGet
(
    IN   GT_U8      devNum,
    IN   GT_U32     smbNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __prvCpssDxChCfgh */

