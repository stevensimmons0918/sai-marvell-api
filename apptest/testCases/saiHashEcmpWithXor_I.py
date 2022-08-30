import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'Ipv4 Packet',
    'tcName' : 'saiHashEcmpWithXor',
    'description' : 'Verify xor loadbalance for ecmp hash',
    'ingressPort' : ['29'],
    'egressPort' : ['28'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap0'],
    'count' : 1,             # expected data count
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1

back 
virtualrouter
sai_create_virtual_router 9288674231451648 > vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:03

back
routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710684 $vr_id > rif_i_prt_1
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710685 $vr_id > rif_i_prt_2
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710686 $vr_id > rif_i_prt_3

back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_i_prt_1 0 30.1.1.20 00:30:03:04:05:06
sai_create_neighbor_entry 9288674231451648 $rif_i_prt_3 0 111.2.3.100 00:30:33:44:55:66

back
nexthop
sai_create_next_hop 9288674231451648 0 $rif_i_prt_1 SAI_NEXT_HOP_TYPE_IP 0 30.1.1.20 > nh1
sai_create_next_hop 9288674231451648 0 $rif_i_prt_3 SAI_NEXT_HOP_TYPE_IP 0 111.2.3.100 > nh3

back 
nexthopgroup
sai_create_next_hop_group 9288674231451648 SAI_NEXT_HOP_GROUP_TYPE_ECMP 0 > nhop_group_id1
sai_create_next_hop_group_member 9288674231451648 $nh1 $nhop_group_id1 0 > nh_group_member_id_1
sai_create_next_hop_group_member 9288674231451648 $nh3 $nhop_group_id1 0 > nh_group_member_id_3
 
back
route
sai_create_route_entry 9288674231451648 $vr_id 0 10.10.10.1 255.255.255.0
sai_set_route_entry_attribute 9288674231451648 $vr_id 0 10.10.10.1 255.255.255.0 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $nhop_group_id1

back
hash
sai_set_hash_attribute 7881299347898368 SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST 0:1

back
switch
sai_set_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_ALGORITHM 1

'''
#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
'''

packet_info = scapy_t.Ether(src="00:00:00:00:00:11",dst="00:00:01:11:22:03")/scapy_t.IP(dst="10.10.10.1", src="20.1.1.7",ttl=45)



#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':''
}

