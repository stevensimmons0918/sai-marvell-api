import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'Ipv4 Packet',
    'tcName' : 'saiHashEcmpWithCrc',
    'description' : 'Verify crc loadbalance for ecmp hash',
    'ingressPort' : ['29'],
    'egressPort' : ['30'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap2'],
    'count' : 1,             # expected data count
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
'''
#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
route
sai_remove_route_entry 9288674231451648 $vr_id 0 10.10.10.1 255.255.255.0

back
nexthopgroup
sai_remove_next_hop_group_member $nh_group_member_id_1
sai_remove_next_hop_group_member $nh_group_member_id_3
sai_remove_next_hop_group $nhop_group_id1

back
nexthop
sai_remove_next_hop $nh1
sai_remove_next_hop $nh3

back
neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_i_prt_1 0 30.1.1.20
sai_remove_neighbor_entry 9288674231451648 $rif_i_prt_3 0 111.2.3.100

back
routerinterface
sai_remove_router_interface $rif_i_prt_1
sai_remove_router_interface $rif_i_prt_2
sai_remove_router_interface $rif_i_prt_3

back
virtualrouter
sai_remove_virtual_router $vr_id

back
hash
sai_set_hash_attribute 7881299347898368 SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST 14:17:18:19
'''

packet_info = scapy_t.Ether(src="00:00:00:00:00:11",dst="00:00:01:11:22:03")/scapy_t.IP(dst="10.10.10.1", src="20.1.1.222",ttl=45)



#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':''
}

