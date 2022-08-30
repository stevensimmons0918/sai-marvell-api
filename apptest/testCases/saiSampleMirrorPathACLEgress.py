import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.
tcParams = {
    'ingressPacket' : 'L4Packet',
    'tcName' : 'saiSampleMirrorPathACLIngress',
    'description' : 'IACL sample for L4 Dst port Entry',
    'ingressPort' : ['28'],
    'egressPort'  : ['29','30'],
    'vlan'        : '75',
    'pktAction'   : 'FORWARD',
    'ingressTapIntf' : 'tap0',
    'egressTapIntf'  : ['tap2'],
    'count'          : 1,   # expected data count
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.
tcProgramStr = '''
home
sai
vlan
sai_create_vlan 9288674231451648 75 > vlan75
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218076 1 > memtap0
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218077 1 > memtap1

back
mirror
sai_create_mirror_session 9288674231451648 SAI_IP_ADDR_FAMILY_IPV4 0 SAI_IP_ADDR_FAMILY_IPV4 0 0 0 SAI_MIRROR_SESSION_TYPE_SFLOW 281474976710686 0 0 0 0 0 > mirror_session0

back
samplepacket
sai_create_samplepacket 9288674231451648 SAI_SAMPLEPACKET_MODE_SHARED SAI_SAMPLEPACKET_TYPE_MIRROR_SESSION 1 > smpl
sai_set_samplepacket_attribute $smpl SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE 1

back
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_EGRESS [] 1 > acl_table_group
sai_create_acl_table 9288674231451648 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 > acl_tbl_id

sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id0

sai_create_acl_entry 9288674231451648 $acl_tbl_id 10 > acl_entry_id0
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 195.169.0.44
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_FORWARD
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_MIRROR_EGRESS 1 $mirror_session0
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_EGRESS_SAMPLEPACKET_ENABLE 1 $smpl
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id0
back
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_EGRESS_ACL $acl_table_group
back
acl
sai_create_acl_table_group_member 9288674231451648 $acl_tbl_id $acl_table_group 0 >acl_table_group_mem
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.
tcFlushStr = '''
home
sai
acl
sai_get_acl_counter_attribute $acl_counter_id0 SAI_ACL_COUNTER_ATTR_PACKETS 1

home
sai
acl
sai_remove_acl_entry $acl_entry_id0
sai_remove_acl_table_group_member $acl_table_group_mem
sai_remove_acl_counter $acl_counter_id0
sai_remove_acl_table $acl_tbl_id

back
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_EGRESS_ACL 0

back
acl
sai_remove_acl_table_group $acl_table_group

back
vlan
sai_remove_vlan_member $memtap0
sai_remove_vlan_member $memtap1
sai_remove_vlan $vlan75

back
samplepacket
sai_remove_samplepacket $smpl

back
mirror
sai_remove_mirror_session $mirror_session0
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP(src="195.169.0.44", dst="195.169.0.18")/scapy_t.TCP(sport=2471,dport=2472)

#
#expectedData: This dictionary expected egress stream for each egress port.
#
#'tap1':[<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=64 proto=hopopt chksum=0xf370 src=195.169.0.44 dst=195.169.0.18
expectedData = {
       'expect1':'''
       'tap2':[<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=40 id=1 flags= frag=0 ttl=64 proto=tcp chksum=0xf33e src=195.169.0.44 dst=195.169.0.18 |<TCP  sport=2471 dport=2472 seq=0 ack=0 dataofs=5 reserved=0 flags=S window=8192 chksum=0xf502 urgptr=0 |
'''
}
