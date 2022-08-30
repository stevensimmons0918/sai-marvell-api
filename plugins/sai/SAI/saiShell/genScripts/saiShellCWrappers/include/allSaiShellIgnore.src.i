%ignore xpSaiSamplePacketSessionCreate(sai_object_id_t* session_id,uint32_t attr_count,const sai_attribute_t *attr_list);
%ignore sai_dbg_generate_dump(const char *dump_file_name);

%ignore xpSaiFdbNotify(xpSaiFdbChangeNotif_t *notif);

%ignore xpSaiDumpSoftFwdInfo();
%ignore xpSaiGetPktDetails (void *inPkt, uint16_t len, uint8_t *pOutTtl, bool *pOutIsArp, bool *pOutIsVlanUntagged);

%ignore xpSaiNeighborUpdateXpsHostEntry(xpsL3HostEntry_t *xpsHostEntry);
%ignore xpSaiNeighborResolvePktCmd(xpsPktCmd_e neighPktCmd, xpsPktCmd_e fdbPktCmd);

%ignore xpSaiGetLagAttributes(sai_object_id_t lag_id, uint32_t attr_count, sai_attribute_t *attr_list);

%ignore xpSaiNextHopUsesNeighbor(xpSaiNextHopEntry_t *nhEntry, const sai_neighbor_entry_t *neighEntry);

%ignore xpSaiFdbInitLock();
%ignore xpSaiFdbDeinitLock();
%ignore xpSaiFdbLock();
%ignore xpSaiFdbUnlock();
%ignore xpSaiNextHopInitLock();
%ignore xpSaiNextHopDeinitLock();
%ignore xpSaiNextHopLock();
%ignore xpSaiNextHopUnlock();
%ignore xpSaiNeighborInitLock();
%ignore xpSaiNeighborDeinitLock();
%ignore xpSaiNeighborLock();
%ignore xpSaiNeighborUnlock();
%ignore xpSaiNextHopGroupGetSaiId(uint32_t xpsNhGrpId, sai_object_id_t *saiNhGrpId);
%ignore xpSaiNextHopGroupGetXpsId(sai_object_id_t saiNhGrpId, uint32_t *xpsNhGrpId);
%ignore xpSaiGetNextHopGroupAttrNextHopMemberList(uint32_t nhGrpId, sai_attribute_value_t* pValue);
%ignore xpSaiNextHopGroupMemberSetAttributeWeight (sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t weight);
%ignore xpSaiNextHopGroupMemberGetAttributeWeight(sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t *weight);
%ignore xpSaiGetNextHopGroupAttrNextHopCount(uint32_t nhGrpId, sai_attribute_value_t* pValue);
%ignore xpSaiGetNextHopGroupAttrType(uint32_t nhGrpId, sai_attribute_value_t* pValue);
%ignore xpSaiCreateNextHopGroup (sai_object_id_t *next_hop_group_id, sai_object_id_t switch_id, uint32_t attr_count, const sai_attribute_t *attr_list);
%ignore xpSaiRemoveNextHopGroup (sai_object_id_t next_hop_group_id);
%ignore xpSaiSetNextHopGroupAttribute (sai_object_id_t next_hop_group_id, const sai_attribute_t *attr);
%ignore xpSaiGetNextHopGroupAttribute(sai_object_id_t saiNhGrpId, sai_attribute_t *attr, uint32_t attr_index);
%ignore xpSaiGetNextHopGroupAttributes (sai_object_id_t next_hop_group_id, uint32_t attr_count, sai_attribute_t *attr_list);
%ignore xpSaiCreateNextHopGroupMember (sai_object_id_t* next_hop_group_member_id, sai_object_id_t switch_id, uint32_t attr_count, const sai_attribute_t *attr_list);
%ignore xpSaiRemoveNextHopGroupMember (sai_object_id_t next_hop_group_member_id);
%ignore xpSaiSetNextHopGroupMemberAttribute (sai_object_id_t next_hop_group_member_id, const sai_attribute_t *attr);
%ignore xpSaiGetNextHopGroupMemberAttributes (sai_object_id_t next_hop_group_member_id, uint32_t attr_count, sai_attribute_t *attr_list);
%ignore xpSaiNextHopGroupDestroyXpsNextHopGroup(uint32_t xpsNhGrpId);
%ignore xpSaiNextHopGroupAddXpsNextHop(sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t weight);
%ignore xpSaiNextHopGroupRemoveXpsNextHop(sai_object_id_t nhGrpId, sai_object_id_t nhId);
%ignore xpSaiNextHopGroupSetXpsNextHopWeight(sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t weight);
%ignore xpSaiNextHopGroupGetXpsNextHopWeight(sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t *weight);
%ignore xpSaiNextHopGroupContainsNextHop(sai_object_id_t nhGrpId, sai_object_id_t nhId);
%ignore xpSaiNextHopGroupMemberCreateId(sai_object_id_t nhGrpId, sai_object_id_t nhId, sai_object_id_t *nhMemberId);
%ignore xpSaiNextHopGroupGetMemberInfo(sai_object_id_t nhMemberId, xpSaiNextHopGroupMemberInfo_t *info);
%ignore xpSaiNextHopGroupInfoKeyCompare(void* key1, void* key2);
%ignore gSaiNextHopGroupInfoDbHndl;

%ignore xpSaiRouterInterfaceGetVrfId(sai_object_id_t rif_id, uint32_t *vrfId);

%ignore xpSaiQueueGetQueueIdList(xpsScope_t scopeId, uint32_t *queueId);

%ignore xpSaiPortGetXpsIntfId(sai_object_id_t port, xpsInterfaceId_t *intfId);

%ignore xpSaiCountPolicerObjects(uint32_t *count);
%ignore xpSaiMaxCountPolicerAttribute(uint32_t *count);
%ignore xpSaiGetPolicerObjectList(uint32_t object_count, sai_object_key_t *object_list);

%ignore xpSaiMaxCountQosMapsAttribute(uint32_t *count);
%ignore xpSaiGetQosMapsObjectList(uint32_t object_count, sai_object_key_t *object_list);
%ignore xpSaiCountQosMapsObjects(uint32_t *count);


%ignore xpSaiConvertNextHopGroupOid(sai_object_id_t next_hop_group_id, uint32_t* pNhGrpId);

%ignore xpSaiNeighborGetMacDa(sai_neighbor_entry_t *neighborKey, macAddr_t *macDa);
%ignore xpSaiNeighborGetXpsEgrIntfId(sai_neighbor_entry_t *neighborKey, xpsInterfaceId_t *egressIntfId);
%ignore xpSaiNeighborIsResolved(sai_neighbor_entry_t *neighborKey);
%ignore xpSaiNeighborGetXpsPktCmd(sai_neighbor_entry_t *neighborKey, xpsPktCmd_e *pktCmd);
%ignore xpSaiNeighborGetXpsNhData(sai_neighbor_entry_t *neighborKey, macAddr_t *macDa, xpsInterfaceId_t *egressIntfId, xpsPktCmd_e *pktCmd);
%ignore xpSaiNeighborDeInit(xpsDevice_t xpSaiDevId);


%ignore xpSaiHostInterfaceNetdevInit(void);
%ignore xpSaiHostInterfaceNetdevDeInit(void);

%ignore xpSaiUpdatePbrNextHopGroup(sai_object_id_t nhGroupOid);
%ignore xpSaiIsPbrNxtHopGroupExist(sai_object_id_t nhGroupId, uint8_t *IsPbrExist);
