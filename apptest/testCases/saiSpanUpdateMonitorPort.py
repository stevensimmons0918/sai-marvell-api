import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.
tcParams = {
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiSpanUpdateMonitorport',
    'description' : 'TC to verify updating monitor port of SPAN session',
    'ingressPort' : ['28'],
    'egressPort' : ['31'],
    'vlan'       : '',
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap0',
    'egressTapIntf'  : ['tap3'],
    'count' : 1,   # expected data count
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.
tcProgramStr = '''
home
sai
mirror
sai_create_mirror_session 9288674231451648 0 0.0.0.0 0 0.0.0.0 0 0 0 281474976710686 0 0 0 0 0 > mirror_session0
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_MIRROR_SESSION $mirror_session0
sai_get_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_MIRROR_SESSION 1
back
mirror
sai_set_mirror_session_attribute $mirror_session0 SAI_MIRROR_SESSION_ATTR_MONITOR_PORT 281474976710687 1
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

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP(src="195.169.0.20", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#
expectedData = {
       'expect1':'''
'tap3':[<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf370 src=195.169.0.20 dst=195.169.0.18
'''
}
