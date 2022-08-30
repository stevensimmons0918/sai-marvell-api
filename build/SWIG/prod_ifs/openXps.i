////////////////////////////////////////
//<product>.i - interface specification for product <product> to Python
////////////////////////////////////////

%include "SWIG/sdk.top.i"

%include <stdint.i>
%include <std_vector.i>

//common stuff to all Python product interfaces
%include "SWIG/common.i"


%include <cpointer.i>
%include <carrays.i>
%include <cmalloc.i>
%array_functions(uint8_t, uint8Arr_tp)
%array_functions(uint8_t, arr2_tp)
%array_functions(uint8_t, charArr_tp)
%array_functions(uint8_t*, twoDimChar_tp)
%array_functions(uint8_t, arr4_tp)
%array_functions(uint8_t, arr6_tp)
%array_functions(uint8_t, arr16_tp)
%array_functions(int ,intArr)
%array_functions(char, charArr)
%array_functions(xpDevice_t, xpDevArr)
%array_functions(uint32_t ,arrUint32)
%array_functions(uint64_t ,arrUint64)
%array_functions(uint32_t*,arrPtrUint32)
%array_functions(uint16_t ,arrUint16)
%array_functions(uint8_t, CONSTDATA256_tp)
%array_functions(xpTranslationType_t, xpTranslationType_arr);
%array_functions(xpWcmInstrType_t, xpWcmInstrType_arr);
%array_functions(xpWcmInstr, xpWcmInstr_arr);
%array_functions(xpQueueInfo, xpQueueInfoArr)
%array_functions(xpPort_t*, xpPort_arr)
%array_functions(xpSerdesAddr_t, xpSerdesAddr_tp)
%pointer_functions(XP_PROFILE_TYPE, XP_PROFILE_TYPE_P)
%array_functions(xpSlaveResetId_e, xpSlaveResetId_arr)
%array_functions(xpHashField,xpHashField_arr)
%array_functions(xpLayerType_t,xpLayerType_arr)
%array_functions(xpsInterfaceId_t,xpsInterfaceId_arr)
%pointer_functions(xpsVlanConfig_t, xpsVlanConfig_tp);
%pointer_functions(xpsServiceIdData_t, xpsServiceIdData_tp);
%pointer_functions(uint8_t, uint8_tp);
%pointer_functions(bool, bool_tp)
%pointer_functions(bool, boolp)
%pointer_functions(xpPreambleLenBytes, xpPreambleLenBytesp)
%pointer_functions(xpRxPreambleLenBytes, xpRxPreambleLenBytesp)
%pointer_functions(xpSpeed, xpSpeedp)
%pointer_functions(xp_Statistics, xp_Statisticsp)
%pointer_functions(float, floatp)
%pointer_functions(xpMacConfigMode, xpMacConfigMode_p)
%pointer_functions(uint16_t, uint16_tp);
%pointer_functions(uint32_t, uint32_tp);
%pointer_functions(uint64_t, uint64_tp);
%pointer_functions(int8_t, int8_tp);
%pointer_functions(int16_t, int16_tp);
%pointer_functions(int32_t, int32_tp);
%pointer_functions(int64_t, int64_tp);
%pointer_functions(xpSpeed, xpSpeed_p);
%pointer_functions(int, intp);
%pointer_functions(xpMplsLabelOper, xpMplsLabelOperp);
%pointer_functions(xpAcm_t, xpAcm_tp);
%pointer_functions(char, charp);
%pointer_functions(xphRxHdr, xphRxHdr_tp);
%pointer_functions(xpIdRangeInfo, xpIdRangeInfop);
%pointer_functions(xpPortList_t, xpPortList_tp);
%pointer_functions(xpsDevice_t, xpsDevice_tp);
%pointer_functions(xpLagList_t, xpLagListTp);
%pointer_functions(xpVif_t, xpVif_tp);
%pointer_functions(vector<uint32_t>, xpUint32Vecp);
%pointer_functions(xpL2DomainCtx_t, xpL2DomainCtx_tp)
%pointer_functions(xpMulticastVifEntry_t, xpMulticastVifEntry_tp)
%pointer_functions(xpMcastDomainId_t, xpMcastDomainId_tp)
%pointer_functions(xpsMulticastIPv4BridgeEntry_t, xpsMulticastIPv4BridgeEntry_tp)
%pointer_functions(xpsMulticastIPv6BridgeEntry_t, xpsMulticastIPv6BridgeEntry_tp)
%pointer_functions(xpsMulticastIPv4RouteEntry_t, xpsMulticastIPv4RouteEntry_tp)
%pointer_functions(xpsMulticastIPv6RouteEntry_t, xpsMulticastIPv6RouteEntry_tp)
%pointer_functions(xpsMulticastIPv4PimBidirRpfEntry_t, xpsMulticastIPv4PimBidirRpfEntry_tp)
%pointer_functions(xpsMcastDomainId_t, xpsMcastDomainId_tp)
%pointer_functions(xpDatapath_t, xpDatapath_tp)
%pointer_functions(xpFlowEntry_t, xpFlowEntry_tp)
%pointer_functions(xpPktCmd_e, xpPktCmd_ep)
%pointer_functions(xpMplsLabelOper, xpMplsLabelOper_tp)
%pointer_functions(xpsIpTunnelData_t, xpsIpTunnelData_tp)
%pointer_functions(xpsPrivateVlanType_e, xpsPrivateVlanType_ep)
%pointer_functions(xpPortConfig_t, xpPortConfig_tp)
%pointer_functions(xpVlanStgState_e, xpVlanStgState_ep)
%pointer_functions(xpCountMode_e, xpCountMode_ep)
%pointer_functions(xpVlanBridgeMcMode_e, xpVlanBridgeMcMode_ep)
%pointer_functions(xpVlanRouteMcMode_t, xpVlanRouteMcMode_tp)
%pointer_functions(xpVlanNatMode_e, xpVlanNatMode_ep)
%pointer_functions(xpVlan_t, xpVlan_tp)
%pointer_functions(xpScope_t, xpScope_tp)
%pointer_functions(xpAgeFifoData, xpAgeFifoData_tp)
%pointer_functions(xpSeInsPtrs_t, xpSeInsPtrs_tp)
%pointer_functions(xpsVlan_t, xpsVlan_tp)
%pointer_functions(xpRegAccessMode_t, xpRegAccessMode_tp)
%array_functions(xpsVlan_t*,xpsVlan_tpp);
%pointer_functions(xpsVlanBridgeMcMode_e, xpsVlanBridgeMcMode_ep)
%pointer_functions(xpsPktCmd_e, xpsPktCmd_ep)
%pointer_functions(xpsCountMode_e, xpsCountMode_ep)
%pointer_functions(xpsInterfaceId_t, xpsInterfaceId_tp)
%pointer_functions(xpsPeg_t, xpsPeg_tp)
%array_functions(xpsInterfaceId_t*,xpsInterfaceId_tpp);
%pointer_functions(xpsStp_t, xpsStp_tp)
%pointer_functions(xpsScope_t, xpsScope_tp)
%pointer_functions(xpsEgressFilter_t, xpsEgressFilter_tp)
%pointer_functions(xpsDbHandle_t, xpsDbHandle_tp)
%pointer_functions(xpsStpState_e, xpsStpState_ep)
%pointer_functions(xpsFdbEntry_t, xpsFdbEntry_tp)
%pointer_functions(xpsInterfaceInfo_t, xpsInterfaceInfo_tp)
%pointer_functions(xpsInterfaceType_e, xpsInterfaceType_ep)
%pointer_functions(xpsPortList_t, xpsPortList_tp)
%pointer_functions(xpsNatEntryKey_t, xpsNatEntryKey_tp)
%pointer_functions(xpsNatEntryMask_t, xpsNatEntryMask_tp)
%pointer_functions(xpsNatEntryData_t, xpsNatEntryData_tp)
%pointer_functions(xpsVpnGreTunnelConfig_t, xpsVpnGreTunnelConfig_tp)
%pointer_functions(xpsMplsVpnParams_t, xpsMplsVpnParams_tp)
%pointer_functions(xpsMplsVpnGreLooseModeParams_t, xpsMplsVpnGreLooseModeParams_tp)
%pointer_functions(xpsMplsLabelEntry_t, xpsMplsLabelEntry_tp)
%pointer_functions(xpPCSTestPattern, xpPCSTestPatternp)
%pointer_functions(xpSerdesPmdConfig_t, xpSerdesPmdConfig_tp)
%pointer_functions(xpSerdesDfeRepeat_t, xpSerdesDfeRepeat_tp)
%pointer_functions(xpSerdesDfeState_t, xpSerdesDfeState_tp)
%pointer_functions(xpDiagConfig_t,xpDiagConfig_tp)
%pointer_functions(xpSerdesEyeConfig_t,xpSerdesEyeConfig_tp)
%pointer_functions(xpSerdesEyeData_t,xpSerdesEyeData_tp)
%pointer_functions(xpsPortIntfMap_t,xpsPortIntfMap_tp)
%pointer_functions(BOOL,BOOLp)
%pointer_functions(xpDevice_t,xpDevice_tp)
%pointer_functions(xpPort_t,xpPort_tp)
%pointer_functions(macAddrLow_t, macAddrLow_tp)
%pointer_functions(xpsL3HostEntry_t, xpsL3HostEntry_tp)
%pointer_functions(xpsL3RouteEntry_t, xpsL3RouteEntry_tp)
%pointer_functions(xpsL3NextHopEntry_t, xpsL3NextHopEntry_tp)
%pointer_functions(xpsGeneveFormatType_t,xpsGeneveFormatType_tp)
%pointer_functions(xpsVxlanTunnelConfig_t,xpsVxlanTunnelConfig_tp)
%pointer_functions(xpsNvgreTunnelConfig_t,xpsNvgreTunnelConfig_tp)
%pointer_functions(xpsIpGreTunnelConfig_t,xpsIpGreTunnelConfig_tp)
%pointer_functions(xpsIpinIpTunnelConfig_t,xpsIpinIpTunnelConfig_tp)
%pointer_functions(xpsGeneveTunnelConfig_t,xpsGeneveTunnelConfig_tp)
%pointer_functions(xpsMplsTunnelKey_t,xpsMplsTunnelKey_tp)
%pointer_functions(xpsMplsTunnelParams_t,xpsMplsTunnelParams_tp)
%pointer_functions(xpSerdesTxDataSel_t,xpSerdesTxDataSel_tp)
%pointer_functions(xpSerdesEyeHbtc_t,xpSerdesEyeHbtc_tp)
%pointer_functions(xpSerdesEyeVbtc_t,xpSerdesEyeVbtc_tp)
%pointer_functions(xpSerdesTxEqLimits_t,xpSerdesTxEqLimits_tp)
%pointer_functions(xpSerdesCtle_t,xpSerdesCtle_tp)
%pointer_functions(xpSerdesTxEq_t,xpSerdesTxEq_tp)
%pointer_functions(xpsMcL2InterfaceListId_t, xpsMcL2InterfaceListId_tp)
%pointer_functions(xpsPortConfig_t,xpsPortConfig_tp)
%pointer_functions(xpSerdesRxCmpData_t,xpSerdesRxCmpData_tp)
%pointer_functions(xpSerdesRxTerm_t,xpSerdesRxTerm_tp)
%pointer_functions(xpSerdesRxDataQual_t,xpSerdesRxDataQual_tp)
%pointer_functions(xpSerdesTxPllClk_t,xpSerdesTxPllClk_tp)
%pointer_functions(xpSerdesAnInfo_t,xpSerdesAnInfo_tp)
%pointer_functions(xpSerdesRxCmpMode_t,xpSerdesRxCmpMode_tp)
%pointer_functions(xpSerdesMemType_t,xpSerdesMemType_tp)
%pointer_functions(xpsMcL3InterfaceListId_t, xpsMcL3InterfaceListId_tp)
%pointer_functions(xpLagDistributionEntry_t,xpLagDistributionEntry_tp)
%pointer_functions(xpsPortFrameType_e,xpsPortFrameType_ep)
%pointer_functions(xpsPolicerEntry_t,xpsPolicerEntry_tp)
%pointer_functions(xpsPolicerCounterEntry_t,xpsPolicerCounterEntry_tp)
%pointer_functions(xpPolicerCounterEntry_t,xpPolicerCounterEntry_tp)
%pointer_functions(xpPolicerResult_t,xpPolicerResult_tp)
%pointer_functions(xpSerdesPllState_t,xpSerdesPllState_tp)
%pointer_functions(xpSerdesPcsFifoClk_t,xpSerdesPcsFifoClk_tp)
%pointer_functions(xpSerdesStatus_t,xpSerdesStatus_tp)
%pointer_functions(xpSerdesInt_t,xpSerdesInt_tp)
%pointer_functions(xpSerdesBsbMode_t,xpSerdesBsbMode_tp)
%pointer_functions(xpSerdesBsbClkSel_t,xpSerdesBsbClkSel_tp)
%pointer_functions(FILE,FILEp)
%pointer_functions(xpDevConfigStruct,xpDevConfigStructp)
%pointer_functions(xpPolicerEntry_t,xpPolicerEntry_tp)
%pointer_functions(xpsLagPortIntfList_t, xpsLagPortIntfList_tp)
%pointer_functions(xpQosMapPfl_t, xpQosMapPfl_tp)
%pointer_functions(xpPCSEnableTestMode,xpPCSEnableTestModep)
%pointer_functions(xpPCSDataPattern,xpPCSDataPatternp)
%pointer_functions(xpBackPlaneAbilityModes,xpBackPlaneAbilityModesp)
%pointer_functions(xphTxHdrMetaData,xphTxHdrMetaData_tp)
%pointer_functions(xpCoreClkFreq_t, xpCoreClkFreq_tp)
%pointer_functions(xpBypassMode_e, xpBypassMode_ep)
%pointer_functions(XP_AGE_MODE_T, XP_AGE_MODE_Tp)
%pointer_functions(xpWredMode_e, xpWredMode_ep)
%pointer_functions(xpQGuarThresholdGranularity_e, xpQGuarThresholdGranularity_ep)
%pointer_functions(xpDynThldFraction_e, xpDynThldFraction_ep)
%pointer_functions(xpDynThldCoeff_e, xpDynThldCoeff_ep)
%pointer_functions(xpDynThldOperator_e, xpDynThldOperator_ep)
%pointer_functions(xpEgressCosMapData_t, xpEgressCosMapData_tp)
%pointer_functions(xpTxqQueuePathToPort_t, xpTxqQueuePathToPort_tp)
%pointer_functions(xpCoppEntryData_t, xpCoppEntryData_tp)
%pointer_functions(xpsHashIndexList_t, xpsHashIndexList_tp)
%pointer_functions(xpCpuTypes_e, xpCpuTypes_ep)
%pointer_functions(xpFecMode, xpFecModep)
%pointer_functions(xpDecodeTrap, xpDecodeTrapp)
%pointer_functions(xpMacRxFault, xpMacRxFaultp)
%pointer_functions(xpWcmInstrs, xpWcmInstrs_tp)
%pointer_functions(xpWcmInstr, xpWcmInstr_tp)
%pointer_functions(xpWcmMatchInstr,xpWcmMatchInstr_tp)
%pointer_functions(xpWcmRangeInstr,xpWcmRangeInstr_tp)
%pointer_functions(xpTranslationType_t,xpTranslationType_tp);
%pointer_functions(xpHashIndexList_t, xpHashIndexList_tp);
%pointer_functions(xpPrefix_t, xpPrefix_tp);
%pointer_functions(xpsPort_t, xpsPort_tp);
%pointer_functions(Float, Floatp)
%pointer_functions(xpSerdesClk_t, xpSerdesClk_tp)
%pointer_functions(xpsVlanCountMode_e, xpsVlanCountMode_ep)
%pointer_functions(xpSerdes_t, xpSerdes_tp)
%pointer_functions(xpSerdesDfeTune_t, xpSerdesDfeTune_tp)
%pointer_functions(xphRxHdr, xphRxHdrp)
%pointer_functions(xpPacketInfo, xpPacketInfop)
%pointer_functions(xpSchedPolicy, xpSchedPolicyp)
%pointer_functions(rxQueueWeightInfo, rxQueueWeightInfop)
%pointer_functions(xpRxConfigMode, xpRxConfigModep)
%pointer_functions(xpPacketInterface, xpInterfacep)
%pointer_functions(xpsVlanNatMode_e, xpsVlanNatMode_ep)
%pointer_functions(xpsUrpfMode_e, xpsUrpfMode_ep)
%pointer_functions(xpsL2EncapType_e, xpsL2EncapType_ep)
%pointer_functions(xpsMirrorBktDbEntry_t, xpsMirrorBktDbEntry_tp)
%pointer_functions(xpL2EncapType_e, xpL2EncapType_ep)
%pointer_functions(xpsIpTunnelConfig_t, xpsIpTunnelConfig_tp)
%pointer_functions(uint8_t, ipv4Addr_tp)
%pointer_functions(xpsIpMcOIFData_t, xpsIpMcOIFData_tp)
%pointer_functions(xpDmaDescDebugEntities, xpDmaDescDebugEntities_tp)
%pointer_functions(xpPolicerStandard_e, xpPolicerStandard_ep)
%pointer_functions(xpsSrhData_t, xpsSrhData_tp)
%pointer_functions(xpsSidEntry, xpsSidEntry_tp)
%pointer_functions(xpPtGlobalState_e, xpPtGlobalState_ep)
%pointer_functions(xpPtPortState_e, xpPtPortState_ep)
%pointer_functions(xpPtMirrorModes_e, xpPtMirrorModes_ep)
%pointer_functions(xpPktTraceMode, xpPktTraceMode_ep)

