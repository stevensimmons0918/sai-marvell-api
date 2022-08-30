import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L4Packet',
    'tcName'         : 'saiErspanEgrOutPortsMultiMir_2',
    'description'    : 'TC to verify IACL trap for inports Entry',
    'ingressPort'    : ['28'],
    'egressPort'     : ['30'],
    'vlan'           : '75',
    'pktAction'      : 'FORWARD',
    'ingressTapIntf' : 'tap0',
    'egressTapIntf'  : ['tap2'],
    'count'          : 1 ,             # expected data count
    'acl_counter'    : ['acl_counter_id1'] #only for acl cases
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
sai_remove_acl_entry $acl_entry_id0
sai_remove_acl_entry $acl_entry_id1
sai_remove_acl_counter $acl_counter_id0
sai_remove_acl_counter $acl_counter_id1
sai_remove_acl_table_group_member $acl_table_group_mem
sai_remove_acl_table $acl_tbl_id
sai_remove_acl_table_group $acl_table_group
back                                                                                                           
mirror
sai_remove_mirror_session $mirror_session0
sai_remove_mirror_session $mirror_session1
back
vlan
sai_remove_vlan_member $memtap0
sai_remove_vlan_member $memtap1
sai_remove_vlan $vlan75
home
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.Dot1Q(vlan =75)/scapy_t.IPv6(src="0300:0001:0:0:0:0:0:0008",dst="0200:0001:0:0:0:0:0:0009")


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
    'expect1':''
}

