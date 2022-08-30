import scapy.all as scapy_t
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : 'Vxlan Ipv4 Packet',
    'tcName' : 'saiVxlanV4EncapPipe',
    'description' : 'Verify Vxlan tunnel start',
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

home
sai
virtualrouter
sai_create_virtual_router 9288674231451648 > vr_id
sai_set_virtual_router_attribute $vr_id SAI_VIRTUAL_ROUTER_ATTR_SRC_MAC_ADDRESS 00:00:01:11:22:02
back
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
tunnel
sai_create_tunnel_map 9288674231451648 SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI > tunnel_map1
sai_create_tunnel_map_entry 9288674231451648 SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI 0 $tunnel_map1 2000 0 100 0 0 > tunmapEntry1
sai_create_tunnel_map 9288674231451648 SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID > tunnel_map2
sai_create_tunnel_map_entry 9288674231451648 SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID 0 $tunnel_map2 0 100 0 2000 0 > tunmapEntry2

back
vlan
sai_create_vlan 9288674231451648 100 > vlan100
back
tunnel
sai_create_tunnel 9288674231451648 $rif_i_prt_01 $tunnel_map2 0 192.168.3.1 0 0 192.168.2.10 SAI_TUNNEL_TYPE_VXLAN $tunnel_map1 > tunnelId
sai_create_tunnel_term_table_entry 9288674231451648 0 192.168.2.10 0 255.255.255.255 $tunnelId SAI_TUNNEL_TYPE_VXLAN 0 192.168.3.1 SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P $vr_id 0 255.255.255.255 > tuntermId

sai_set_tunnel_attribute $tunnelId SAI_TUNNEL_ATTR_ENCAP_TTL_MODE SAI_TUNNEL_TTL_MODE_PIPE_MODEL
sai_set_tunnel_attribute $tunnelId SAI_TUNNEL_ATTR_ENCAP_TTL_VAL 99
sai_set_tunnel_attribute $tunnelId SAI_TUNNEL_ATTR_ENCAP_DSCP_MODE SAI_TUNNEL_DSCP_MODE_PIPE_MODEL
sai_set_tunnel_attribute $tunnelId SAI_TUNNEL_ATTR_ENCAP_DSCP_VAL 18

sai_set_tunnel_attribute $tunnelId SAI_TUNNEL_ATTR_ENCAP_TTL_MODE SAI_TUNNEL_TTL_MODE_UNIFORM_MODEL
sai_set_tunnel_attribute $tunnelId SAI_TUNNEL_ATTR_ENCAP_DSCP_MODE SAI_TUNNEL_DSCP_MODE_UNIFORM_MODEL

sai_set_tunnel_attribute $tunnelId SAI_TUNNEL_ATTR_ENCAP_TTL_MODE SAI_TUNNEL_TTL_MODE_PIPE_MODEL
sai_set_tunnel_attribute $tunnelId SAI_TUNNEL_ATTR_ENCAP_TTL_VAL 66
sai_set_tunnel_attribute $tunnelId SAI_TUNNEL_ATTR_ENCAP_DSCP_MODE SAI_TUNNEL_DSCP_MODE_PIPE_MODEL
sai_set_tunnel_attribute $tunnelId SAI_TUNNEL_ATTR_ENCAP_DSCP_VAL 9

back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID 1 > brg
back
bridge
sai_create_bridge_port 9288674231451648 0 0 $brg SAI_BRIDGE_PORT_TYPE_TUNNEL $tunnelId 0 > brgport
back

vlan
alter_create_mode 1
sai_create_vlan_member 9288674231451648 $vlan100 16325548649218077 1 > vmem29
sai_create_vlan_member 9288674231451648 $vlan100 $brgport 1 > vmem1
alter_create_mode 0
back
'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
vlan
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
'''
packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:03")/scapy_t.Dot1Q(vlan =100)/scapy_t.IP(dst="10.10.10.1", src="20.1.1.1",ttl=45)

#
#expectedData: This dictionary expected egress stream for each egress port.
#
#       'tap0':[<Ether  dst=00:04:04:04:04:49 src=00:00:01:11:22:02 type=IPv4 |<IP  version=4 ihl=5 tos=0x24 len=92 id=0 flags= frag=0 ttl=66 proto=udp chksum=0xf211 src=192.168.3.1 dst=192.168.2.10 |<UDP  sport=51259 dport=4789 len=72 chksum=0x0 |<VXLAN  flags=Instance reserved1=0 vni=0x7d0 reserved2=0x0 |<Ether  dst=00:00:01:11:22:03 src=00:00:11:00:11:23 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=45 proto=hopopt chksum=0x64dd src=20.1.1.1 dst=10.10.10.1 |
#'tap0':[<Ether  dst=00:04:04:04:04:49 src=00:00:01:11:22:02 type=IPv4 |<IP  version=4 ihl=5 tos=0x24 len=92 id=0 flags= frag=0 ttl=66 proto=udp chksum=0xf211 src=192.168.3.1 dst=192.168.2.10 |<UDP  sport=51259 dport=4789 len=72 chksum=0x0 |<VXLAN  flags=Instance reserved1=0 vni=0x7d0 reserved2=0x0 |<Ether  dst=00:00:01:11:22:03 src=00:00:11:00:11:23 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=45 proto=hopopt chksum=0x64dd src=20.1.1.1 dst=10.10.10.1 |

expectedData = {
'expect1':'''
'tap0':[<Ether  dst=00:04:04:04:04:49 src=00:00:01:11:22:02 type=IPv4 |<IP  version=4 ihl=5 tos=0x24 len=92 id=1 flags= frag=0 ttl=66 proto=udp chksum=0xf210 src=192.168.3.1 dst=192.168.2.10 |<UDP  sport=51259 dport=4789 len=72 chksum=0x0 |<VXLAN  flags=Instance reserved1=0 vni=0x7d0 reserved2=0x0 |<Ether  dst=00:00:01:11:22:03 src=00:00:11:00:11:23 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=45 proto=hopopt chksum=0x64dd src=20.1.1.1 dst=10.10.10.1 |
'''
}
