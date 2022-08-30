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
* @file mvHwsD2dPcsIf.h
*
* @brief D2D PCS interface API
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsD2dPcsIf_H
#define __mvHwsD2dPcsIf_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>

/* PCS calendar slice number, CPSS configures 5 groups            */
/* each of 16 slices for channels and one slice CPU port          */
/* one slice for 25G channel, two slices for 50G channel          */
/* JIRA 8688 truncates last slice group removing CPU port from it */
/* Real CPU port speed is 10G                                     */
/* HW default - calendar size is 85 slices - fixed by SW to 84    */
#define HWS_D2D_PCS_CAL_LEN_CNS 84

GT_STATUS mvHwsD2dPcsReset
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_BOOL enable
);

/**
* @internal mvHwsD2dPcsTestGen function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[in] configPtr                - Defines D2D PRBS.
*/
GT_STATUS mvHwsD2dPcsTestGen
(
    IN GT_U8                           devNum,
    IN GT_U32                          d2dNum,
    IN MV_HWS_D2D_PRBS_CONFIG_STC      *configPtr
);

/**
* @internal mvHwsD2dPcsLoopbackSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[in] lbType                   - loopback type
*/
GT_STATUS mvHwsD2dPcsLoopbackSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   d2dNum,
    IN MV_HWS_PORT_LB_TYPE      lbType
);

/**
* @internal mvHwsD2dPcsLoopbackGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[out] lbType                  - loopback type
*/
GT_STATUS mvHwsD2dPcsLoopbackGet
(
    IN GT_U8                    devNum,
    IN GT_U32                   d2dNum,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
);

/**
* @internal mvHwsD2dPcsErrorInjectionSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number 0..63
* @param[in] laneIdx                  - lane 0..3
* @param[in] errorNum                 - number of errors to inject
*/
GT_STATUS mvHwsD2dPcsErrorInjectionSet
(
    IN GT_U8        devNum,
    IN GT_U32       d2dNum,
    IN GT_U32       laneIdx,  /* lane 0..3 */
    IN GT_U32       errorNum    /* 1..0x7FF, 0x7FF enables continuously injection */
);

/**
* @internal mvHwsD2dPcsErrorCounterGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number
* @param[in] laneIdx                  - lane index
* @param[out] errorCntPtr             - (pointer to) number of errors counter
*/
GT_STATUS mvHwsD2dPcsErrorCounterGet
(
    IN GT_U8            devNum,
    IN GT_U32           d2dNum,
    IN GT_U32           laneIdx,  /* lane 0..3 */
    OUT GT_U32          *errorCntPtr
);

/**
* @internal mvHwsD2dPcsFecCounterGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number
* @param[in] laneIdx                  - lane index
* @param[out] correctedErrorCntPtr    - (pointer to) Count number 160b words corrected by FEC
* @param[out] uncorrectedErrorCntPtr  - (pointer to) Count number 160b words with Uncorrectable error
*/
GT_STATUS mvHwsD2dPcsFecCounterGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  d2dNum,
    IN  GT_U32                  laneIdx,    /* lane 0..3 */
    OUT GT_U32                  *correctedErrorCntPtr,
    OUT GT_U32                  *uncorrectedErrorCntPtr
);

/**
* @internal mvHwsD2dPcsFecModeSet function
* @endinternal
 *
* @param[in] driverPtr                - driver to write with smi
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - D2D number
* @param[in] direction                - direction
* @param[in] fecMode                  - FEC mode
*/
GT_STATUS mvHwsD2dPcsFecModeSet
(
    IN CPSS_HW_DRIVER_STC               *driverPtr,
    IN  GT_U8                           devNum,
    IN  GT_U32                          d2dNum,
    IN  MV_HWS_D2D_DIRECTION_MODE_ENT   direction,
    IN  MV_HWS_PORT_FEC_MODE            fecMode
);

/**
* @internal mvHwsD2dPcsCalendarLengthSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - portGroup
* @param[in] pcsNum                   - port number
* @param[in] direction                - direction
* @param[in] length                   - calendar length
*/
GT_STATUS mvHwsD2dPcsCalendarLengthSet
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          pcsNum,
    IN MV_HWS_D2D_DIRECTION_MODE_ENT   direction,
    IN GT_U32                          length
);

