import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'L2Packet',
    'tcName' : 'saiQosIngressFlowPolicer',
    'description' : 'TC to verify ingress flow based policer - Action Drop',
    'ingressPort' : ['29'],
    'egressPort' : ['28','30'],
    'vlan'       : '75',
    'pktAction' : 'DROP',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap0','tap2'],
    'count' : 1             # expected data count
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
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218078 1 > memtap2
back
policer
sai_create_policer 9288674231451648 0 0 0 > policer_id
sai_set_policer_attribute $policer_id SAI_POLICER_ATTR_CIR 1000
sai_set_policer_attribute $policer_id SAI_POLICER_ATTR_PIR 2000
sai_set_policer_attribute $policer_id SAI_POLICER_ATTR_CBS 2000
sai_set_policer_attribute $policer_id SAI_POLICER_ATTR_GREEN_PACKET_ACTION 0
sai_set_policer_attribute $policer_id SAI_POLICER_ATTR_YELLOW_PACKET_ACTION 0
sai_set_policer_attribute $policer_id SAI_POLICER_ATTR_RED_PACKET_ACTION 0
back
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_INGRESS [] 1 > acl_table_group
sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 > acl_tbl_id

sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id

sai_create_acl_entry 9288674231451648 $acl_tbl_id 10 > acl_entry_id
sai_set_acl_entry_attribute $acl_entry_id SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT 1 65535 2472
sai_set_acl_entry_attribute $acl_entry_id SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_FORWARD
sai_set_acl_entry_attribute $acl_entry_id SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id
sai_set_acl_entry_attribute $acl_entry_id SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER 1 $policer_id
back
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_INGRESS_ACL $acl_table_group
back
acl
sai_create_acl_table_group_member 9288674231451648 $acl_tbl_id $acl_table_group 0 > acl_table_group_mem
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
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_INGRESS_ACL 0
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
       'expect1':''
}