// common stuff to this product interface front-end files
%include "SWIG/sdk.gen.i"

// stuff from the files that we want SWIG to ignore
%include "../utils/state/include/xpsState.src.i"
%include "../xps/include/openXpsTypes.src.i"
%include "../xps/include/xpsInterface.src.i"
%include "../xps/include/xpsSerdes.src.i"

// stuff from product .cpp files that has to be given to SWIG
%include "SWIG/sdk.gen.cpp"

%include "../demo/common/py/xpPyEmbed.gen.i"

%include "../demo/config/include/xpDevTypes.gen.i"

//%include "../utils/allocator/include/mrvl_allocator.gen.i"
//%include "../utils/allocator/include/mrvl_allocator_mgr.gen.i"
//%include "../utils/allocator/include/mrvl_bit_vector.gen.i"
//%include "../utils/allocator/include/mrvl_id_allocator_context.gen.i"
//%include "../utils/allocator/include/mrvl_id_allocator.gen.i"
//%include "../utils/allocator/include/mrvl_lock_arr.gen.i"
//%include "../utils/allocator/include/mrvl_lock.gen.i"
//%include "../utils/allocator/include/mrvl_mutex_base.gen.i"
//%include "../utils/allocator/include/mrvl_mutexDummy.gen.i"
//%include "../utils/allocator/include/mrvl_mutex.gen.i"
//%include "../utils/allocator/include/mrvl_mutex_posix.gen.i"
//%include "../utils/allocator/include/mrvl_persistent_bit_vector.gen.i"