/**
* @internal mvHwsD2dPcsCalendarLengthGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - portGroup
* @param[in] pcsNum                   - port number
* @param[in] direction                - direction
* @param[out] length                  - (pointer to) calendar length
*/
GT_STATUS mvHwsD2dPcsCalendarLengthGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroup,
    IN  GT_U32                          pcsNum,
    IN  MV_HWS_D2D_DIRECTION_MODE_ENT   direction,
    OUT GT_U32                          *lengthPtr
);


/**
* @internal mvHwsD2dPcsCalendarSlotConfigSet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - portGroup
* @param[in] pcsNum                   - port number
* @param[in] direction                - direction
* @param[in] slotNum                  - slot index
*                                           (APPLICABLE RANGES: 0..127)
* @param[in] chNum                    - channel number
*                                           (APPLICABLE RANGES: 0..63)
*/
GT_STATUS mvHwsD2dPcsCalendarSlotConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroup,
    IN  GT_U32                          pcsNum,
    IN  MV_HWS_D2D_DIRECTION_MODE_ENT   direction,
    IN  GT_U32                          slotNum,
    IN  GT_U32                          chNum
);

/**
* @internal mvHwsD2dPcsCalendarSlotConfigGet function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] portGroup                - portGroup
* @param[in] pcsNum                   - port number
* @param[in] direction                - direction
* @param[in] slotNum                  - slot index
*                                           (APPLICABLE RANGES: 0..127)
* @param[out] chNumPtr                - (pointer to) channel number
*/
GT_STATUS mvHwsD2dPcsCalendarSlotConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroup,
    IN  GT_U32                          pcsNum,
    IN  MV_HWS_D2D_DIRECTION_MODE_ENT   direction,
    IN  GT_U32                          slotNum,
    OUT GT_U32                          *chNumPtr
);

/**
* @internal mvHwsD2dPcsIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - pointer to func array
*/
GT_STATUS mvHwsD2dPcsIfInit
(
    IN GT_U8 devNum,
    IN MV_HWS_PCS_FUNC_PTRS **funcPtrArray
);


/**
* @internal hwsD2dPcsIfClose function
* @endinternal
 *
* @param[in] devNum                   - system device number
*/
void hwsD2dPcsIfClose
(
    IN GT_U8      devNum
);


/**
* @internal mvHwsD2dPcsInitPart1 function
* @endinternal
*   Part 1 of Raven-D2D-PCS init (before the D2D-GW(phy) init)
* @param[in] driverPtr                - driver to write with smi
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
*/
GT_STATUS mvHwsD2dPcsInitPart1
(
    IN CPSS_HW_DRIVER_STC  *driverPtr,
    IN GT_U8               devNum,
    IN GT_U32              d2dNum

);
/**
* @internal mvHwsD2dPcsInitPart2 function
* @endinternal
*   Part 1 of Raven-D2D-PCS init (before the D2D-GW(phy) init)
* @param[in] driverPtr                - driver to write with smi
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
*/
GT_STATUS mvHwsD2dPcsInitPart2
(
    IN CPSS_HW_DRIVER_STC  *driverPtr,
    IN GT_U8               devNum,
    IN GT_U32              d2dNum

);

/**
* @internal mvHwsD2dPcsInit_Z2 function
* @endinternal
 *
* @param[in] driverPtr                - driver to write with smi
* @param[in] devNum                   - system device number
* @param[in] d2dNum                   - d2d number
*/
GT_STATUS mvHwsD2dPcsInit_Z2
(
    IN CPSS_HW_DRIVER_STC  *driverPtr,
    IN GT_U8               devNum,
    IN GT_U32              d2dNum

);


GT_STATUS hwsFalconD2dPcsResetEnable
(
    IN GT_U8   devNum,
    IN GT_U32  d2dNum,
    IN GT_BOOL enable
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsD2dPcsIf_H */

