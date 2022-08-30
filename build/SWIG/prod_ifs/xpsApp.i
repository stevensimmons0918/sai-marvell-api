////////////////////////////////////////
//<product>.i - interface specification for product <product> to Python
////////////////////////////////////////

%include "../device/sdksrc/xp/dm/tableManager/include/xpTableContext.gen.i"
%include "SWIG/xpsApp.top.i"

%include <stdint.i>
%include <std_vector.i>

//common stuff to all Python product interfaces
%include "SWIG/common.i"


%include <cpointer.i>
%include <carrays.i>
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
%array_functions(uint16_t ,arrUint16)
%array_functions(uint8_t, CONSTDATA256_tp)
%array_functions(xpEaclkeyField_t, xpEaclkeyField_arr);
%array_functions(xpsEaclkeyFieldList_t*, xpsEaclkeyFieldList_arr);
%array_functions(xpQueueInfo, xpQueueInfoArr)
%array_functions(xpHashTableContext, xpHashTableContext_arr)
%array_functions(xpDirectTableContext, xpDirectTableContext_arr)
%array_functions(xpIsmeTableContext, xpIsmeTableContext_arr)
%array_functions(xpMatchTableContext, xpMatchTableContext_arr)
%array_functions(xpLpmTableContext, xpLpmTableContext_arr)
%array_functions(xpPrfxTblContext_t, xpPrfxTblContext_arr)
%array_functions(xpNhTableContext, xpNhTblContext_arr)
%array_functions(xpH2Grp, xpH2Grp_arr)
%array_functions(xpH1Grp, xpH1Grp_arr)
%array_functions(xpPortGrp, xpPortGrp_arr)
%array_functions(xpPort_t*, xpPort_arr)
%array_functions(xpTmNodePrio, xpTmNodePrio_arr)
%array_functions(xpQGrp, xpQGrp_arr)
%array_functions(xpSerdesAddr_t, xpSerdesAddr_tp)
%pointer_functions(XP_PROFILE_TYPE, XP_PROFILE_TYPE_P)
%pointer_functions(xpAcmRsltEntry, xpAcmRsltEntryp)
%array_functions(xpHashField,xpHashField_arr)
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
%pointer_functions(xpNhFieldList_t, xpNhFieldList_tp);
%pointer_functions(xpIdAllocatorContext, xpIdAllocatorContextp);
%pointer_functions(xpMplsLabelOper, xpMplsLabelOperp);
%pointer_functions(uint8_t, ipv4Addr_tp)
%pointer_functions(xpsIpTunnelData_t, xpsIpTunnelData_tp)
%pointer_functions(xpsPrivateVlanType_e, xpsPrivateVlanType_ep)
%pointer_functions(xpAcm_t, xpAcm_tp);
%pointer_functions(char, charp);
%pointer_functions(xphRxHdr, xphRxHdr_tp);
%pointer_functions(xpIdRangeInfo, xpIdRangeInfop);
%pointer_functions(xpInsertCmd_t, xpInsertCmd_tp);
%pointer_functions(xpInstruction_t, xpInstruction_tp);
//%pointer_functions(xpInstructionT, xpInstructionTp);
%pointer_functions(xpTblDepthPerIdx, xpTblDepthPerIdxp);
%pointer_functions(xpPortList_t, xpPortList_tp);
%pointer_functions(xpsDevice_t, xpsDevice_tp);
%pointer_functions(xpLagList_t, xpLagListTp);
%pointer_functions(xpVif_t, xpVif_tp);
%pointer_functions(vector<uint32_t>, xpUint32Vecp);
%pointer_functions(xpL2DomainCtx_t, xpL2DomainCtx_tp)
%pointer_functions(xpMulticastVifEntry_t, xpMulticastVifEntry_tp)
%pointer_functions(xpMcastDomainId_t, xpMcastDomainId_tp)
//%pointer_functions(xpMulticastMdtNode_t, xpMulticastMdtNode_tp)
%pointer_functions(xpsMulticastIPv4BridgeEntry_t, xpsMulticastIPv4BridgeEntry_tp)
%pointer_functions(xpsMulticastIPv6BridgeEntry_t, xpsMulticastIPv6BridgeEntry_tp)
%pointer_functions(xpsMulticastIPv4RouteEntry_t, xpsMulticastIPv4RouteEntry_tp)
%pointer_functions(xpsMulticastIPv6RouteEntry_t, xpsMulticastIPv6RouteEntry_tp)
%pointer_functions(xpsMulticastIPv4PimBidirRpfEntry_t, xpsMulticastIPv4PimBidirRpfEntry_tp)
//%pointer_functions(xpsMulticastIPv6PimBidirRpfEntry_t, xpsMulticastIv6PimBidirRpfEntry_tp)
%pointer_functions(xpsMcastDomainId_t, xpsMcastDomainId_tp)
%pointer_functions(xpIdAllocatorContext, xpIdAllocatorContextTp)
%pointer_functions(xpL2EncapData_t, xpL2EncapData_tp)
%pointer_functions(xpIpTnlStartEntry_t,xpIpTnlStartEntry_tp )
%pointer_functions(xpDatapath_t, xpDatapath_tp)
//%pointer_functions(xpOfDatapathConfig_t, xpOfDatapathConfig_tp)
%pointer_functions(xpFlowEntry_t, xpFlowEntry_tp)
//%pointer_functions(xpOfInstructions_t, xpOfInstructions_tp)
//%pointer_functions(xpOfGroupType_t, xpOfGroupType_tp)
//%pointer_functions(xpOfBucket_t, xpOfBucket_tp) 
//%pointer_functions(xpOfGroupStats_t, xpOfGroupStats_tp)
//%pointer_functions(xpOfMeterBand_t, xpOfMeterBand_tp)
//%pointer_functions(xpOfMeterStats_t, xpOfMeterStats_tp)
//%pointer_functions(xpOfMeterBandStats_t, xpOfMeterBandStats_tp)
%pointer_functions(xpMplsTnlTerminationKey_t, xpMplsTnlTerminationKey_tp)
%pointer_functions(xpMplsTnlTerminationData_t, xpMplsTnlTerminationData_tp)
%pointer_functions(xpMplsTnlStartData_t, xpMplsTnlStartData_tp)
%pointer_functions(xpIpTnlStartData_t, xpIpTnlStartData_tp)
%pointer_functions(xpIpTnlTerminationEntry_t, xpIpTnlTerminationEntry_tp)
%pointer_functions(xpIpTnlTerminationKey_t, xpIpTnlTerminationKey_tp)
%pointer_functions(xpIpTnlTerminationData_t, xpIpTnlTerminationData_tp)
%pointer_functions(xpIpTunnelIdData_t, xpIpTunnelIdData_tp)
%pointer_functions(xpPbbTerminationData_t, xpPbbTerminationData_tp)
%pointer_functions(xpPbbStartData_t,xpPbbStartData_tp )
%pointer_functions(xpPbbTerminationKey_t,xpPbbTerminationKey_tp)
%pointer_functions(xpPktCmd_e, xpPktCmd_ep)
%pointer_functions(xpMplsLabelOper, xpMplsLabelOper_tp)
%pointer_functions(xpPortConfig_t, xpPortConfig_tp)
%pointer_functions(xpVlanStgState_e, xpVlanStgState_ep)
%pointer_functions(xpCountMode_e, xpCountMode_ep)
%pointer_functions(xpVlanBridgeMcMode_e, xpVlanBridgeMcMode_ep) 
%pointer_functions(xpVlanRouteMcMode_t, xpVlanRouteMcMode_tp)
%pointer_functions(xpVlanNatMode_e, xpVlanNatMode_ep)
%pointer_functions(xpNh_t, xpNh_tp)
%pointer_functions(xpVlan_t, xpVlan_tp)
%pointer_functions(XP_SE_MRE_SEARCH_COMMAND_T, XP_SE_MRE_SEARCH_COMMAND_Tp)
%pointer_functions(xpTblCtxPerIdx, xpTblCtxPerIdx_tp)
%pointer_functions(xpAgeFifoData, xpAgeFifoData_tp)
%pointer_functions(xpNatEntryKey_t, xpNatEntryKey_tp)
%pointer_functions(xpNatEntryMask_t, xpNatEntryMask_tp)
%pointer_functions(xpNatEntryData_t, xpNatEntryData_tp)
%pointer_functions(xpSeInsPtrs_t, xpSeInsPtrs_tp)
%pointer_functions(xpAqmPflEntry_t, xpAqmPflEntry_tp)
%pointer_functions(xpsVlan_t, xpsVlan_tp)
%pointer_functions(xpsScope_t, xpsScope_tp)
%array_functions(xpsVlan_t*,xpsVlan_tpp);
%pointer_functions(xpsPktCmd_e, xpsPktCmd_ep)
//%pointer_functions(xpsLearningMode_e, xpsLearningMode_ep)
%pointer_functions(xpsCountMode_e, xpsCountMode_ep)
%pointer_functions(xpsInterfaceId_t, xpsInterfaceId_tp)
%pointer_functions(xpsPeg_t, xpsPeg_tp)
%array_functions(xpsInterfaceId_t*,xpsInterfaceId_tpp);
%pointer_functions(xpsStp_t, xpsStp_tp)
%pointer_functions(xpsEgressFilter_t, xpsEgressFilter_tp)
%pointer_functions(xpsDbHandle_t, xpsDbHandle_tp)
%pointer_functions(xpsStpState_e, xpsStpState_ep)
%pointer_functions(xpsFdbEntry_t, xpsFdbEntry_tp)
%pointer_functions(xpsInterfaceInfo_t, xpsInterfaceInfo_tp)
%pointer_functions(xpsInterfaceType_e, xpsInterfaceType_ep)
//%pointer_functions(cWrapXpsStateEntry_t, cWrapXpsStateEntry_tp)
//%pointer_functions(cWrapLdeData, cWrapLdeDatap)
%pointer_functions(xp80LdeMgr, xp80LdeMgrp)
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
%pointer_functions(xpSerdesCtle_t,xpSerdesCtle_tp)
%pointer_functions(xpDiagConfig_t,xpDiagConfig_tp)
%pointer_functions(xpSerdesEyeConfig_t,xpSerdesEyeConfig_tp)
%pointer_functions(xpSerdesEyeData_t,xpSerdesEyeData_tp)
%pointer_functions(xpsPortIntfMap_t,xpsPortIntfMap_tp)
%pointer_functions(BOOL,BOOLp)
%pointer_functions(xpDevice_t,xpDevice_tp)
%pointer_functions(xpPort_t,xpPort_tp)
%pointer_functions(xpScope_t, xpScope_tp)
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
%pointer_functions(xpSerdesTxEq_t,xpSerdesTxEq_tp)
%pointer_functions(xpSerdes_t, xpSerdes_tp)
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
%pointer_functions(xpPolicerResult_t,xpPolicerResult_tp)
%pointer_functions(xpSerdesPllState_t,xpSerdesPllState_tp)
%pointer_functions(xpSerdesPcsFifoClk_t,xpSerdesPcsFifoClk_tp)
%pointer_functions(xpSerdesStatus_t,xpSerdesStatus_tp)
%pointer_functions(xpSerdesInt_t,xpSerdesInt_tp)
%pointer_functions(xpSerdesBsbMode_t,xpSerdesBsbMode_tp)
%pointer_functions(xpSerdesBsbClkSel_t,xpSerdesBsbClkSel_tp)
%pointer_functions(FILE,new_FILEp)
%pointer_functions(xpDevConfigStruct,xpDevConfigStructp)
%pointer_functions(xpVifEntryStruct_t,xpVifEntryStruct_tp)
%pointer_functions(xpPolicerEntry_t,xpPolicerEntry_tp)
%pointer_functions(xpsLagPortIntfList_t, xpsLagPortIntfList_tp)
%pointer_functions(xpQosMapPfl_t, xpQosMapPfl_tp)
%pointer_functions(xpPCSEnableTestMode,xpPCSEnableTestModep)
%pointer_functions(xpPCSDataPattern,xpPCSDataPatternp)
%pointer_functions(xpBackPlaneAbilityModes,xpBackPlaneAbilityModesp)
%pointer_functions(xphTxHdrMetaData,xphTxHdrMetaData_tp)
%pointer_functions(xpEthHdr_t,xpEthHdr_tp)
%pointer_functions(xpCoreClkFreq_t, xpCoreClkFreq_tp)
%pointer_functions(xpBypassMode_e, xpBypassMode_ep)
%pointer_functions(XP_AGE_MODE_T, XP_AGE_MODE_Tp)
%pointer_functions(xpWredMode_e, xpWredMode_ep)
%pointer_functions(xpQGuarThresholdGranularity_e, xpQGuarThresholdGranularity_ep)
%pointer_functions(xpDynThldFraction_e, xpDynThldFraction_ep)
%pointer_functions(xpDynThldCoeff_e, xpDynThldCoeff_ep)
%pointer_functions(xpDynThldOperator_e, xpDynThldOperator_ep)
%pointer_functions(XpBitVector,XpBitVectorp)
%pointer_functions(xpEgressCosMapData_t, xpEgressCosMapData_tp)
%pointer_functions(xpTxqQueuePathToPort_t, xpTxqQueuePathToPort_tp)
%pointer_functions(xpCoppEntryData_t, xpCoppEntryData_tp)
%pointer_functions(xpCpuTypes_e, xpCpuTypes_ep) 