%include "../utils/sal/include/xpsSal.gen.i"

%include "../utils/state/include/xpsRBTree.gen.i"
%include "../utils/state/include/xpsState.gen.i"

%include "../xps/include/openXpsAcm.gen.i"
%include "../xps/include/openXpsAging.gen.i"
%include "../xps/include/openXpsCopp.gen.i"
%include "../xps/include/openXpsEnums.gen.i"
%include "../xps/include/openXpsFdb.gen.i"
%include "../xps/include/openXpsInit.gen.i"
%include "../xps/include/openXpsInterface.gen.i"
%include "../xps/include/openXpsL3.gen.i"
%include "../xps/include/openXpsLag.gen.i"
%include "../xps/include/openXpsMac.gen.i"
%include "../xps/include/openXpsOpenflow.gen.i"
%include "../xps/include/openXpsPacketDrv.gen.i"
%include "../xps/include/openXpsPolicer.gen.i"
%include "../xps/include/openXpsPort.gen.i"
%include "../xps/include/openXpsQos.gen.i"
%include "../xps/include/openXpsReasonCodeTable.gen.i"
%include "../xps/include/openXpsStp.gen.i"
%include "../xps/include/openXpsTypes.gen.i"
%include "../xps/include/openXpsVlan.gen.i"
%include "../xps/include/xpDevConfig.gen.i"
%include "../xps/include/xpEnums.gen.i"
%include "../utils/allocator/include/xpsGeneric.gen.i"
%include "../xps/include/xpsReasonCodeTable.gen.i"
%include "../xps/include/xps8021Br.gen.i"
%include "../xps/include/xpsAc.gen.i"
%include "../xps/include/xpsAcm.gen.i"
%include "../xps/include/xpsAging.gen.i"
%include "../xps/include/xpsAllocator.gen.i"
%include "../xps/include/xpsCommon.gen.i"
%include "../xps/include/xpsCopp.gen.i"
%include "../xps/include/xpsEgressFilter.gen.i"
%include "../xps/include/xpsEnums.gen.i"
%include "../xps/include/xpsErspanGre.gen.i"
%include "../xps/include/xpsFdb.gen.i"
%include "../xps/include/xpsGeneve.gen.i"
%include "../xps/include/xpsGlobalSwitchControl.gen.i"
%include "../xps/include/xpsAcl.gen.i"
%include "../xps/include/xpsInit.gen.i"
%include "../xps/include/xpsInterface.gen.i"
%include "../xps/include/xpsInternal.gen.i"
%include "../xps/include/xpsInt.gen.i"
%include "../xps/include/xpsIpGre.gen.i"
%include "../xps/include/xpsIpinIp.gen.i"
%include "../xps/include/xpsL3.gen.i"
%include "../xps/include/xpsLag.gen.i"
%include "../xps/include/xpsLink.gen.i"
%include "../xps/include/xpsLock.gen.i"
%include "../xps/include/xpsMac.gen.i"
%include "../xps/include/xpsMirror.gen.i"
%include "../xps/include/xpsMpls.gen.i"
%include "../xps/include/xpsMtuProfile.gen.i"
%include "../xps/include/xpsMulticast.gen.i"
%include "../xps/include/xpsNat.gen.i"
%include "../xps/include/xpsNhGrp.gen.i"
%include "../xps/include/xpsNvgre.gen.i"
%include "../xps/include/xpsOpenflow.gen.i"
%include "../xps/include/xpsPacketDrv.gen.i"
%include "../xps/include/xpsPacketTrakker.gen.i"
%include "../xps/include/xpsPbb.gen.i"
%include "../xps/include/xpsPolicer.gen.i"
%include "../xps/include/xpsPort.gen.i"
%include "../xps/include/xpsPtp.gen.i"
%include "../xps/include/xpsQos.gen.i"
%include "../xps/include/xpsScope.gen.i"
%include "../xps/include/xpsSerdes.gen.i"
%include "../xps/include/xpsSflow.gen.i"
%include "../xps/include/xpsSr.gen.i"
%include "../xps/include/xpsStp.gen.i"
%include "../xps/include/xpsTunnel.gen.i"
%include "../xps/include/xpsUtil.gen.i"
%include "../xps/include/xpsVlan.gen.i"
%include "../xps/include/xpsVpnGre.gen.i"
%include "../xps/include/xpsVsi.gen.i"
%include "../xps/include/xpsVxlan.gen.i"
%include "../xps/include/xpsXpImports.gen.i"
%include "../xps/include/xpTypes.gen.i"
%include "../xps/include/xpsUmac.gen.i"

