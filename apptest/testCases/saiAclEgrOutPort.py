import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L4Packet',
    'tcName'         : 'saiEgrAclOutPort',
    'description'    : 'TC to verify IACL trap for outport Entry',
    'ingressPort'    : ['28'],
    'egressPort'     : ['29'],
    'vlan'           : '75',
    'pktAction'      : 'FORWARD',
    'ingressTapIntf' : 'tap0',
    'egressTapIntf'  : ['tap1'],
    'count'          : 1 ,             # expected data count
    'acl_counter'    : ['acl_counter_id0'] #only for acl cases
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
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710687 SAI_PORT_ATTR_ADMIN_STATE 1

home
sai 
vlan
sai_create_vlan 9288674231451648 75 > vlan75
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218076 1 > memtap0
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218077 1 > memtap1
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218078 1 > memtap2
back
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_EGRESS [] 1 > acl_table_group
sai_create_acl_table 9288674231451648 1 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 > acl_tbl_id
sai_create_acl_table_group_member 9288674231451648 $acl_tbl_id $acl_table_group 0 > acl_table_group_mem

sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id0

sai_create_acl_entry 9288674231451648 $acl_tbl_id 11 > acl_entry_id0
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_TRAP
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id0
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_FIELD_OUT_PORT 1 281474976710686 0

'''


#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home
sai
acl
sai_remove_acl_entry $acl_entry_id0
sai_remove_acl_counter $acl_counter_id0
sai_remove_acl_table_group_member $acl_table_group_mem
sai_remove_acl_table $acl_tbl_id
sai_remove_acl_table_group $acl_table_group
back                                                                                                                                
vlan                                                                                                                                
sai_remove_vlan_member $memtap0                                                                                                     
sai_remove_vlan_member $memtap1                                                                                                     
sai_remove_vlan_member $memtap2                                                                                                     
sai_remove_vlan $vlan75   
home

'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP()/scapy_t.TCP(sport=2471,dport=2472)

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
    'expect1':''
}

