import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'L2Packet',
    'tcName' : 'saiQosEgressFlowPolicer',
    'description' : 'TC to verify egress flow based policer - Action Drop',
    'ingressPort' : ['29'],
    'egressPort' : ['28','30'],
    'vlan'       : '75',
    'pktAction' : 'DROP',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap2'],
    'count' : 1             # expected data count
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
sai_remove_vlan_member $memtap0
sai_remove_vlan_member $memtap1
sai_remove_vlan_member $memtap2
sai_remove_vlan $vlan75
back
acl
sai_set_acl_entry_attribute $acl_entry_id SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER 1 0
sai_remove_acl_entry $acl_entry_id
sai_remove_acl_counter $acl_counter_id
sai_remove_acl_table_group_member $acl_table_group_mem
sai_remove_acl_table $acl_tbl_id
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_EGRESS_ACL 0
back
acl
sai_remove_acl_table_group $acl_table_group
back
policer
sai_remove_policer $policer_id
home
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP()/scapy_t.TCP(sport=2471,dport=2472)


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
'tap2': [<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=40 id=1 flags= frag=0 ttl=64 proto=tcp chksum=0x7ccd src=127.0.0.1 dst=127.0.0.1 |<TCP  sport=2471 dport=2472 seq=0 ack=0 dataofs=5 reserved=0 flags=S window=8192 chksum=0x7e91 urgptr=0'''
}
