import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.
tcParams = {
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiErspanIpv6OverIpv6',
    'description' : 'TC to ERSPAN - Ipv6 over Ipv6',
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
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IPv6(src="00:00:11::33", dst="00:00:11::22")

#
#expectedData: This dictionary expected egress stream for each egress port.
#
expectedData = {
       'expect1':'''
'tap2':[<Ether  dst=00:00:11:22:33:44 src=00:00:11:22:33:55 type=IPv6 |<IPv6  version=6 tc=8 fl=0 plen=92 nh=GRE hlim=255 src=2001:db08:1111:2222::2 dst=2001:db08:1111:2222::1 |<GRE  chksum_present=0 routing_present=0 key_present=0 seqnum_present=1 strict_route_source=0 recursion_control=0 flags=0 version=0 proto=0x88be seqence_number=0x2 |<ERSPAN  ver=1 vlan=0 cos=0 en=3 t=0 session_id=0 reserved=0 index=28 |<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv6 |<IPv6  version=6 tc=0 fl=0 plen=0 nh=No Next Header hlim=64 src=0:0:11::33 dst=0:0:11::22
'''
}