// Vector support
namespace std {
   %template(Uint32Vector) vector<uint32_t>;
}

namespace std {
   %template(Uint8Vector) vector<uint8_t>;
}

namespace std {
   %template(xpIdRangeInfoVec) vector<xpIdRangeInfo*>;
}


%inline %{

    uint8_t* getIpAddrPtr(void* addr) {
        return (uint8_t*)addr;
    }

    ipv4Addr_t* getUint8IpAddrPtr(ipv4Addr_t addr) {
        ipv4Addr_t * tmp = (ipv4Addr_t *)malloc(sizeof(ipv4Addr_t)); + memcpy(tmp, addr, sizeof(ipv4Addr_t));
        return tmp;
    }


    macAddrHigh_t* getUint8macAddrHighPtr(macAddrHigh_t addr) {
        macAddrHigh_t * tmp = (macAddrHigh_t *)malloc(sizeof(macAddrHigh_t)); + memcpy(tmp, addr, sizeof(macAddrHigh_t));
        return tmp;
    }

    void memZero (void *addr, uint32_t blockSize) {
        memset(addr, 0, blockSize);
    }

    extern xpDevConfigStruct devDefaultConfig;
%}

//To find sizeof any data-type
//Format : %sizeof(datatype, name)
//How to use? : To find size of any below defined datatype, use "sizeof_<name>"
//Example : sizeof_uint16, sizeof_xpWcmIaclkeyFieldList
%sizeof(char, char);
%sizeof(bool, bool);
%sizeof(int, int);
%sizeof(float, float);
%sizeof(long, long);
%sizeof(double, double);
%sizeof(int8_t, int8);
%sizeof(uint8_t, uint8);
%sizeof(int16_t, int16);
%sizeof(uint16_t, uint16);
%sizeof(int32_t, int32);
%sizeof(uint32_t, uint32);
%sizeof(int64_t, int64);
%sizeof(uint64_t, uint64);

//Front end files those need to be swigified for packetTrakker

