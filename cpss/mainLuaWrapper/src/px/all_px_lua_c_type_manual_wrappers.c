/*************************************************************************
* all_px_lua_c_type_manual_wrappers.c
*
* DESCRIPTION:
*       A MANUAL lua type wrapper , for those that not generated 'automatically'
*       It implements support for the following types:
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
**************************************************************************/
/*#include <px/all_px_lua_c_type_wrappers.c>*/
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <mainLuaWrapper/wraplNetIf.h>
#include <cpss/common/diag/cpssCommonDiag.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

typedef struct{

    CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC           eventInfo;
    GT_U32                                          errorCounter;
    GT_U32                                          failedRow;
    GT_U32                                          failedSegment;
    GT_U32                                          failedSyndrome;

} CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_FULL_INFO_STC;
/***** declarations ********/
use_prv_struct(GT_IPV6ADDR);
use_prv_struct(GT_IPADDR);
use_prv_print_struct(GT_IPADDR);
use_prv_print_struct(GT_IPV6ADDR);
use_prv_struct(CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC);
use_prv_struct(CPSS_PX_PORT_SERDES_EYE_RESULT_STC);
use_prv_struct(MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA);
use_prv_struct(MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA);

#ifndef CHX_FAMILY
/*************************************************************************
* prv_lua_to_c_APP_DEMO_MICRO_INIT_STC
*
* Description:
*       Convert to "c" APP_DEMO_MICRO_INIT_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_APP_DEMO_MICRO_INIT_STC
(
    lua_State                               *L,
    APP_DEMO_MICRO_INIT_STC  *val
)
{
    F_NUMBER(val,   -1, deviceId, GT_U32);
    F_NUMBER(val,   -1, mngInterfaceType, GT_U32);
    F_NUMBER(val,   -1, coreClock, GT_U32);
    F_NUMBER(val,   -1, featuresBitmap, GT_U32);
}


/*************************************************************************
* prv_c_to_lua_APP_DEMO_MICRO_INIT_STC
*
* Description:
*       Convert APP_DEMO_MICRO_INIT_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_APP_DEMO_MICRO_INIT_STC(
    lua_State                               *L,
    APP_DEMO_MICRO_INIT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val,  t, deviceId, GT_U32);
    FO_NUMBER(val,  t, mngInterfaceType, GT_U32);
    FO_NUMBER(val,  t, coreClock, GT_U32);
    FO_NUMBER(val,  t, featuresBitmap, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(APP_DEMO_MICRO_INIT_STC);
#endif

#if !defined(CHX_FAMILY)
typedef enum { CPSS_NET_RX_CPU_CODE_ENT_E } CPSS_NET_RX_CPU_CODE_ENT;

add_mgm_enum(CPSS_NET_RX_CPU_CODE_ENT);
#endif /* !defined(CHX_FAMILY) */

void prv_lua_to_c_CPSS_PX_INGRESS_IP2ME_ENTRY_STC(
    lua_State *L,
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC *val
)
{
    F_BOOL(val, -1, valid);
    F_NUMBER(val, -1, prefixLength, GT_U32);
    F_BOOL(val, -1, isIpv6);
    /* done manually [[[ */
    if(val->isIpv6)
    {
        F_STRUCT_CUSTOM_N(&(val->ipAddr), -1, ipv6Addr, ipAddr, GT_IPV6ADDR)
    }
    else
    {
        F_STRUCT_CUSTOM_N(&(val->ipAddr), -1, ipv4Addr, ipAddr, GT_IPADDR)
    }
    /* ]]] done manually */
}

void prv_c_to_lua_CPSS_PX_INGRESS_IP2ME_ENTRY_STC(
    lua_State *L,
    CPSS_PX_INGRESS_IP2ME_ENTRY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, valid);
    FO_NUMBER(val, t, prefixLength, GT_U32);
    FO_BOOL(val, t, isIpv6);
    if(val->isIpv6)
    {
        FO_STRUCT_N(&(val->ipAddr), t, ipv6Addr, ipAddr, GT_IPV6ADDR);
    }
    else
    {
        FO_STRUCT_N(&(val->ipAddr), t, ipv4Addr, ipAddr, GT_IPADDR);
    }
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_IP2ME_ENTRY_STC);


void prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_FULL_INFO_STC(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_FULL_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, eventInfo, CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC);
    FO_NUMBER(val, t, errorCounter, GT_U32);
    FO_NUMBER(val, t, failedRow, GT_U32);
    FO_NUMBER(val, t, failedSegment, GT_U32);
    FO_NUMBER(val, t, failedSyndrome, GT_U32);
    lua_settop(L, t);
}
typedef struct{

    CPSS_PX_LOGICAL_TABLE_ENT                              table;
    GT_BOOL                                                errorCountEnable;
    CPSS_EVENT_MASK_SET_ENT                                eventMask;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT          errorType;
    GT_BOOL                                                injectEnable;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT         injectMode;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT protectionType;
}CPSS_PX_DIAG_DATA_INTEGRITY_CONFIG_STATUS_STC;

void prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_CONFIG_STATUS_STC(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_CONFIG_STATUS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, table, CPSS_PX_LOGICAL_TABLE_ENT);
    FO_BOOL(val, t, errorCountEnable);
    FO_ENUM(val, t, eventMask, CPSS_EVENT_MASK_SET_ENT);
    FO_ENUM(val, t, errorType, CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT);
    FO_BOOL(val, t, injectEnable);
    FO_ENUM(val, t, injectMode, CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT);
    FO_ENUM(val, t, protectionType,  CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT);
    lua_settop(L, t);
}

typedef struct{

    CPSS_PX_LOGICAL_TABLE_ENT                              table;
    GT_U32                                                 tableSize;
}CPSS_PX_DIAG_DATA_INTEGRITY_SHADOW_SIZE_STC;



void prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_SHADOW_SIZE_STC
(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_SHADOW_SIZE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, table, CPSS_PX_LOGICAL_TABLE_ENT);
    FO_NUMBER(val, t, tableSize, GT_U32);

    lua_settop(L, t);
}

use_prv_struct(PACKET_STC);
use_prv_print_struct(PACKET_STC);

/*******************************************/
/* PX_RX_DATA_STC                             */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_PX_RX_DATA_STC
*
* Description:
*       Convert to "c" PX_RX_DATA_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_PX_RX_DATA_STC(
    lua_State *L,
    PX_RX_DATA_STC *val
)
{
    F_NUMBER(val, -1, devNum, GT_U8);
    F_NUMBER(val, -1, portNum, GT_PHYSICAL_PORT_NUM);
    F_NUMBER(val, -1, originByteCount, GT_U32);
    F_NUMBER(val, -1, timestamp, GT_U32);
    /* special handling for packet: can be table, string, userdata */
    F_STRUCT_CUSTOM(val, -1, packet, PACKET_STC);
}

