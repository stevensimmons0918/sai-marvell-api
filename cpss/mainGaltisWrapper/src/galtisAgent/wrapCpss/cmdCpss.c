/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cmdBaseInit.c
*
* DESCRIPTION:
*       Init for Galtis Agent
*
* FILE REVISION NUMBER:
*       $Revision: 58 $
*
*******************************************************************************/


/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssSysConfig.h>
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>

#ifdef CHX_FAMILY
    #define HWS_CMD_INIT
    #define HWS_CMD_COMMON_INIT
    #define GENERIC_MAIN_PP_DRV_CMD_INIT
#endif /*CHX_FAMILY*/



#ifdef IMPL_CPSS_EXMX
GT_STATUS cmdLibInitCpssExMxBridge(GT_VOID);
GT_STATUS cmdLibInitCpssExMxConfig(GT_VOID);
GT_STATUS cmdLibInitCpssExMxCos(GT_VOID);
GT_STATUS cmdLibInitCpssExMxDiag(GT_VOID);
GT_STATUS cmdLibInitCpssExMxHwInit(GT_VOID);
GT_STATUS cmdLibInitCpssExMxI2c(GT_VOID);
GT_STATUS cmdLibInitCpssExMxInlif(GT_VOID);
GT_STATUS cmdLibInitCpssExMxMirror(GT_VOID);
GT_STATUS cmdLibInitCpssExMxStc(GT_VOID);
GT_STATUS cmdLibInitCpssExMxNetworkIf(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPcl(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPhy(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPolicy(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPolicer(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPort(GT_VOID);
GT_STATUS cmdLibInitCpssExMxTrunk(GT_VOID);
GT_STATUS cmdLibInitCpssExMxIp(GT_VOID);
GT_STATUS cmdLibInitCpssExMxIpFlow(GT_VOID);
GT_STATUS cmdLibInitCpssExMxVb(GT_VOID);
GT_STATUS cmdLibInitCpssExMxIpLpm(GT_VOID);
#endif /*IMPL_CPSS_EXMX*/


#ifdef IMPL_CPSS_EXMXTG
GT_STATUS cmdLibInitCpssExMxTgBridge(GT_VOID);
GT_STATUS cmdLibInitCpssExMxTgConfig(GT_VOID);
GT_STATUS cmdLibInitCpssExMxTgPcl(GT_VOID);
GT_STATUS cmdLibInitCpssExMxTgTrunk(GT_VOID);
GT_STATUS cmdLibInitCpssExMxInlifTg(GT_VOID);
GT_STATUS cmdLibInitCpssExMxTgIp(GT_VOID);
GT_STATUS cmdLibInitCpssExMxTgTunnelEntry(GT_VOID);
GT_STATUS cmdLibInitCpssExMxTgTunnelTerm(GT_VOID);
#endif /*IMPL_CPSS_EXMXTG*/


#ifdef CHX_FAMILY
GT_STATUS cmdLibInitCpssDxChBridge(GT_VOID);
GT_STATUS cmdLibInitCpssDxChBridgeCount(GT_VOID);
GT_STATUS cmdLibInitCpssDxChBridgeGen(GT_VOID);
GT_STATUS cmdLibInitCpssDxChBridgeVlan(GT_VOID);
GT_STATUS cmdLibInitCpssDxChBridgePrvEdgeVlan(GT_VOID);
GT_STATUS cmdLibInitCpssDxChBridgeFdb(GT_VOID);
GT_STATUS cmdLibInitCpssDxChBridgeFdbRouting(GT_VOID);
GT_STATUS cmdLibInitCpssDxChBridgeSecurityBreach(GT_VOID);
GT_STATUS cmdLibInitCpssDxChBridgeSrcId(GT_VOID);
GT_STATUS cmdLibInitCpssDxChBridgePe(GT_VOID);
GT_STATUS cmdLibInitCpssDxChCnc(GT_VOID);/***/
GT_STATUS cmdLibInitCpssDxChConfig(GT_VOID);
GT_STATUS cmdLibInitCpssDxChCos(GT_VOID);
GT_STATUS cmdLibInitCpssDxChCscd(GT_VOID);
GT_STATUS cmdLibInitCpssDxChDiag(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPcl(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPha(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPhy(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPolicer(GT_VOID);
GT_STATUS cmdLibInitCpssDxChNetIf(GT_VOID);
GT_STATUS cmdLibInitCpssDxChNst(GT_VOID);
GT_STATUS cmdLibInitCpssDxChNstPortIsolation(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPort(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPortCtrl(GT_VOID);
GT_STATUS cmdLibInitCpssDxChMirror(GT_VOID);
GT_STATUS cmdLibInitCpssDxChStc(GT_VOID);
GT_STATUS cmdLibInitCpssDxChTrunk(GT_VOID);
GT_STATUS cmdLibInitCpssDxChIpCtrl(GT_VOID);
GT_STATUS cmdLibInitCpssDxChIp(GT_VOID);
GT_STATUS cmdLibInitCpssDxChIpLpm(GT_VOID);
GT_STATUS cmdLibInitCpssDxChTcamManager(GT_VOID);
GT_STATUS cmdLibInitCpssDxChTunnel(GT_VOID);
GT_STATUS cmdLibInitCpssDxChHwInit(GT_VOID);
GT_STATUS cmdLibInitCpssDxChHwInitLedCtrl(GT_VOID);
GT_STATUS cmdLibInitCpssDxChTti(GT_VOID);
GT_STATUS cmdLibInitCpssDxChVnt(GT_VOID);
GT_STATUS cmdLibInitCpssDxChVersion(GT_VOID);
GT_STATUS cmdLibInitCpssDxChLogicalTargetMapping(GT_VOID);
GT_STATUS cmdLibInitCpssDxChCutThrough(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPtp(GT_VOID);
GT_STATUS cmdLibInitCpssDxChMultiPortGroup(GT_VOID);
GT_STATUS cmdLibInitCpssDxChFabricHGLink(GT_VOID);
GT_STATUS cmdLibInitCpssDxChHsu(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPortAp(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPortPip(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPortEee(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPortMapping(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPortInterlaken(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPortDiag(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPortSlicesDirectCtrl(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPortDynamicPizzaArbiter(GT_VOID);
GT_STATUS cmdLibInitCpssDxChDiagDataIntegrity(GT_VOID);
GT_STATUS cmdLibInitCpssDxChOam(GT_VOID);
GT_STATUS cmdLibInitCpssDxChOamSrv(GT_VOID);
GT_STATUS cmdLibInitCpssDxChL2Mll(GT_VOID);
GT_STATUS cmdLibInitCpssDxChTcam(GT_VOID);
GT_STATUS cmdLibInitCpssDxChExactMatch(GT_VOID);
GT_STATUS cmdLibInitCpssDxChProtection(GT_VOID);
GT_STATUS cmdLibInitCpssDxChLpm(GT_VOID);
GT_STATUS cmdLibInitCpssDxChVirtualTcam(GT_VOID);
GT_STATUS cmdLibInitCpssPhyMcd(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPortManager(GT_VOID);
GT_STATUS cmdLibInitCpssDxChPortLoopback(GT_VOID);
GT_STATUS cmdLibInitCpssDxChLatencyMonitoring(GT_VOID);
GT_STATUS cmdLibInitCpssDxCh3Policer(GT_VOID);
GT_STATUS cmdLibInitCpssDxChIpfix(GT_VOID);

#if defined (INCLUDE_TM)
GT_STATUS cmdLibInitCpssDxChTmGlueDrop(GT_VOID);
GT_STATUS cmdLibInitCpssDxChTmGlueAgingAndDelay(GT_VOID);
GT_STATUS cmdLibInitCpssDxChTmGlueQueueMap(GT_VOID);
GT_STATUS cmdLibInitCpssDxChTmGlueDram(GT_VOID);
GT_STATUS cmdLibInitCpssDxChTmGluePfc(GT_VOID);
GT_STATUS cmdLibInitCpssDxChTmGlueFlowControl(GT_VOID);
#endif /*#if defined (INCLUDE_TM)*/

#endif /*CHX_FAMILY*/

#ifdef IMPL_CPSS_DXCH3P
/*Ch3p*/
GT_STATUS cmdLibInitCpssDxCh3pBrgCapwap(GT_VOID);
GT_STATUS cmdLibInitCpssDxCh3pBrgMc(GT_VOID);
GT_STATUS cmdLibInitCpssDxCh3pBrgVlan(GT_VOID);
GT_STATUS cmdLibInitCpssDxCh3pCapwapGen(GT_VOID);
GT_STATUS cmdLibInitCpssDxCh3pIp(GT_VOID);
GT_STATUS cmdLibInitCpssDxCh3pTti(GT_VOID);
GT_STATUS cmdLibInitCpssDxCh3pTunnel(GT_VOID);



#endif /*IMPL_CPSS_DXCH3P*/

#ifdef IMPL_CPSS_EXMXPM

/* bridge */
GT_STATUS cmdLibInitCpssExMxPmBrgCount(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmBrgEgrFlt(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmBrgFdb(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmBrgFdbHash(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmBrgGen(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmBrgMc(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmBrgNestVlan(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmBrgPrvEdgeVlan(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmBrgSecurityBreach(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmBrgSrcId(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmBrgStp(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmBrgVlan(GT_VOID);
/* config */
GT_STATUS cmdLibInitCpssExMxPmCfg(GT_VOID);
/* CNC */
GT_STATUS cmdLibInitCpssExMxPmCnc(GT_VOID);
/* cpssHwInit */
GT_STATUS cmdLibInitCpssExMxPmLedCtrl(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmHwInit(GT_VOID);
/*dit*/
GT_STATUS cmdLibInitCpssExMxPmDit(GT_VOID);
/* inlif */
GT_STATUS cmdLibInitCpssExMxPmInlif(GT_VOID);
/* ip */
GT_STATUS cmdLibInitCpssExMxPmIpCtrl(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmIpStat(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmIpTables(GT_VOID);
/* ipLpmEngine */
GT_STATUS cmdLibInitCpssExMxPmIpLpm(GT_VOID);
/* hsu */
GT_STATUS cmdLibInitCpssExMxPmHsu(GT_VOID);
/*mpls*/
GT_STATUS cmdLibInitCpssExMxPmMpls(GT_VOID);
/* monitor */
GT_STATUS cmdLibInitCpssExMxPmMirror(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmStc(GT_VOID);
/* networkIf */
GT_STATUS cmdLibInitCpssExMxPmNetIf(GT_VOID);
/* nst */
GT_STATUS cmdLibInitCpssExMxPmNstBrg(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmNstIp(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmNstPort(GT_VOID);
/* pcl */
GT_STATUS cmdLibInitCpssExMxPmPcl(GT_VOID);
/* external memory */
GT_STATUS cmdLibInitCpssExMxPmExternalMemory(GT_VOID);
/* external TCAM */
GT_STATUS cmdLibInitCpssExMxPmExtTcam(GT_VOID);
/* phy */
GT_STATUS cmdLibInitCpssExMxPmPhySmi(GT_VOID);
/* policer */
GT_STATUS cmdLibInitCpssExMxPmPolicer(GT_VOID);
/* port */
GT_STATUS cmdLibInitCpssExMxPmPortBufMg(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmPortInterlaken(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmPortCtrl(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmPortStat(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmPortTx(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmPortPfc(GT_VOID);
/* sct */
GT_STATUS cmdLibInitCpssExMxPmSct(GT_VOID);
/* trunk */
GT_STATUS cmdLibInitCpssExMxPmTrunk(GT_VOID);
/* qos */
GT_STATUS cmdLibInitCpssExMxPmQos(GT_VOID);
/* tti */
GT_STATUS cmdLibInitCpssExMxPmTti(GT_VOID);
/* tunnelStart */
GT_STATUS cmdLibInitCpssExMxPmTunnelStart(GT_VOID);

GT_STATUS cmdLibInitCpssExMxPmDiag(GT_VOID);
/*fabric*/
GT_STATUS cmdLibInitCpssExMxPmFabricXbar(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmFabricVoq(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmFabricE2e(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmFabricPktReasm(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmFabricSerdes(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmFabricHGLink(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmFabricTxqDevMap(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmFabricTxqHeaderMerger(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmFabricTxqFc(GT_VOID);
GT_STATUS cmdLibInitCpssExMxPmFabricTxqQueueMap(GT_VOID);

/* vb */
GT_STATUS cmdLibInitCpssExMxPmVb(GT_VOID);

/* vnt */
GT_STATUS cmdLibInitCpssExMxPmVnt(GT_VOID);

/* counter engine */
GT_STATUS cmdLibInitCpssExMxPmCounterEngine(GT_VOID);

#endif /*IMPL_CPSS_EXMXPM*/

#ifdef PX_FAMILY
#define HWS_CMD_COMMON_INIT
GT_STATUS cmdLibInitCpssPxPortMgBuf(GT_VOID);
GT_STATUS cmdLibInitCpssPxCos(GT_VOID);
GT_STATUS cmdLibInitCpssPxCutThrough(GT_VOID);
GT_STATUS cmdLibInitCpssPxIngress(GT_VOID);
GT_STATUS cmdLibInitCpssPxEgress(GT_VOID);
GT_STATUS cmdLibInitCpssPxIngressHash(GT_VOID);
GT_STATUS cmdLibInitCpssPxPortCtrl(GT_VOID);
GT_STATUS cmdLibInitCpssPxDiag(GT_VOID);
GT_STATUS cmdLibInitCpssPxPortMapping(GT_VOID);
GT_STATUS cmdLibInitCpssPxPortStat(GT_VOID);
GT_STATUS cmdLibInitCpssPxDiagPacketGenerator(GT_VOID);
GT_STATUS cmdLibInitCpssPxNetIf(GT_VOID);
GT_STATUS cmdLibInitCpssPxPortTxDebug(GT_VOID);
GT_STATUS cmdLibInitCpssPxPortFWS(GT_VOID);
GT_STATUS cmdLibInitCpssPxPortTxScheduler(GT_VOID);
GT_STATUS cmdLibInitCpssPxPortTxTailDrop(GT_VOID);
GT_STATUS cmdLibInitCpssPxCnc(GT_VOID);
GT_STATUS cmdLibInitCpssPxPortTxShaper(GT_VOID);
GT_STATUS cmdLibInitCpssPxHwInitLedCtrl(GT_VOID);
GT_STATUS cmdLibInitCpssPxHwInitMisc(GT_VOID);
GT_STATUS cmdLibInitCpssPxHwInit(GT_VOID);
GT_STATUS cmdLibInitCpssPxPortCn(GT_VOID);
GT_STATUS cmdLibInitCpssPxPortManager(GT_VOID);
GT_STATUS cmdLibInitCpssPxPortAp(GT_VOID);
GT_STATUS cmdLibInitCpssPxPortEcn(GT_VOID);
GT_STATUS cmdLibInitCpssPxPfc(GT_VOID);
GT_STATUS cmdLibInitCpssPxPtp(GT_VOID);

#endif /*PX_FAMILY*/

#if defined (INCLUDE_TM)
#ifdef IMPL_TM
extern GT_STATUS cmdLibInitCpssTmCtl(void);
extern GT_STATUS cmdLibInitCpssTmSched(void);
extern GT_STATUS cmdLibInitCpssTmShaping(void);
extern GT_STATUS cmdLibInitCpssTmDrop(void);
extern GT_STATUS cmdLibInitCpssTmNodesCreate(void);
extern GT_STATUS cmdLibInitCpssTmNodesTree(void);
extern GT_STATUS cmdLibInitCpssTmNodesRead(void);
extern GT_STATUS cmdLibInitCpssTm2Tm(void);
extern GT_STATUS cmdLibInitCpssTmNodesUpdate(void);
extern GT_STATUS cmdLibInitCpssTmNodesReorder(void);
extern GT_STATUS cmdLibInitCpssTmNodesStatus(void);
extern GT_STATUS cmdLibInitCpssTmStatistics(void);
extern GT_STATUS cmdLibInitCpssTmEligPrioFunc(void);
extern GT_STATUS cmdLibInitCpssTmNodesCtl(void);
extern GT_STATUS cmdLibInitCpssTmLogicalLayer(void);

#endif /*IMPL_TM*/
#endif /*#if defined (INCLUDE_TM)*/

#ifdef INCLUDE_UTF
GT_STATUS cmdLibInitUtfMain(GT_VOID);
#endif
GT_STATUS cmdLibInitCpssDriverPpHw(GT_VOID);

#ifdef GENERIC_MAIN_PP_DRV_CMD_INIT
GT_STATUS cmdLibInitCpssGenPhyVct(GT_VOID);
GT_STATUS cmdLibInitCpssGenPhySmi(GT_VOID);
GT_STATUS cmdLibInitCpssGenHsu(GT_VOID);
#endif /*GENERIC_MAIN_PP_DRV_CMD_INIT */
GT_STATUS cmdLibInitCpssGenSmi(GT_VOID);

GT_STATUS cmdLibInitCpssGenNetIf(GT_VOID);
GT_STATUS cmdLibInitCpssGenEvents(GT_VOID);
GT_STATUS cmdLibInitCpssGenSystemRecovery(GT_VOID);

#ifdef DRAGONITE_TYPE_A1
GT_STATUS cmdLibInitCpssGenDragonite(GT_VOID);
#endif

GT_STATUS cmdLibInitCpssLog(GT_VOID);

#ifdef IMPL_FPGA
    GT_STATUS cmdLibRhodesInit(GT_VOID);
#endif

#ifdef HWS_CMD_COMMON_INIT
GT_STATUS cmdLibInitHwsMacIf(GT_VOID);
GT_STATUS cmdLibInitHwsPcsIf(GT_VOID);
GT_STATUS cmdLibInitHwsSerdesInit(GT_VOID);
GT_STATUS cmdLibInitSerdesExt(GT_VOID);
GT_STATUS cmdLibInitCommon(GT_VOID);
GT_STATUS cmdLibInitHwsPortInit(GT_VOID);
#endif /*HWS_CMD_COMMON_INIT*/

#ifdef HWS_CMD_INIT
/* HWS wrappers*/
GT_STATUS cmdLibInitDDR3(GT_VOID);
GT_STATUS cmdLibInitDDR3TrainingIP(GT_VOID);
GT_STATUS cmdLibInitPortAp(GT_VOID);
GT_STATUS cmdLibInitDdr3Bap(GT_VOID);
GT_STATUS cmdLibInitHwsDebug(GT_VOID);
#endif /*HWS_CMD_INIT*/


/**************************************************************************/
/*For Stubs set here in order to handle Unsupported Galtis funcs*/
static CMD_STATUS wrCpssDxChCfgDevMacAddrSet

(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);
    galtisOutput(outArgs, GT_NOT_SUPPORTED, "");
    return CMD_OK;
}

static CMD_STATUS wrCpssDxChCfgDevMacAddrLsbModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(inArgs);
    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);
    galtisOutput(outArgs, GT_NOT_SUPPORTED, "");
    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChCfgDevMacAddrSet",
        &wrCpssDxChCfgDevMacAddrSet,
        2, 0},

    {"cpssDxChCfgDevMacAddrLsbModeSet",
        &wrCpssDxChCfgDevMacAddrLsbModeSet,
        2, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**************************************************************************/


/* global CPSS database initialization routine */
GT_STATUS cmdCpssLibInit( void )
{

#ifdef IMPL_CPSS_EXMX
    if ( cmdLibInitCpssExMxBridge() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxConfig() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxCos() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxDiag() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxHwInit() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxI2c() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxInlif() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxMirror() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxStc() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxNetworkIf() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxPcl() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxPhy() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxPolicy() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxPolicer() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxPort() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxTrunk() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxIp() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxIpFlow() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxVb() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxIpLpm() != GT_OK)
        return GT_FAIL;

#endif /* IMPL_CPSS_EXMX */


#ifdef IMPL_CPSS_EXMXTG
    if ( cmdLibInitCpssExMxTgBridge() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxTgConfig() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxTgPcl() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxTgTrunk() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxInlifTg() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxTgIp() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxTgTunnelEntry() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssExMxTgTunnelTerm() != GT_OK)
        return GT_FAIL;

#endif /* IMPL_CPSS_EXMXTG*/



#ifdef CHX_FAMILY
    if ( cmdLibInitCpssDxChBridge() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChBridgeCount() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChBridgeFdb() != GT_OK)
        return GT_FAIL;
     if ( cmdLibInitCpssDxChBridgeFdbRouting() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChBridgeGen() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChBridgePrvEdgeVlan() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChBridgeSecurityBreach() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChBridgeSrcId() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChBridgeVlan() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChBridgePe() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChCnc() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChConfig() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChCos() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChCscd() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChDiag() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChDiagDataIntegrity() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChMirror() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChStc() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChNetIf() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChNst() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChNstPortIsolation() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChPcl() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChPha() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChPhy() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChPort() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChPortCtrl() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChPolicer() != GT_OK)
        return GT_FAIL;
   if ( cmdLibInitCpssDxChTrunk() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChIp() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChIpCtrl() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChIpLpm() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChHsu() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChTcamManager() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChTunnel() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssDxChHwInit() != GT_OK)
         return GT_FAIL;
    if (cmdLibInitCpssDxChHwInitLedCtrl() != GT_OK)
         return GT_FAIL;
    if ( cmdLibInitCpssDxChTti() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChVnt() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChVersion() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChLogicalTargetMapping() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChCutThrough() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChPtp() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChMultiPortGroup() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChFabricHGLink() != GT_OK)
        return GT_FAIL;
    if( cmdLibInitCpssDxChPortAp() != GT_OK)
        return GT_FAIL;
    if( cmdLibInitCpssDxChPortPip() != GT_OK)
        return GT_FAIL;
    if( cmdLibInitCpssDxChPortEee() != GT_OK)
        return GT_FAIL;
    if( cmdLibInitCpssDxChPortMapping() != GT_OK)
        return GT_FAIL;
    if( cmdLibInitCpssDxChPortInterlaken() != GT_OK)
        return GT_FAIL;
    if( cmdLibInitCpssDxChPortDiag() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssDxChPortSlicesDirectCtrl() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssDxChPortDynamicPizzaArbiter() != GT_OK)
        return GT_FAIL;
    if( cmdLibInitCpssDxChOam() != GT_OK)
        return GT_FAIL;
    if( cmdLibInitCpssDxChOamSrv() != GT_OK)
        return GT_FAIL;
    if( cmdLibInitCpssDxChL2Mll() != GT_OK)
        return GT_FAIL;
    if( cmdLibInitCpssDxChTcam() != GT_OK)
        return GT_FAIL;
    if( cmdLibInitCpssDxChExactMatch() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChProtection() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssDxChPortManager() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssDxChPortLoopback() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssDxChLatencyMonitoring() != GT_OK)
        return GT_FAIL;

#if defined (INCLUDE_TM)
    if ( cmdLibInitCpssDxChTmGlueDrop() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChTmGlueAgingAndDelay() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChTmGlueQueueMap() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChTmGlueDram() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChTmGluePfc() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChTmGlueFlowControl() != GT_OK)
        return GT_FAIL;
#endif /*#if defined (INCLUDE_TM)*/


    if ( cmdLibInitCpssDxChLpm() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCpssDxChVirtualTcam() != GT_OK)
        return GT_FAIL;

    if (cmdLibInitCpssDxCh3Policer() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssDxChIpfix() != GT_OK)
        return GT_FAIL;
#endif /* CHX_FAMILY*/


#ifdef PX_FAMILY
    if (cmdLibInitCpssPxPortMgBuf() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxCos() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxCutThrough() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxIngress() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxEgress() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxIngressHash() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPortCtrl() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxDiag() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPortMapping() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPortStat() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxDiagPacketGenerator() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxNetIf() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPortTxDebug() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPortFWS() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPortTxScheduler() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPortTxTailDrop() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxCnc() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPortTxShaper() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxHwInitLedCtrl() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxHwInitMisc() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxHwInit() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPortCn() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPortAp() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPortEcn() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPtp() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPortManager() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssPxPfc() != GT_OK)
        return GT_FAIL;

#endif /*PX_FAMILY*/

#if defined (INCLUDE_TM)
#ifdef IMPL_TM
    if (cmdLibInitCpssTmCtl() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmSched() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmShaping() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmDrop() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmNodesCreate() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmNodesTree() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmNodesRead() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTm2Tm() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmNodesUpdate() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmNodesReorder() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmNodesStatus() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmStatistics() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmNodesCtl() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmEligPrioFunc() != GT_OK)
        return GT_FAIL;
    if (cmdLibInitCpssTmLogicalLayer() != GT_OK)
        return GT_FAIL;
#endif /*IMPL_TM*/
#endif /*#if defined (INCLUDE_TM)*/


if( cmdLibInitCpssGenNetIf() != GT_OK)
    return GT_FAIL;

if( cmdLibInitCpssGenEvents() != GT_OK)
    return GT_FAIL;
#ifdef GENERIC_MAIN_PP_DRV_CMD_INIT
if ( cmdLibInitCpssGenPhyVct() != GT_OK)
    return GT_FAIL;
if ( cmdLibInitCpssGenPhySmi() != GT_OK)
    return GT_FAIL;
if ( cmdLibInitCpssGenHsu() != GT_OK)
    return GT_FAIL;
#endif /*GENERIC_MAIN_PP_DRV_CMD_INIT*/

if ( cmdLibInitCpssGenSmi() != GT_OK)
    return GT_FAIL;

#ifdef DRAGONITE_TYPE_A1
if ( cmdLibInitCpssGenDragonite() != GT_OK)
    return GT_FAIL;
#endif /* DRAGONITE_TYPE_A1 */
if ( cmdLibInitCpssGenSystemRecovery() != GT_OK)
    return GT_FAIL;
#ifdef INCLUDE_UTF
    if ( cmdLibInitUtfMain() != GT_OK)
        return GT_FAIL;
#endif

    if ( cmdLibInitCpssDriverPpHw() != GT_OK)
        return GT_FAIL;

    /*For Stubs set here in order to handle Unsupported Galtis funcs*/
    if (cmdInitLibrary(dbCommands, numCommands) != GT_OK)
        return GT_FAIL;


#ifdef IMPL_FPGA
    if ( cmdLibRhodesInit() != GT_OK)
        return GT_FAIL;
#endif

#ifdef HWS_CMD_INIT
    if ( cmdLibInitDDR3TrainingIP() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitPortAp() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitDdr3Bap() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitHwsDebug() != GT_OK)
        return GT_FAIL;
#endif /*HWS_CMD_INIT*/

#ifdef HWS_CMD_COMMON_INIT
    if ( cmdLibInitHwsMacIf() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitHwsPcsIf() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitHwsPortInit() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitHwsSerdesInit() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitSerdesExt() != GT_OK)
        return GT_FAIL;
    if ( cmdLibInitCommon() != GT_OK)
        return GT_FAIL;
#endif /*HWS_CMD_COMMON_INIT*/

#ifdef CPSS_LOG_ENABLE
    if ( cmdLibInitCpssLog() != GT_OK)
        return GT_FAIL;
#endif


    return GT_OK;
}