%pointer_functions(xpControlMacEntry, xpControlMacEntryp)
%pointer_functions(xpInsertionEntry, xpInsertionEntryp)
%pointer_functions(xpMdtEntry, xpMdtEntryp)
%pointer_functions(xpPortMaskEntry, xpPortMaskEntryp)
%pointer_functions(xpVifEntry, xpVifEntryp)
%pointer_functions(xpVifTableType_t, xpVifTableType_tp)
%pointer_functions(xpControlMacKeyMask_t,xpControlMacKeyMask_tp)
%pointer_functions(xpControlMacData_t, xpControlMacData_tp)
%pointer_functions(xpMdtMirrorEntryData_t, xpMdtMirrorEntryData_tp)
%pointer_functions(xpMdtRouteEntryData_t, xpMdtRouteEntryData_tp)
%pointer_functions(xpMdtBridgeEntryData_t, xpMdtBridgeEntryData_tp)
%pointer_functions(xpMdtOfBridgeEntryData_t, xpMdtOfBridgeEntryData_tp)
%pointer_functions(xpMdtOfRouteEntryData_t, xpMdtOfRouteEntryData_tp)
%pointer_functions(xpMdtEntryData_t, xpMdtEntryData_tp)
%pointer_functions(xpMacRxFault, xpMacRxFaultp)
%pointer_functions(xpFecMode, xpFecModep)
%pointer_functions(xpRegAccessMode_t, xpRegAccessMode_tp)
%pointer_functions(xpWcmInstrs, xpWcmInstrsp)
%pointer_functions(xpDmaDescDebugEntities, xpDmaDescDebugEntities_tp)
%pointer_functions(xpPolicerStandard_e, xpPolicerStandard_ep)
%pointer_functions(xpPtGlobalState_e, xpPtGlobalState_ep)
%pointer_functions(xpPtPortState_e, xpPtPortState_ep)
%pointer_functions(xpPtMirrorModes_e, xpPtMirrorModes_ep)
%pointer_functions(xpPktTraceMode, xpPktTraceMode_ep)

