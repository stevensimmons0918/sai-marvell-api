import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : 'L4Packet',
    'tcName'         : 'saiIngrAclLagL4SrcDst',
    'description'    : 'TC to verify Ingress drop Acl Table Entry for L4 Src and L4 Dst',
    'ingressPort'    : ['30'],
    'egressPort'     : ['29'],
    'vlan'           : '75',
    'pktAction'      : 'DROP',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : ['tap1'],
    'count'          : 1,              # expected data count
    'acl_counter'    : ['acl_counter_id1'] #only for acl cases
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
vlan
sai_create_vlan 9288674231451648 75 > vlan75
sai_create_vlan_member 9288674231451648 $vlan75 16325548649218077 1 > memtap0
back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_ADMIN_STATE 1
sai_set_port_attribute 281474976710686 SAI_PORT_ATTR_ADMIN_STATE 1
back
lag
sai_create_lag 9288674231451648 1 > lag0
sai_create_lag_member 9288674231451648 281474976710686 $lag0 > mem30
back
switch
sai_get_switch_attribute 9288674231451648 SAI_SWITCH_ATTR_DEFAULT_1Q_BRIDGE_ID 1 > brg
back
bridge
sai_create_bridge_port 9288674231451648 0 $lag0 $brg SAI_BRIDGE_PORT_TYPE_PORT 0 0 > brgportlag
back
vlan
alter_create_mode 1
sai_create_vlan_member 9288674231451648 $vlan75 $brgportlag 1 > memtap1
back
acl
alter_create_mode 0
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_INGRESS [] 1 > acl_table_group
sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 1 0 0 > acl_tbl_id

sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id0
sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id1

sai_create_acl_entry 9288674231451648 $acl_tbl_id 10 > acl_entry_id0
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT 1 65535 2471
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_DROP
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id0
sai_create_acl_entry 9288674231451648 $acl_tbl_id 11 > acl_entry_id1
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT 1 65535 2472
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_DROP
sai_set_acl_entry_attribute $acl_entry_id1 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id1
back
lag
sai_set_lag_attribute $lag0 SAI_LAG_ATTR_INGRESS_ACL $acl_table_group
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
sai_remove_acl_entry $acl_entry_id0
sai_remove_acl_entry $acl_entry_id1
sai_remove_acl_counter $acl_counter_id0
sai_remove_acl_counter $acl_counter_id1
sai_remove_acl_table_group_member $acl_table_group_mem
sai_remove_acl_table $acl_tbl_id
back
lag
sai_set_lag_attribute $lag0 SAI_LAG_ATTR_INGRESS_ACL 0
back
acl
sai_remove_acl_table_group $acl_table_group
back
vlan
sai_remove_vlan_member $memtap0
sai_remove_vlan_member $memtap1
sai_remove_vlan $vlan75
back
bridge
sai_remove_bridge_port $brgportlag
back
lag
sai_remove_lag_member $mem30
sai_remove_lag $lag0
back
home

'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.Dot1Q(vlan =75)/scapy_t.IP()/scapy_t.TCP(sport=2471,dport=2472)

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
        'expect1':'''
'tap2': [<Ether  dst=00:00:11:00:11:23 src=00:00:11:00:11:00 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=40 id=1 flags= frag=0 ttl=64 proto=tcp chksum=0x7ccd src=127.0.0.1 dst=127.0.0.1 |'''
}