/*************************************************************************
* prv_c_to_lua_PX_RX_DATA_STC
*
* Description:
*       Convert PX_RX_DATA_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_PX_RX_DATA_STC(
    lua_State *L,
    PX_RX_DATA_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, devNum, GT_U8);
    FO_NUMBER(val, t, portNum, GT_PHYSICAL_PORT_NUM);
    FO_NUMBER(val, t, originByteCount, GT_U32);
    FO_NUMBER(val, t, timestamp, GT_U32);
    FO_STRUCT(val, t, packet, PACKET_STC);
    lua_settop(L, t);
}
add_mgm_type(PX_RX_DATA_STC);

void prv_lua_to_c_CPSS_PX_PORT_SERDES_EYE_RESULT_STC(
    lua_State *L,
    CPSS_PX_PORT_SERDES_EYE_RESULT_STC *val
)
{
    const char *array = lua_tostring(L, -1);
    unsigned int idx;

    lua_getfield(L, -1, "matrixPtr");
    for (idx=0; idx<lua_objlen(L,-1); idx++)
    {
        cpssOsStrNCpy(&val->matrixPtr[idx],&array[idx],sizeof(char));
    }
    lua_pop(L, 1);

    F_NUMBER(val, -1, x_points, GT_U32);
    F_NUMBER(val, -1, y_points, GT_U32);

    lua_getfield(L, -1, "vbtcPtr");
    for (idx=0; idx<lua_objlen(L,-1); idx++)
    {
        cpssOsStrNCpy(&val->vbtcPtr[idx],&array[idx],sizeof(char));
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "hbtcPtr");
    for (idx=0; idx<lua_objlen(L,-1); idx++)
    {
        cpssOsStrNCpy(&val->hbtcPtr[idx],&array[idx],sizeof(char));
    }
    lua_pop(L, 1);

    F_NUMBER(val, -1, height_mv, GT_U32);
    F_NUMBER(val, -1, width_mui, GT_U32);
    F_NUMBER(val, -1, globalSerdesNum, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_SERDES_EYE_RESULT_STC(
    lua_State *L,
    CPSS_PX_PORT_SERDES_EYE_RESULT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);

    if(val->matrixPtr)
    {
        FO_ARRAY_START(val, t, matrixPtr);
        {
            const char *array = &val->matrixPtr[0];
            lua_pushstring(L, array);
            lua_rawseti(L, -2, 0);
        }
        FO_ARRAY_END(val, t, matrixPtr);
    }
    FO_NUMBER(val, t, x_points, GT_U32);
    FO_NUMBER(val, t, y_points, GT_U32);

    if(val->vbtcPtr)
    {
        FO_ARRAY_START(val, t, vbtcPtr);
        {
            const char *array = &val->vbtcPtr[0];
            lua_pushstring(L, array);
            lua_rawseti(L, -2, 0);
        }
        FO_ARRAY_END(val, t, vbtcPtr);
    }

    if(val->hbtcPtr)
    {
        FO_ARRAY_START(val, t, hbtcPtr);
        {
            const char *array = &val->hbtcPtr[0];
            lua_pushstring(L, array);
            lua_rawseti(L, -2, 0);
        }
        FO_ARRAY_END(val, t, hbtcPtr);
    }
    FO_NUMBER(val, t, height_mv, GT_U32);
    FO_NUMBER(val, t, width_mui, GT_U32);
    FO_NUMBER(val, t, globalSerdesNum, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_SERDES_EYE_RESULT_STC);

#ifndef CHX_FAMILY
void prv_lua_to_c_MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA *val
)
{


    F_NUMBER(val, -1, dcGain,           GT_U16);
    F_NUMBER(val, -1, lowFrequency,     GT_U16);
    F_NUMBER(val, -1, highFrequency,    GT_U16);
    F_NUMBER(val, -1, bandWidth,        GT_U16);
    F_NUMBER(val, -1, squelch,          GT_U16);

    F_NUMBER(val, -1, gainshape1,       GT_U8);
    F_NUMBER(val, -1, gainshape2,       GT_U8);
    F_BOOL(val,   -1, shortChannelEn);
    F_NUMBER(val, -1, dfeGAIN,          GT_U8);
    F_NUMBER(val, -1, dfeGAIN2,         GT_U8);

}

void prv_c_to_lua_MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, dcGain,           GT_U16);
    FO_NUMBER(val, t, lowFrequency,     GT_U16);
    FO_NUMBER(val, t, highFrequency,    GT_U16);
    FO_NUMBER(val, t, bandWidth,        GT_U16);
    FO_NUMBER(val, t, squelch,          GT_U16);

    FO_NUMBER(val, t, gainshape1,       GT_U8);
    FO_NUMBER(val, t, gainshape2,       GT_U8);
    FO_BOOL(val,   t, shortChannelEn);
    FO_NUMBER(val, t, dfeGAIN,          GT_U8);
    FO_NUMBER(val, t, dfeGAIN2,         GT_U8);

    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA);

void prv_lua_to_c_MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA *val
)
{


    F_NUMBER(val, -1, dcGain,           GT_U16);
    F_NUMBER(val, -1, lowFrequency,     GT_U16);
    F_NUMBER(val, -1, highFrequency,    GT_U16);
    F_NUMBER(val, -1, bandWidth,        GT_U16);
    F_NUMBER(val, -1, squelch,          GT_U16);

    F_NUMBER(val, -1, gainshape1,       GT_U8);
    F_NUMBER(val, -1, gainshape2,       GT_U8);
    F_BOOL(val,   -1, shortChannelEn);
    F_NUMBER(val, -1, dfeGAIN,          GT_U8);
    F_NUMBER(val, -1, dfeGAIN2,         GT_U8);

}

void prv_c_to_lua_MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, dcGain,           GT_U16);
    FO_NUMBER(val, t, lowFrequency,     GT_U16);
    FO_NUMBER(val, t, highFrequency,    GT_U16);
    FO_NUMBER(val, t, bandWidth,        GT_U16);
    FO_NUMBER(val, t, squelch,          GT_U16);

    FO_NUMBER(val, t, gainshape1,       GT_U8);
    FO_NUMBER(val, t, gainshape2,       GT_U8);
    FO_BOOL(val,   t, shortChannelEn);
    FO_NUMBER(val, t, dfeGAIN,          GT_U8);
    FO_NUMBER(val, t, dfeGAIN2,         GT_U8);

    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA);
#endif

/* CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT */
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT, info_802_1br_E2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC)
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT, info_802_1br_U2E_MC, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC)
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT, info_dsa_ET2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC)
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT, info_dsa_EU2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC)
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT, info_edsa_E2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC)
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT, info_dsa_QCN, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC)
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT, info_evb_QCN, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC)
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT, info_pre_da_PTP, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC)
UNION_MEMBER_NUMBER(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT, notNeeded, GT_U32)


/* CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT */
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT, info_802_1br, CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC)
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT, info_dsa, CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC)
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT, info_evb, CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC)
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT, info_pre_da, CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC)
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC)

/* CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT */
UNION_MEMBER_STRUCT(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT, info_common, CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC)
UNION_MEMBER_NUMBER(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT, notNeeded, GT_U32)