//TODO fn ptrs
//%constant XP_STATUS xpsFdbGetEntry (xpsDevice_t devId, xpsFdbEntry_t *fdbEntry);
%pythoncallback;
XP_STATUS (*xpsFdbGetEntry) (xpsDevice_t devId, xpsFdbEntry_t *fdbEntry);
%nopythoncallback;
//%ignore xpsFdbGetEntry;

// common stuff to this product interface front-end files
%include "SWIG/xpsApp.gen.i"

// stuff from the files that we want SWIG to ignore
%include "../utils/xps/include/xpsState.src.i"

// stuff from product .cpp files that has to be given to SWIG
%include "SWIG/xpsApp.gen.cpp"


%include "../utils/xp/include/xpLog.gen.i"
%include "../utils/xp/include/xpLogMgr.src.i"
%include "../utils/xp/include/xpLogMgr.gen.i"
%include "../device/sdksrc/xp/cInterface/init/include/xpInit.gen.i"
%include "../device/sdksrc/xp/cInterface/acm/include/xpAcm.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/l2/include/xpLag.gen.i"
%include "../device/sdksrc/xp/cInterface/qos/include/xpShaper.gen.i"
%include "../device/sdksrc/xp/cInterface/packetDriver/include/xpPacketDrv.gen.i"
%include "../device/sdksrc/xp/cInterface/bufferMgr/include/xpBufMgr.gen.i"
%include "../utils/xp/include/xpLoggable.gen.i"
%include "../utils/xps/include/xpsDisplayTables.gen.i"
%include "../utils/xps/include/xpsTcamListShuffling.gen.i"
//%include "../xp/cInterface/acl/include/xpAcl.gen.i"
%include "../utils/xp/include/xpLogModFl.gen.i"
%include "../utils/xp/include/xpLogModPl.gen.i"
%include "../utils/xp/include/xpLogModTl.gen.i"
%include "../utils/xp/include/xpLogModDeviceMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpPrimitiveMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpPLCommon.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpPLCommon.gen.i"
%include "../utils/xp/include/xpTypes.gen.i"
%include "../utils/xp/include/openXpsEnums.gen.i"
%include "../utils/xp/include/openXpsTypes.gen.i"
%include "../utils/xp/include/openXpsTypes.src.i"
%include "../device/sdksrc/xp/xpDiag/debug/include/xpMgmtDebug.gen.i"
%include "../device/sdksrc/xp/xpDiag/debug/include/xpBdkDebug.gen.i"
%include "../device/sdksrc/xp/xpDiag/debug/include/xpMacA0Debug.gen.i"
%include "../device/sdksrc/xp/xpDiag/debug/include/xpMacB0Debug.gen.i"
%include "../device/sdksrc/xp/xpDiag/debug/include/xpMacDebug.gen.i"
%include "../utils/xp/include/xpEnums.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/include/xpPl.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/include/xpAclPl.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/include/xpL2Pl.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/include/xpMcastPl.gen.i"
%include "../device/sdksrc/xp/pl/include/xpQosPl.gen.i"
%include "../device/sdksrc/xp/pl/include/xpIfPl.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/include/xpCoppPl.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/include/xpL3Pl.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/include/xpTunnelPl.gen.i"
%include "../device/sdksrc/xp/pl/include/xpAcmPl.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/acl/include/xpAclEntryFormat.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/openflow/include/xpOpenFlowEntryFormat.gen.i"
%include "../device/sdksrc/xp/dm/devices/common/config/include/xpAcmConfig.gen.i"
%include "../device/sdksrc/xp/dm/devices/include/xpDataPathMgr.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/common/include/xp80DataPathMgr.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/common/include/xp80TxqMgr.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/common/include/xp80MgmtBlockMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsAcm.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsAging.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsCopp.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsFdb.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsIacl.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsInit.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsInterface.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsL3.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsLag.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsMac.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsOpenflow.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsPacketDrv.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsPolicer.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsPort.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsQos.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsStp.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/openXpsVlan.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsInternal.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsCommon.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsPolicer.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsAcl.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/acl/include/xpEgressAclMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsAcm.gen.i"
//%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsDebug.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsEnums.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsFdb.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsInit.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsInterface.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsInterface.src.i"
//%includ "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsL2Multicast.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsMulticast.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsAging.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsL3.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsVpnGre.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsNhGrp.gen.i"
//%includ "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsL3Multicast.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsLag.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsMac.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsMirror.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsMpls.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsMtuProfile.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsLink.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsSerdes.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsSerdes.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsNat.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsPacketDrv.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsPort.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsQos.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsGlobalSwitchControl.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsPacketTrakker.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsCopp.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xps8021Br.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsPtp.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsInt.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsGlobalSwitchControl.gen.i"
%include "../utils/xps/include/xpsRBTree.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsSflow.gen.i"
%include "../utils/xps/include/xpsState.gen.i"
%include "../utils/xps/include/xpsUtil.gen.i"
%include "../pipeline-profiles/xpDefault/demo/common/include/xpAppUtil.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/common/include/xp80LdeMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsStp.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsEgressFilter.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsTunnel.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsVlan.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsIpGre.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsVxlan.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsIpinIp.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsNvgre.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsVxlan.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsGeneve.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsErspanGre.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsScope.gen.i"
%include "../device/sdksrc/xp/fl/linkManager/include/xpLinkManager.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpMplsLabelMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpQmapEntry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpMplsLabelMgr.gen.i"
%include "../device/sdksrc/xp/fl/linkManager/include/xpMac.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpFdbMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpFdbMgr.gen.i"
%include "../utils/xp/include/xpIdAllocatorContext.gen.i"
%include "../device/sdksrc/xp/cInterface/sal/include/xpBufferMgr.gen.i"
%include "../device/sdksrc/xp/system/sal/bufferMgr/include/xpBufferManager.gen.i"
%include "../device/sdksrc/xp/system/sal/include/xpSal.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpPortVlanIvifMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpPortVlanIvifMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpReasonCodeMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpReasonCodeMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpAcmRsltMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpBdMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpBdMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpPortConfigMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpPortMaskEntry.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpEgressFilterMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpMplsTunnelMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpMplsTunnelMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpTrunkResolutionMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpAcmBankMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv6HostMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv6HostMgr.gen.i"
%include "../device/sdksrc/xp/dm/tableManager/include/xpTableManager.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpMdtMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpMdtMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIaclMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIaclMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/acl/include/xpAclEntryFormat.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpEaclMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpEaclMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv4BridgeMcMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv4PIMBiDirRPFMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv6PIMBiDirRPFMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv4PIMBiDirRPFMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv6PIMBiDirRPFMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv4BridgeMcMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv4RouteMcMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv4RouteMcMgr.gen.i"
//%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv4RouteMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv4HostMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv4HostMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpCoppMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpCoppMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpNATIpv4Mgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpNATIpv4Mgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv6BridgeMcMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv6BridgeMcMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv6RouteMcMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv6RouteMcMgr.gen.i"
//%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpv6RouteMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpNhMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpNhMgr.gen.i"
%include "../device/sdksrc/xp/dm/tableManager/include/xpTableProfile.gen.i"
%include "../device/sdksrc/xp/dm/devices/common/config/include/xpDevConfig.gen.i"
%include "../pipeline-profiles/xpDefault/profiles/devices/xp80/parser/include/xp80ParserConfigData.gen.i"
%include "../device/sdksrc/xp/cInterface/debug/include/xpDebug.gen.i"
%include "../pipeline-profiles/xpDefault/demo/xpsApp/include/xpsApp.gen.i"
%include "../pipeline-profiles/xpDefault/demo/common/py/xpPyEmbed.gen.i"
%include "../pipeline-profiles/xpDefault/demo/common/include/xpAppUtil.gen.i"
%include "../pipeline-profiles/xpDefault/demo/xpsApp/l2/include/xpsAppL2.gen.i"
%include "../pipeline-profiles/xpDefault/demo/xpsApp/include/xpsApp.gen.i"
%include "../device/sdksrc/xp/cInterface/driverWrapper/include/xpDrvWrapper.gen.i"
%include "../device/sdksrc/xp/cInterface/sal/include/xpSalInterface.gen.i"
%include "../device/sdksrc/xp/xpDiag/debug/include/xpMgmtDebug.gen.i"
%include "../pipeline-profiles/xpDefault/demo/config/include/xpAppConfig.gen.i"
%include "../device/sdksrc/xp/dm/devices/common/include/xpParserDefinitions.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpTunnelIvifMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpTunnelIvifMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpLocalSidMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpLocalSidMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpLocalVTEPMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpTunnelIdMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpVifMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpVifMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpQosMapMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpHdrModificationMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpHdrModificationMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpInsertionMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpControlMacMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpRouteLpmMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpAqmPflEntry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpAqmQPflEntry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpDwrrEntry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpEqCfgEntry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpFastShapersEntry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpPktLimThresholdEntry.gen.i"
//%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpPfcMapCfgEntry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpQmapEntry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpQmapTblIndexEntry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpShapersEntry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpTmCfgH1Entry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpTmCfgH2Entry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpTmCfgPipeEntry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpTmCfgPortEntry.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpAqmPflMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpAqmQPflMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpDwrrMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpShapersMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpTmCfgMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpEqCfgMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpH1CounterMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpH2CounterMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpPfcCountersMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpPktLimThresholdMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpPortCounterMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/xpTxqCounterEntry.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpQCounterMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpQmappingMgr.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpTmCfgMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpEgressCosMapMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpEgressCosMapMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpBdMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpEgressBdMgr.gen.i"
%include "../device/sdksrc/xp/fl/allocator/include/xpAllocatorMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/acl/include/xpAclMgr.gen.i"
%include "../device/sdksrc/xp/fl/acm/include/xpAcmMgr.gen.i"
%include "../device/sdksrc/xp/fl/profileMgr/include/xpProfileMgr.src.i"
%include "../device/sdksrc/xp/fl/profileMgr/include/xpProfileMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l2/include/xpL2LearnMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l2/include/xpL2LearnMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l2/include/xpStpMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l2/include/xpL2DomainMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/of/include/xpOfMgr.src.i"
//%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l3/include/xpIpRouterMacMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l3/include/xpIpv4RouteMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l3/include/xpIpv4RouteMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l3/include/xpIpv6RouteMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l3/include/xpIpv6RouteMgr.gen.i" 
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l3/include/xpIpHostMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l3/include/xpIpHostMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l3/include/xpIpNhMgr.gen.i"
//%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l3/include/xpIpNhMgr.src.i"
%include "../device/sdksrc/xp/fl/interface/include/xpCtrlMacMgr.src.i"
%include "../device/sdksrc/xp/fl/interface/include/xpCtrlMacMgr.gen.i"
//%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/l3/include/xpIpv6RouteMgr.gen.i"
%include "../device/sdksrc/xp/fl/interface/include/xpMtuProfileMgr.gen.i"
%include "../device/sdksrc/xp/fl/age/include/xpAgeMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/tunnel/include/xpTunnelMgr.gen.i"
%include "../device/sdksrc/xp/fl/debug/include/xpDebugMgr.gen.i"
%include "../device/sdksrc/xp/fl/qos/include/xpFlowControlMgr.gen.i"
%include "../device/sdksrc/xp/fl/qos/include/xpShaperMgr.gen.i"
%include "../device/sdksrc/xp/fl/qos/include/xpAqmMgr.gen.i"
%include "../device/sdksrc/xp/dm/devices/common/hwShadow/include/xpDalFactory.gen.i"
%include "../device/sdksrc/xp/dm/devices/common/hwShadow/include/xpDal.gen.i"
%include "../device/sdksrc/xp/dm/devices/common/hwShadow/include/xpDalDbg.gen.i"
%include "../device/sdksrc/xp/fl/acm/include/xpAcmMgr.gen.i"
%include "../device/sdksrc/xp/fl/qos/include/xpSchedulerMgr.gen.i"
%include "../device/sdksrc/xp/fl/qos/include/xpQueueCounterMgr.gen.i"
%include "../device/sdksrc/xp/fl/qos/include/xpCpuStormControlMgr.gen.i"
%include "../device/sdksrc/xp/fl/qos/include/xpPortIngressQosMgr.gen.i"
%include "../device/sdksrc/xp/fl/qos/include/xpEgressQosMapMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/copp/include/xpControlPlanePolicingMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/int/include/xpIntMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/nat/include/xpNatMgr.gen.i"
%include "../device/sdksrc/xp/fl/interface/include/xpInterfaceMgr.gen.i"
%include "../device/sdksrc/xp/fl/init/include/xpInitMgr.gen.i"
%include "../device/sdksrc/xp/fl/interface/include/xpInterfaceMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/interface/include/xpInterface.gen.i"
%include "../device/sdksrc/xp/cInterface/interruptManager/include/xpInterruptMgr.gen.i"
%include "../device/sdksrc/xp/system/hal/interruptManager/include/xpInterruptManager.gen.i"
%include "../device/sdksrc/xp/dm/include/xpDeviceMgr.gen.i"
%include "../device/sdksrc/xp/dm/include/xpDeviceMgr.src.i"
//%include "../device/sdksrc/xp/system/hal/driverWrapper/include/xpDriverWrapper.gen.i"
//%include "../device/sdksrc/xp/system/hal/driverWrapper/include/xpDriverWrapper.src.i"
%include "../device/sdksrc/xp/dm/devices/include/xpDevice.gen.i"
%include "../pipeline-profiles/xpDefault/profiles/devices/xp80/range/include/xpDefRangeProfile1Pipe.gen.i"
%include "../pipeline-profiles/xpDefault/profiles/devices/xp80/range/include/xpDefRangeProfile2Pipe.gen.i"
%include "../pipeline-profiles/xpDefault/profiles/devices/xp80/common/config/include/xp80UrwSinglePipeConfigData.gen.i"
%include "../pipeline-profiles/xpDefault/profiles/devices/xp80/common/config/include/xp80UrwMultiPipeConfigData.gen.i"
%include "../pipeline-profiles/xpDefault/profiles/devices/xp80/common/config/include/xp80UrwCommonConfigData.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/profiles/urw/include/xpHdrMgrProfile.gen.i"
%include "../pipeline-profiles/xpDefault/profiles/devices/xp80/urw/include/xpDefaultHdrMgrProfile.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/common/include/xp80Device.gen.i"
%include "../device/sdksrc/xp/dm/devices/include/xpIsmeMgr.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/common/include/xp80IsmeMgr.gen.i"
%include "../device/sdksrc/xp/dm/devices/include/xpDeviceFactory.gen.i"
%include "../device/sdksrc/xp/dm/devices/include/xpHwAccessMgr.gen.i"
//%include "../device/sdksrc/xp/dm/devices/common/config/include/xpDevConfig.gen.cpp"
//%include "../whitemodel/exec/include/xpWmTop.src.i"
//%include "../whitemodel/exec/include/xpWmTop.gen.i"
%include "../whitemodel/common/ipc_srv/xpWmIpcSrv.gen.i"
%include "../device/sdksrc/xp/dm/devices/include/xpMreMgr.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/common/include/xp80MreMgr.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/common/include/xp80SeMgr.gen.i"
%include "../device/sdksrc/xp/dm/devices/include/xpSeMgr.gen.i"
%include "../device/sdksrc/xp/dm/devices/include/xpDevice.gen.i"
%include "../device/sdksrc/xp/dm/devices/include/xpParser.gen.i"
%include "../device/sdksrc/xp/dm/devices/common/include/xpParserMgr.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/common/include/xp80ParserMgr.src.i"
%include "../device/sdksrc/xp/dm/devices/xp80/common/include/xp80ParserMgr.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/A0/include/xp80Profile.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/common/include/xp80MmeMgr.gen.i"
%include "../device/sdksrc/xp/system/mgmt/ipc/include/xpIpcInternal.gen.i"
%include "../device/sdksrc/xp/cInterface/linkManager/include/xpLinkMgr.gen.i"
%include "../device/sdksrc/xp/xpDiag/include/xpDiagMgr.gen.i"
%include "../device/sdksrc/xp/xpDiag/include/xpScpuDiag.gen.i"
%include "../device/sdksrc/xp/dm/devices/common/include/xpTxqMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xps/include/xpsOpenflow.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/fl/of/include/xpOfMgr.gen.i"
%include "../utils/xp/include/XpBitVector.gen.i"
%include "../device/sdksrc/xp/dm/tableManager/include/xpDirectAccessTable.gen.i"
%include "../device/sdksrc/xp/dm/tableManager/include/xpTable.gen.i"
%include "../device/sdksrc/xp/fl/globalSwitchControl/include/xpGlobalSwitchControlMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpEgressBdMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIpRouteLpmMgr.src.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpPortConfigMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpAcmRsltMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpControlMacMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpDwrrMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpEgressFilterMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpEqCfgMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpH1CounterMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpH2CounterMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpPfcCountersMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpPktLimThresholdMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpPortCounterMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpQCounterMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpQmappingMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpQosMapMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpShapersMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpTmCfgMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpTrunkResolutionMgr.src.i"


