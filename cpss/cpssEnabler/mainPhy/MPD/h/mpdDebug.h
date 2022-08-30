/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

#ifndef MPD_DEBUG_H_
#define MPD_DEBUG_H_
/**
 * @file mpdDebug.h
 *
 * @brief This file contains types and declarations
 *  required to debug MPD
 */

#include <mpdTypes.h>
#include <mpdApi.h>


extern char prvMpdDebugString[];

extern UINT_32 prvMpdDebugReadFlagId;
extern UINT_32 prvMpdDebugWriteFlagId;
extern UINT_32 prvMpdDebugErrorFlagId;
extern UINT_32 prvMpdDebugOperationFlagId;


/**
 * @enum    MPD_DEBUG_CONVERT_ENT
 *
 * @brief   Enumerator to convert different types to strings for debug.
 */
typedef enum{
    MPD_DEBUG_CONVERT_PHY_TYPE_E,
    MPD_DEBUG_CONVERT_OP_ID_E,
    MPD_DEBUG_CONVERT_MDI_MODE_E,
    MPD_DEBUG_CONVERT_CONBO_MODE_E,
    MPD_DEBUG_CONVERT_MEDIA_TYPE_E,
    MPD_DEBUG_CONVERT_GREEN_SET_E,
    MPD_DEBUG_CONVERT_PORT_SPEED_E,
    MPD_DEBUG_CONVERT_GREEN_READINESS_E,
    MPD_DEBUG_CONVERT_PHY_KIND_E,
    MPD_DEBUG_CONVERT_PHY_SPEED_E,
    MPD_DEBUG_CONVERT_PHY_EEE_SPEED_E,
    MPD_DEBUG_CONVERT_PHY_EEE_CAPABILITY_E,
    /* must be last */
    MPD_DEBUG_CONVERT_LAST_E
} MPD_DEBUG_CONVERT_ENT;


typedef struct {
    UINT_32       num_of_entries;
    char       ** text_ARR;
} PRV_MPD_DEBUG_CONVERT_STC;


extern char * prvMpdDebugConvert(
    /*!     INPUTS:             */
    MPD_DEBUG_CONVERT_ENT   conv_type,
    UINT_32                 value
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdDebugPerformPhyOperation(
    /*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    MPD_OP_CODE_ENT                 op,
    MPD_OPERATIONS_PARAMS_UNT     * params_PTR,
    BOOLEAN                         before
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);


extern MPD_RESULT_ENT prvMpdMtdDebugRegDump(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdDebugPrintPortDb(
    /*!     INPUTS:             */
    UINT_32    rel_ifIndex,
    BOOLEAN    fullDb
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
);


extern MPD_RESULT_ENT prvMpdMdioDebugCl22WriteRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 page,
    UINT_16 address,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdMdioDebugCl22ReadRegister(
    /*     INPUTS:             */
    BOOLEAN readAll,
    UINT_32 rel_ifIndex,
    UINT_16 page,
    UINT_16 address
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdMdioDebugCl45WriteRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 device,
    UINT_16 address,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdMdioDebugCl45ReadRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 device,
    UINT_16 address
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdDebugConvertSpeedToString(
    /*!     INPUTS:             */
    MPD_SPEED_ENT speed,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    char *speedString_PTR
);

extern MPD_RESULT_ENT prvMpdDebugConvertMtdCapabilitiesToString(
    /*!     INPUTS:             */
    UINT_16 capabilities,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    char *capabilitiesString_PTR
);


#endif /* MPD_DEBUG_H_ */

