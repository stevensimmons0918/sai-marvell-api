#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : '',
    'tcName' : 'saiFdbAgingIpv4_I.py',
    'description' : 'TC to verify the aging with ipv4 entries already present',
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
fdb_get_table_depth 0 > max_fdb_cnt
home
sai
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1
back
virtualrouter
sai_create_virtual_router 9288674231451648 > vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02
back
home
sai
routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710684 $vr_id > rif_i_prt_01
back
nexthop
sai_create_next_hop 9288674231451648 0 $rif_i_prt_01 SAI_NEXT_HOP_TYPE_IP 0 192.168.2.100 > nh0
back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_i_prt_01 0 192.168.2.100 00:04:04:04:04:49
back
route
sai_create_route_entry 9288674231451648 $vr_id 0 192.168.2.10 255.255.255.0
sai_set_route_entry_attribute 9288674231451648 $vr_id 0 192.168.2.10 255.255.255.0 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $nh0
back
home
sai
vlan
sai_create_vlan 9288674231451648 75 > vlan75
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218076 1 > memtap0
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218077 1 > memtap1
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218079 1 > memtap3
home
sai
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_AVAILABLE_FDB_ENTRY 0
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''

'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#

#expected: fdb - no.of RIF *  mac2me + 1 v4 nbr
expectedData = {
       'expect1':'$max_fdb_cnt - 2',
}