//B0 files
%include "../device/sdksrc/xp/dm/devices/xp80/B0/include/xp80B0Device.gen.i"
%include "../device/sdksrc/xp/cInterface/pl/include/xpControlMac.gen.i"
%include "../device/sdksrc/xp/cInterface/pl/include/xpEgressFilter.gen.i"
%include "../device/sdksrc/xp/cInterface/pl/include/xpHdrModification.gen.i"
%include "../device/sdksrc/xp/cInterface/pl/include/xpMdt.gen.i"
%include "../device/sdksrc/xp/cInterface/pl/include/xpTrunkResolution.gen.i"
%include "../device/sdksrc/xp/cInterface/pl/include/xpVif.gen.i"
%include "../device/sdksrc/xp/pl/managers/include/xpAcmBankMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpAqmPflMgr.src.i"
%include "../device/sdksrc/xp/pl/managers/include/xpAqmQPflMgr.src.i"
%include "../device/sdksrc/xp/cInterface/pl/include/xpIp4RouteLpm.gen.i"
%include "../device/sdksrc/xp/cInterface/pl/include/xpIp6RouteLpm.gen.i"
%include "../device/sdksrc/xp/dm/devices/xp80/common/include/xp80UrwMgr.gen.i"

//Common
%include "../device/sdksrc/xp/dm/tableManager/include/xpRegEnums.gen.i"
%include "../device/sdksrc/xp/dm/tableManager/include/xpRegManager.gen.i"

