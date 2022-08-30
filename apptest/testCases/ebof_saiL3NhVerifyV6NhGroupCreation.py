#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'ebof_saiNhVerifyV6NhGroupCreation',
    'description' : 'TC to verify if the V6 NH group is created and programmed properly',
    'ingressPort' : [''],
    'egressPort' : [''],
    'pktAction' : '',
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = ''' 
home
sai
port
sai_set_port_attribute 281474976710660 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710676 SAI_PORT_ATTR_ADMIN_STATE 1

home
sai
virtualrouter
sai_create_virtual_router 9288674231451648 >vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:de:ad:be:ef

back
routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710660 $vr_id >rif_i_prt_01
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710676 $vr_id >rif_i_prt_02

back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_i_prt_01 1 10.01.0.0.0.0.0.0.0.0.0.0.0.0.0.1 00:01:01:01:01:01
sai_create_neighbor_entry 9288674231451648 $rif_i_prt_02 1 20.01.0.0.0.0.0.0.0.0.0.0.0.0.0.1 00:02:02:02:02:02

back
nexthop
sai_create_next_hop 9288674231451648 0 $rif_i_prt_01 SAI_NEXT_HOP_TYPE_IP 1 [10.01.0.0.0.0.0.0.0.0.0.0.0.0.0.1] > nh0
sai_create_next_hop 9288674231451648 0 $rif_i_prt_02 SAI_NEXT_HOP_TYPE_IP 1 [20.01.0.0.0.0.0.0.0.0.0.0.0.0.0.1] > nh1

back
nexthopgroup
sai_create_next_hop_group 9288674231451648 SAI_NEXT_HOP_GROUP_TYPE_ECMP 0 >nhop_group_id
sai_create_next_hop_group_member 9288674231451648 $nh0 $nhop_group_id 0 >nh_group_member_id_2
sai_create_next_hop_group_member 9288674231451648 $nh1 $nhop_group_id 0 >nh_group_member_id_3
sai_get_next_hop_group_attribute $nhop_group_id SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT 0
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
nexthopgroup
sai_remove_next_hop_group_member $nh_group_member_id_2
sai_remove_next_hop_group_member $nh_group_member_id_3
sai_remove_next_hop_group $nhop_group_id
back

nexthop
sai_remove_next_hop $nh0
sai_remove_next_hop $nh1
back

neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_i_prt_01 1 [10.01.0.0.0.0.0.0.0.0.0.0.0.0.0.1]
sai_remove_neighbor_entry 9288674231451648 $rif_i_prt_02 1 [20.01.0.0.0.0.0.0.0.0.0.0.0.0.0.1]
back

routerinterface
sai_remove_router_interface $rif_i_prt_01
sai_remove_router_interface $rif_i_prt_02
back
'''
#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'2',
}
