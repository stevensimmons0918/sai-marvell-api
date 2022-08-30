import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiEverflowIpv4ErspanMultipleMirrorSessionTest_2',
    'description' : 'TC to verify Ipv4 mirroring(ERSPAN) with multiple mirror session and multiple ACL Rules',
    'ingressPort' : ['28'],
    'egressPort' : ['31'],
    'vlan'       : '75',
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

'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
acl
sai_remove_acl_entry $acl_entry0
sai_remove_acl_entry $acl_entry1
sai_remove_acl_entry $acl_entry2
sai_remove_acl_counter $acl_counter0
sai_remove_acl_counter $acl_counter1
sai_remove_acl_counter $acl_counter2
sai_remove_acl_table_group_member $acl_table_group_member0
sai_remove_acl_table $acl_table0
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_INGRESS_ACL 0
back
acl
sai_remove_acl_table_group $acl_table_group0
back
mirror
sai_remove_mirror_session $mirror_session0
sai_remove_mirror_session $mirror_session1
sai_remove_mirror_session $mirror_session2
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP(src="195.169.0.22", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
'tap3':[<Ether  dst=00:00:11:22:33:44 src=00:00:11:22:33:55 type=IPv4 |<IP  version=4 ihl=5 tos=0x8 len=100 id=0 flags= frag=0 ttl=255 proto=gre chksum=0xb55d src=1.1.1.1 dst=2.2.2.2 |<GRE  chksum_present=0 routing_present=0 key_present=0 seqnum_present=1 strict_route_source=0 recursion_control=0 flags=0 version=0 proto=0x88be seqence_number=0x1 |<ERSPAN  ver=1 vlan=0 cos=0 en=3 t=0 session_id=2 reserved=0 index=28 |<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf36e src=195.169.0.22 dst=195.169.0.18
'''
}