// XPIPC
//%include "../device/sdksrc/xp/system/mgmt/ipc/include/xpIpc.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/ipc/include/xpIpcInterface.gen.i"

//XP70 files
%include "../pipeline-profiles/xpDefault/sdksrc/xp/pl/managers/include/xpIaclWcmMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/cInterface/fl/acl/include/xpWcmIacl.gen.i"


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
namespace std {
   %template(tblCtxPerIdxPointVec) vector<xpTblCtxPerIdx*>;
}

namespace std {
   %template(xpTblDepthPerIdxVec) vector<xpTblDepthPerIdx*>;
}

%inline %{
    xpIaclWcmMgr* getIaclWcmMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpIaclWcmMgr*>(base);
    }

    xpFdbMgr* getFdbMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpFdbMgr*>(base);
    }

    xpMplsLabelMgr* getMplsLabelMgrPtr(xpPrimitiveMgr* base)    {
        return static_cast<xpMplsLabelMgr*>(base);
    }

    xpReasonCodeMgr* getReasonCodeMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpReasonCodeMgr*>(base);
    }
    xpAcmRsltMgr* getAcmRsltMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpAcmRsltMgr*>(base);
    }
    xpPortVlanIvifMgr* getPortVlanIvifMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpPortVlanIvifMgr*>(base);
    }
    xpBdMgr* getBdMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpBdMgr*>(base);
    }
    xpPortConfigMgr* getPortConfigMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpPortConfigMgr*>(base);
    }
    xpEgressFilterMgr* getEgressFilterMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpEgressFilterMgr*>(base);
    }
    xpTrunkResolutionMgr* getTrunkResolutionMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpTrunkResolutionMgr*>(base);
    }
    xpIpv6HostMgr* getIpv6HostMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpIpv6HostMgr*>(base);
    }
    xpIpv6RouteMcMgr* getIpv6RouteMcMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpIpv6RouteMcMgr*>(base);
    }
    xpNhMgr* getNhMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpNhMgr*>(base);
    }
    xpIpv4BridgeMcMgr* getIpv4BridgeMcMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpIpv4BridgeMcMgr*>(base);
    }
    xpMdtMgr* getMdtMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpMdtMgr*>(base);
    }
    xpIpv4RouteMcMgr* getIpv4RouteMcMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpIpv4RouteMcMgr*>(base);
    }
