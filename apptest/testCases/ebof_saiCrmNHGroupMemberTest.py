#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'ebof_saiCrmNHGroupMemberTest',
    'description' : 'TC to verify the CRM count for ECMP group member (NH group member) entries',
    'ingressPort' : [''],
    'egressPort' : [''],
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
xps
fdb
fdb_set_aging_time 0 60
home
sai
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_MEMBER_ENTRY 0 > max_count
back
virtualrouter
sai_create_virtual_router 9288674231451648 > vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:de:ad:be:ef
back
routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710664 $vr_id > rif_i_prt_01
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710668 $vr_id > rif_i_prt_02
back
nexthop
sai_create_next_hop 9288674231451648 0 $rif_i_prt_01 SAI_NEXT_HOP_TYPE_IP 0 [110.110.110.0] > nh0
sai_create_next_hop 9288674231451648 0 $rif_i_prt_01 SAI_NEXT_HOP_TYPE_IP 0 [220.220.220.0] > nh1
back
nexthopgroup
sai_create_next_hop_group 9288674231451648 0 0 > nhgroup0
sai_create_next_hop_group_member 9288674231451648 $nh0 $nhgroup0 0 > nhgroup_member0
sai_create_next_hop_group_member 9288674231451648 $nh1 $nhgroup0 0 > nhgroup_member1
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_MEMBER_ENTRY 0
back
nexthopgroup
sai_remove_next_hop_group_member $nhgroup_member0
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_MEMBER_ENTRY 0
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
nexthopgroup
sai_remove_next_hop_group_member $nhgroup_member1
sai_remove_next_hop_group $nhgroup0
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_NEXT_HOP_GROUP_MEMBER_ENTRY 0
back
nexthop
sai_remove_next_hop $nh0
sai_remove_next_hop $nh1
back
routerinterface
sai_remove_router_interface $rif_i_prt_01
sai_remove_router_interface $rif_i_prt_02
back
virtualrouter
sai_remove_virtual_router $vr_id
home
'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$max_count -1'
}

