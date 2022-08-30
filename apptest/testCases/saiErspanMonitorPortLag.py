import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.
tcParams = {
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiErspanMonitorPortLag',
    'description' : 'TC to verify port based ingress ERSPAN with monitor port as lag',
    'ingressPort' : ['28'],
    'egressPort' : ['30'],
    'vlan'       : '',
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap0',
    'egressTapIntf'  : ['tap2'],
    'count' : 1,   # expected data count
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.
tcProgramStr = '''
home
sai
lag
sai_create_lag 9288674231451648 1 > lag0
sai_create_lag_member 9288674231451648 281474976710686 $lag0 > mem1
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID 1 > brg
back
bridge
sai_create_bridge_port 9288674231451648 0 $lag0 $brg SAI_BRIDGE_PORT_TYPE_PORT 0 0 > brgportlag
back
mirror
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV4 2.2.2.2 SAI_IP_ADDR_FAMILY_IPV4 3.3.3.3 0 SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE $lag0 00:00:11:22:33:44 00:00:11:22:33:55 25944 4 8 > mirror_session0
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_MIRROR_SESSION $mirror_session0
sai_get_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_MIRROR_SESSION 1
back
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.
tcFlushStr = '''
home
sai
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_MIRROR_SESSION 0
back
mirror
sai_remove_mirror_session $mirror_session0
back
bridge
sai_remove_bridge_port $brgportlag
back
lag
sai_remove_lag_member $mem1
sai_remove_lag $lag0
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP(src="195.169.0.20", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#
#[<Ether  dst=00:00:11:22:33:44 src=00:00:11:22:33:55 type=IPv4 |<IP  version=4 ihl=5 tos=0x8 len=100 id=0 flags= frag=0 ttl=255 proto=gre chksum=0xb159 src=2.2.2.2 dst=3.3.3.3 |<GRE  chksum_present=0 routing_present=0 key_present=0 seqnum_present=1 strict_route_source=0 recursion_control=0 flags=0 version=0 proto=0x88be seqence_number=0x1 |<ERSPAN  ver=1 vlan=0 cos=0 en=3 t=0 session_id=0 reserved=0 index=28 |<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf370 src=195.169.0.20 dst=195.169.0.18

expectedData = {
       'expect1':'''
'tap2':[<Ether  dst=00:00:11:22:33:44 src=00:00:11:22:33:55 type=IPv4 |<IP  version=4 ihl=5 tos=0x8 len=84 id=13 flags= frag=0 ttl=255 proto=gre chksum=0xb15c src=2.2.2.2 dst=3.3.3.3 |<GRE  chksum_present=0 routing_present=0 key_present=0 seqnum_present=0 strict_route_source=0 recursion_control=0 flags=0 version=0 proto=0x6558 |<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf370 src=195.169.0.20 dst=195.169.0.18
'''
}