//    xpIpv4RouteMgr* getIpv4RouteMgrPtr(xpPrimitiveMgr* base) {
//        return static_cast<xpIpv4RouteMgr*>(base);
//    }
//    xpIpv6RouteMgr* getIpv6RouteMgrPtr(xpPrimitiveMgr* base) {
//        return static_cast<xpIpv6RouteMgr*>(base);
//    }
    xpIpv4HostMgr* getIpv4HostMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpIpv4HostMgr*>(base);
    }
    xpCoppMgr* getCoppMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpCoppMgr*>(base);
    }
    xpEgressCosMapMgr* getEgressCosMapMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpEgressCosMapMgr*>(base);
    }
    xpNATIpv4Mgr* getNATIpv4MgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpNATIpv4Mgr*>(base);
    }
    xpIpv6BridgeMcMgr* getIpv6BridgeMcMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpIpv6BridgeMcMgr*>(base);
    }
    xpVifMgr* getVifMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpVifMgr*>(base);
    }
    xpTunnelIvifMgr* getTnlTablePtr(xpPrimitiveMgr* base) {
        return static_cast<xpTunnelIvifMgr*>(base);
    }
    xpLocalSidMgr* getLocalSidTablePtr(xpPrimitiveMgr* base) {
        return static_cast<xpLocalSidMgr*>(base);
     }
    xpHdrModificationMgr* getHdrModificationMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpHdrModificationMgr*>(base);
    }
    xpQosMapMgr* getQosMapMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpQosMapMgr*>(base);
    }
    xpMplsTunnelMgr* getMplsTunnelMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpMplsTunnelMgr*>(base);
    }
    xpInsertionMgr* getInsertionMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpInsertionMgr*>(base);
    }
    xpControlMacMgr* getControlMacMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpControlMacMgr*>(base);
    }
    xpAcmBankMgr* getAcmBankMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpAcmBankMgr*>(base);
    }
    xpIpv4PIMBiDirRPFMgr* getIpv4PIMBiDirRPFMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpIpv4PIMBiDirRPFMgr*>(base);
    }
    xpIpv6PIMBiDirRPFMgr* getIpv6PIMBiDirRPFMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpIpv6PIMBiDirRPFMgr*>(base);
    }
    xpIpRouteLpmMgr* getIpv4RouteMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpIpRouteLpmMgr*>(base);
    }
    xpIpRouteLpmMgr* getIpv6RouteMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpIpRouteLpmMgr*>(base);
    }

    xpIaclMgr* getIaclMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpIaclMgr*>(base);
    }

	xpEaclMgr* getEaclMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpEaclMgr*>(base);
    }

    xpEqCfgMgr* getEqCfgMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpEqCfgMgr*>(base);
    } 

    xpAqmQPflMgr* getAqmQPflMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpAqmQPflMgr*>(base);
    } 

    xpAqmPflMgr* getAqmPflMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpAqmPflMgr*>(base);
    } 

    xpPfcCountersMgr* getPfcCountersMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpPfcCountersMgr*>(base);
    }

    xpH1CounterMgr* getH1CounterMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpH1CounterMgr*>(base);
    } 

    xpH2CounterMgr* getH2CounterMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpH2CounterMgr*>(base);
    } 

    xpPktLimThresholdMgr* getPktLimThresholdMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpPktLimThresholdMgr*>(base);
    }
 

    xpPortCounterMgr* getPortCounterMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpPortCounterMgr*>(base);
    } 

    xpQmappingMgr* getQmappingMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpQmappingMgr*>(base);
    } 

    xpQCounterMgr* getQCounterMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpQCounterMgr*>(base);
    }
    
    xpEgressBdMgr* getEgressBdMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpEgressBdMgr*>(base);
    }


    xpDwrrMgr* getDwrrMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpDwrrMgr*>(base);
    } 
    xpTmCfgMgr* getTmCfgMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpTmCfgMgr*>(base);
    }
    xpShapersMgr* getShapersMgrPtr(xpPrimitiveMgr* base) {
        return static_cast<xpShapersMgr*>(base);
    }
    xp80ParserMgr* getParserMgrPtr(xpParser* base) {
        return static_cast<xp80ParserMgr*>(base);
    }
    
    xpTableContext* getTableContextPtr(xpHashTableContext* derived) {
        return static_cast<xpTableContext*>(derived);
    }

    xpTableContext* getTableContextPtr(xpDirectTableContext* derived) {
        return static_cast<xpTableContext*>(derived);
    }
  

    xpTableContext* getTableContextPtr(xpMatchTableContext* derived) {
        return static_cast<xpTableContext*>(derived);
    }

    xpTableContext* getTableContextPtr(xpLpmTableContext* derived) {
        return static_cast<xpTableContext*>(derived);
    }

    xpTableContext* getTableContextPtr(xpIsmeTableContext* derived) {
        return static_cast<xpTableContext*>(derived);
    }

    xpTableContext** getHashTableContextPtr(xpHashTableContext* derived) {
        xpHashTableContext **tmp= new  xpHashTableContext*;
        *tmp = derived;
        return (xpTableContext**)tmp;
    }

    xpTableContext** getDirectTableContextPtr(xpDirectTableContext* derived) {
        xpDirectTableContext **tmp = new xpDirectTableContext*;
        *tmp = derived;
        return (xpTableContext**)tmp;
    }

    xpTableContext** getIsmeTableContextPtr(xpIsmeTableContext* derived) {
        xpIsmeTableContext **tmp = new xpIsmeTableContext*;
        *tmp = derived;
        return (xpTableContext**)tmp;
    }

    xpTableContext** getMatchTableContextPtr(xpMatchTableContext* derived) {
        xpMatchTableContext **tmp = new xpMatchTableContext*;
        *tmp = derived;
        return (xpTableContext**)tmp;
    }

    uint8_t* getIpAddrPtr(void* addr) {
        return (uint8_t*)addr;
    }

    ipv4Addr_t* getUint8IpAddrPtr(ipv4Addr_t addr) {
        ipv4Addr_t * tmp = (ipv4Addr_t *)malloc(sizeof(ipv4Addr_t)); + memcpy(tmp, addr, sizeof(ipv4Addr_t)); 
        return tmp;
    }

    xpTableContext** getLpmTableContextPtr(xpLpmTableContext* derived) {
        xpLpmTableContext **tmp = new xpLpmTableContext*;
        *tmp = derived;
        return (xpTableContext**)tmp;
    }
    
    xpTableContext** getNhTblContextPtr(xpNhTableContext* derived) {
        xpNhTableContext**tmp = new xpNhTableContext*;
        *tmp = derived;
        return (xpTableContext**)tmp;
    }

    xpDirectAccessTable* getDirectTablePtr(xpTable* base) {
        return static_cast<xpDirectAccessTable*>(base);
    }

    uint8_t* getxp80ProfilePtr(xp80Profile *derived)
    {
        return (uint8_t*)derived;
    }
 
    xp80MgmtBlockMgr* getXp80MgmtBlockMgrPtr(xpMgmtBlockMgr* base) {
        return static_cast<xp80MgmtBlockMgr*>(base);
    }

    macAddrHigh_t* getUint8macAddrHighPtr(macAddrHigh_t addr) {
        macAddrHigh_t * tmp = (macAddrHigh_t *)malloc(sizeof(macAddrHigh_t)); + memcpy(tmp, addr, sizeof(macAddrHigh_t)); 
        return tmp;
    }

    extern xpDevConfigStruct devDefaultConfig;
    extern xpAppConfig xpAppConf;
%}



//Front end files those need to be swigified for packetTrakker
%include "../device/sdksrc/xp/pl/managers/include/xpPacketTrakkerRegMgr.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/packetTrakker/xpPacketTrakkerCookieEntry.gen.i"
%include "../pipeline-profiles/xpDefault/sdksrc/xp/entryFormats/include/packetTrakker/xpPacketTrakkerEventEntry.gen.i"

