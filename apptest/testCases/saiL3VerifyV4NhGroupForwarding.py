import scapy.all as scapy_t

#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiVerifyV4NhGroupForwarding',
    'description' : 'TC to verify if the traffic forwarding works fine on V4 NH Group',
    'ingressPort' : ['30'],
    'egressPort' : ['29'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : ['tap1'],
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = ''' 
home
sai
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1

home
sai
virtualrouter
sai_create_virtual_router 9288674231451648 >vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02

back
routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710685 $vr_id >rif_i_prt_01
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710686 $vr_id >rif_i_prt_03

back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_i_prt_01 0 10.10.10.1 00:00:01:02:03:04

back
nexthop
sai_create_next_hop 9288674231451648 0 $rif_i_prt_01 SAI_NEXT_HOP_TYPE_IP 0 [10.10.10.1] > nh0

back
nexthopgroup
sai_create_next_hop_group 9288674231451648 SAI_NEXT_HOP_GROUP_TYPE_ECMP 0 >nhop_group_id
sai_create_next_hop_group_member 9288674231451648 $nh0 $nhop_group_id 0 >nh_group_member_id_1 

back
route
sai_create_route_entry 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0]
sai_set_route_entry_attribute 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0] SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $nhop_group_id
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai

route
sai_remove_route_entry 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0]
back

nexthopgroup
sai_remove_next_hop_group_member $nh_group_member_id_1
sai_remove_next_hop_group $nhop_group_id
back

nexthop
sai_remove_next_hop $nh0
back

neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_i_prt_01 0 10.10.10.1
back

routerinterface
sai_remove_router_interface $rif_i_prt_01
sai_remove_router_interface $rif_i_prt_03
back
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:02")/scapy_t.IP(dst="192.168.2.10", src="192.168.1.10")
#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
'expect1':'''
'tap1': [<Ether  dst=00:00:01:02:03:04 src=00:00:01:11:22:02 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=63 proto=hopopt chksum=0xf784 src=192.168.1.10 dst=192.168.2.10 |
 '''
}
