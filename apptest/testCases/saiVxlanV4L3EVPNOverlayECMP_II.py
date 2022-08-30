import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'Vxlan Ipv4 Packet',
    'tcName' : 'saiVxlanV4L3EVPNOverlayECMP_II',
    'description' : 'Verify Vxlan remote tunnel overlay ECMP, route two',
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
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
vlan
sai_remove_vlan_member $vmem29
sai_remove_vlan_member $vmem29_2
sai_remove_vlan_member $vmem1_tun
back

route
sai_remove_route_entry 9288674231451648 $vr_id 0 20.1.1.0 [255.255.255.0]
back

nexthopgroup
sai_remove_next_hop_group_member $nh_group_member_id_1
sai_remove_next_hop_group_member $nh_group_member_id_2
sai_remove_next_hop_group $nhop_group_id
back



nexthop
sai_remove_next_hop $nh2
sai_remove_next_hop $nh3
back

bridge
sai_remove_bridge_port $brgport
sai_remove_bridge_port $brgport2
back

tunnel
sai_remove_tunnel_term_table_entry $tuntermId
sai_remove_tunnel_term_table_entry $tuntermId2
sai_remove_tunnel $tunnelId
sai_remove_tunnel $tunnelId2
sai_remove_tunnel_map_entry $tunmapEntry1
sai_remove_tunnel_map_entry $tunmapEntry2
sai_remove_tunnel_map $tunnel_map1
sai_remove_tunnel_map $tunnel_map2
back

routerinterface
sai_remove_router_interface $rif_vlan200
back

route
sai_remove_route_entry 9288674231451648 $vr_id 0 10.1.1.0 [255.255.255.0]
back

nexthop
sai_remove_next_hop $nh1
back

neighbor
sai_remove_neighbor_entry 9288674231451648 $rif_vlan100 0 10.1.1.2
back

routerinterface
sai_remove_router_interface $rif_vlan100
back

vlan
sai_remove_vlan $vlan100
back

route
sai_remove_route_entry 9288674231451648 $vr_id 0 192.168.2.10 [255.255.255.0]
sai_remove_route_entry 9288674231451648 $vr_id 0 2.2.2.2 [255.255.255.0]
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
vlan
sai_remove_vlan $vlan200
back
virtualrouter
sai_remove_virtual_router $vr_id

'''
packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:02")/scapy_t.Dot1Q(vlan =100)/scapy_t.IP(dst="20.1.1.1", src="10.10.10.2",ttl=45)

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
       'tap0':[<Ether  dst=00:04:04:04:04:49 src=00:00:01:11:22:02 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=92 id=10 flags= frag=0 ttl=44 proto=udp chksum=0xc6da src=192.168.3.1 dst=2.2.2.2 |<UDP  sport=52846 dport=4789 len=72 chksum=0x0 |<VXLAN  flags=Instance reserved1=0 vni=0xbb8 reserved2=0x0 |<Ether  dst=00:06:06:06:06:06 src=00:00:01:11:22:02 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=44 proto=hopopt chksum=0x65dc src=10.10.10.2 dst=20.1.1.1 |
       '''
}
