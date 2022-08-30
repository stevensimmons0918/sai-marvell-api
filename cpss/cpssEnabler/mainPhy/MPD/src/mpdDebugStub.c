/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/
/**
 * @file mpdDebug.c
 *	@brief PHY debug functions.
 *
 */

#include    <mtdApiTypes.h>
#include    <mtdAPI.h>
#include    <mtdAPIInternal.h>
#include    <mtdApiRegs.h>
#include    <mtdDiagnostics.h>
#include    <mtdDiagnosticsRegDumpData.h>

#include    <mpdTypes.h>
#include    <mpdPrv.h>
#include    <mpdDebug.h>



extern char * prvMpdDebugConvert(
/*!     INPUTS:             */
    MPD_DEBUG_CONVERT_ENT   conv_type,
    UINT_32                 value
/*!     INPUTS / OUTPUTS:   */
/*!     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(conv_type);
    MPD_UNUSED_PARAM(value);
    return "";
}
/*$ END OF  prvMpdDebugConvert */


extern BOOLEAN prvMpdIsMtdPhyType(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(rel_ifIndex);
    return TRUE;
}

extern BOOLEAN prvMpdIs1GPhyType(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{ 
    MPD_UNUSED_PARAM(rel_ifIndex);
    return TRUE;
}


MPD_RESULT_ENT prvMpdMtdDebugRegDump(
    /*     INPUTS:             */
    UINT_32    rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(rel_ifIndex);
    return MPD_OK_E;
}


extern MPD_RESULT_ENT prvMpdDebugConvertSpeedToString(
    /*!     INPUTS:             */
    MPD_SPEED_ENT speed,
/*!     INPUTS / OUTPUTS:   */
/*!     OUTPUTS:            */
    char *speedString_PTR
)
{
    MPD_UNUSED_PARAM(speed);
    MPD_UNUSED_PARAM(speedString_PTR);
    return MPD_OK_E;
}


extern MPD_RESULT_ENT prvMpdDebugConvertMtdCapabilitiesToString(
    /*!     INPUTS:             */
    UINT_16 capabilities,
/*!     INPUTS / OUTPUTS:   */
/*!     OUTPUTS:            */
    char *capabilitiesString_PTR
)
{
    MPD_UNUSED_PARAM(capabilities);
    MPD_UNUSED_PARAM(capabilitiesString_PTR);
    return MPD_OK_E;
}


extern MPD_RESULT_ENT prvMpdDebugPrintPortDb(
    /*!     INPUTS:             */
    UINT_32    rel_ifIndex,
    BOOLEAN    fullDb
/*!     INPUTS / OUTPUTS:   */
/*!     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(rel_ifIndex);
    MPD_UNUSED_PARAM(fullDb);
    return MPD_OK_E;
}


extern MPD_RESULT_ENT prvMpdMdioDebugCl45WriteRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 device,
    UINT_16 address,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(rel_ifIndex);
    MPD_UNUSED_PARAM(device);
    MPD_UNUSED_PARAM(address);
    MPD_UNUSED_PARAM(value);
    return MPD_OK_E;
}


extern MPD_RESULT_ENT prvMpdMdioDebugCl45ReadRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 device,
    UINT_16 address
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(rel_ifIndex);
    MPD_UNUSED_PARAM(device);
    MPD_UNUSED_PARAM(address);
    return MPD_OK_E;
}


extern MPD_RESULT_ENT prvMpdMdioDebugCl22WriteRegister(
    /*     INPUTS:             */
    UINT_32 rel_ifIndex,
    UINT_16 page,
    UINT_16 address,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(rel_ifIndex);
    MPD_UNUSED_PARAM(page);
    MPD_UNUSED_PARAM(address);
    MPD_UNUSED_PARAM(value);
    return MPD_OK_E;
}


extern MPD_RESULT_ENT prvMpdMdioDebugCl22ReadRegister(
    /*     INPUTS:             */
    BOOLEAN readAll,
    UINT_32 rel_ifIndex,
    UINT_16 page,
    UINT_16 address
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(rel_ifIndex);
    MPD_UNUSED_PARAM(readAll);
    MPD_UNUSED_PARAM(page);
    MPD_UNUSED_PARAM(address);
    return MPD_OK_E;
}


MPD_RESULT_ENT prvMpdDebugPerformPhyOperation(
    /*!     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    MPD_OP_CODE_ENT                 op,
    MPD_OPERATIONS_PARAMS_UNT     * params_PTR,
    BOOLEAN                         before
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
    MPD_UNUSED_PARAM(portEntry_PTR);
    MPD_UNUSED_PARAM(op);
    MPD_UNUSED_PARAM(params_PTR);
    MPD_UNUSED_PARAM(before);
    return MPD_OK_E;
}

/*$ END OF  prvMpdDebugPerformPhyOperation */

