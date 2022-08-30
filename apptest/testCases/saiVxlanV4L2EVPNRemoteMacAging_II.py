#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiVxlanV4L2EVPNRemoteMacAging_II',
    'description' : 'Verify L2 EVPN Remote MAC adding (part 2)',
    'ingressPort' : [''],
    'egressPort' : [''],
    'count' : 1,             # expected data count
    'sleep_time' : 60
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY 0
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
vlan
sai_remove_vlan_member $vmem30
sai_remove_vlan_member $vmem29
sai_remove_vlan_member $vmem1
back
bridge
sai_remove_bridge_port $brgport
back
tunnel
sai_remove_tunnel_term_table_entry $tuntermId
sai_remove_tunnel $tunnelId
sai_remove_tunnel_map_entry $tunmapEntry1
sai_remove_tunnel_map_entry $tunmapEntry2
sai_remove_tunnel_map $tunnel_map1
sai_remove_tunnel_map $tunnel_map2
back
vlan
sai_remove_vlan $vlan100
back
route
sai_remove_route_entry 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0]
back
nexthop
sai_remove_next_hop $nh0
back
neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_i_prt_01 0 192.168.2.100
back
routerinterface
sai_remove_router_interface $rif_i_prt_01
back
home
xps
fdb
fdb_register_learn_handler 1

'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'$fdb_cnt - 2',
}
