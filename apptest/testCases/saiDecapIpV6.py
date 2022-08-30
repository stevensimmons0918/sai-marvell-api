import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : 'IPoIP Packet',
    'tcName' : 'saiipDecap',
    'description' : 'TC to verify IP Decap and stats works with basic forwarding ',
    'ingressPort' : ['29'],
    'egressPort' : ['30'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap2'],
#    'rif_counter'    : ['ing_rif_id','egr_rif_id'],
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = ''' 
home
sai
port
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1

home
sai

vlan
sai_create_vlan 9288674231451648 100 >vlan_id_100
sai_create_vlan 9288674231451648 200 >vlan_id_200
sai_create_vlan_member 281474976710685 $vlan_id_100 16325548649218077 >vlan_mbr_29
sai_create_vlan_member 281474976710685 $vlan_id_200 16325548649218078 >vlan_mbr_30
back

virtualrouter
sai_create_virtual_router 9288674231451648 >vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02
back

routerinterface
sai_create_router_interface 9288674231451648 $vlan_id_100 SAI_ROUTER_INTERFACE_TYPE_VLAN 0 $vr_id >ing_rif_id
sai_create_router_interface 9288674231451648 $vlan_id_200 SAI_ROUTER_INTERFACE_TYPE_VLAN 0 $vr_id >egr_rif_id

back
neighbor
sai_create_neighbor_entry 9288674231451648 $egr_rif_id 0 10.1.1.20 00:02:03:04:05:06
back

nexthop
sai_create_next_hop 9288674231451648 0 $egr_rif_id SAI_NEXT_HOP_TYPE_IP 0 10.1.1.20 >next_hop_id_out1
back

route
sai_create_route 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0]
sai_set_route_entry_attribute 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0] SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $next_hop_id_out1
back

tunnel
sai_create_tunnel 9288674231451648 0 0 1 [3f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] SAI_TUNNEL_PEER_MODE_P2MP 0 [0] SAI_TUNNEL_TYPE_IPINIP 0 > tnlId
sai_create_tunnel_term_table_entry 9288674231451648 1 [0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0] 1 [ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff:ff] $tnlId SAI_TUNNEL_TYPE_IPINIP 1 [3f:fe:05:01:48:19:0:0:0:0:0:0:0:0:0:0] SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2MP $vr_id 1 [0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0:0] > termId

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = ''' 
home
sai
tunnel
sai_remove_tunnel_term_table_entry $termId
sai_remove_tunnel $tnlId
back
route
sai_remove_route_entry 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0]
back
nexthop
sai_remove_next_hop $next_hop_id_out1
back
neighbor
sai_remove_neighbor_entry 9288674231451648 $egr_rif_id 0 10.1.1.20
back
routerinterface
sai_remove_router_interface $egr_rif_id
sai_remove_router_interface $ing_rif_id
back
vlan
sai_remove_vlan $vlan_id_100
sai_remove_vlan $vlan_id_200
back
'''
packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:02")/scapy_t.Dot1Q(vlan =100)/scapy_t.IPv6(dst="3ffe:0501:4819:0:0:0:0:0", src="3ffe:0502:4819:0:0:0:0:0",hlim=35)/scapy_t.IP(dst="192.168.2.10", src="192.168.1.10",ttl=30)

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
       'tap2': [<Ether  dst=00:02:03:04:05:06 src=00:00:01:11:22:02 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=34 proto=hopopt chksum=0x1485 src=192.168.1.10 dst=0.0.0.0 |
        '''
}
