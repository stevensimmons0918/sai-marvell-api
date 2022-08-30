import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.
tcParams = {
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiSpanPortAndAclIngress_2',
    'description' : 'TC to verify ingress port based and acl based span',
    'ingressPort' : ['28'],
    'egressPort' : ['29'],
    'vlan'       : '',
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap0',
    'egressTapIntf'  : ['tap1'],
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
acl
sai_remove_acl_entry $acl_entry0
sai_remove_acl_table_group_member $acl_table_group_member0
sai_remove_acl_counter $acl_counter0
sai_remove_acl_table $acl_table0
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_ACL 0
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_MIRROR_SESSION 0
back
acl
sai_remove_acl_table_group $acl_table_group0
back
mirror
sai_remove_mirror_session $mirror_session0
sai_remove_mirror_session $mirror_session1
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP(src="195.169.0.55", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#
#[<Ether  dst=00:00:11:22:33:44 src=00:00:11:22:33:55 type=IPv4 |<IP  version=4 ihl=5 tos=0x8 len=100 id=0 flags= frag=0 ttl=255 proto=gre chksum=0xb159 src=2.2.2.2 dst=3.3.3.3 |<GRE  chksum_present=0 routing_present=0 key_present=0 seqnum_present=1 strict_route_source=0 recursion_control=0 flags=0 version=0 proto=0x88be seqence_number=0x1 |<ERSPAN  ver=1 vlan=0 cos=0 en=3 t=0 session_id=0 reserved=0 index=28 |<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf34d src=195.169.0.55 dst=195.169.0.18
expectedData = {
       'expect1':'''
'tap1':[<Ether  dst=00:00:11:22:33:44 src=00:00:11:22:33:55 type=IPv4 |<IP  version=4 ihl=5 tos=0x8 len=84 id=12 flags= frag=0 ttl=255 proto=gre chksum=0xb15d src=2.2.2.2 dst=3.3.3.3 |<GRE  chksum_present=0 routing_present=0 key_present=0 seqnum_present=0 strict_route_source=0 recursion_control=0 flags=0 version=0 proto=0x6558 |<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf34d src=195.169.0.55 dst=195.169.0.18
'''
}
