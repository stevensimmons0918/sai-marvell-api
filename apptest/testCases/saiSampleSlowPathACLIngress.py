import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.
tcParams = {
    'ingressPacket' : 'L4Packet',
    'tcName' : 'saiSampleSlowPathACLIngress',
    'description' : 'IACL sample for L4 Dst port Entry',
    'ingressPort' : ['29'],
    'egressPort' : [],
    'pktAction' : 'TRAP',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : '',
    'count' : 1,   # expected data count
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.
tcProgramStr = '''
home
sai
policer
sai_create_policer 9288674231451648 SAI_METER_TYPE_PACKETS SAI_POLICER_MODE_TR_TCM SAI_POLICER_COLOR_SOURCE_BLIND >plcr1
sai_set_policer_attribute $plcr1 SAI_POLICER_ATTR_CIR 6000
sai_set_policer_attribute $plcr1 SAI_POLICER_ATTR_PIR 800
sai_set_policer_attribute $plcr1 SAI_POLICER_ATTR_CBS 8000
sai_set_policer_attribute $plcr1 SAI_POLICER_ATTR_PBS 1000
sai_set_policer_attribute $plcr1 SAI_POLICER_ATTR_GREEN_PACKET_ACTION 1
sai_set_policer_attribute $plcr1 SAI_POLICER_ATTR_YELLOW_PACKET_ACTION 1
sai_set_policer_attribute $plcr1 SAI_POLICER_ATTR_RED_PACKET_ACTION 1

back
hostif
sai_create_hostif_trap_group 9288674231451648 > trgp
sai_create_hostif_trap 9288674231451648 SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET SAI_PACKET_ACTION_FORWARD > trp
sai_set_hostif_trap_attribute $trp SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP $trgp
sai_set_hostif_trap_group_attribute $trgp SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER $plcr1

back
samplepacket
sai_create_samplepacket 9288674231451648 SAI_SAMPLEPACKET_MODE_SHARED SAI_SAMPLEPACKET_TYPE_SLOW_PATH 1 > smpl
sai_set_samplepacket_attribute $smpl SAI_SAMPLEPACKET_ATTR_SAMPLE_RATE 1

back
acl
sai_create_acl_table_group 9288674231451648 SAI_ACL_STAGE_INGRESS [] 1 > acl_table_group
sai_create_acl_table 9288674231451648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 > acl_tbl_id

sai_create_acl_counter 9288674231451648 $acl_tbl_id 1 1 > acl_counter_id0

sai_create_acl_entry 9288674231451648 $acl_tbl_id 10 > acl_entry_id0
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP 1 255:255:255:255 195.169.0.44
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION 1 SAI_PACKET_ACTION_FORWARD
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_INGRESS_SAMPLEPACKET_ENABLE 1 $smpl
sai_set_acl_entry_attribute $acl_entry_id0 SAI_ACL_ENTRY_ATTR_ACTION_COUNTER 1 $acl_counter_id0
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
policer
sai_get_policer_stats $plcr1 0

home
sai
acl
sai_get_acl_counter_attribute $acl_counter_id0 SAI_ACL_COUNTER_ATTR_PACKETS 1

back
acl
sai_remove_acl_entry $acl_entry_id0
sai_remove_acl_table_group_member $acl_table_group_mem
sai_remove_acl_counter $acl_counter_id0
sai_remove_acl_table $acl_tbl_id

back
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_INGRESS_ACL 0

back
acl
sai_remove_acl_table_group $acl_table_group

back
samplepacket
sai_remove_samplepacket $smpl

home
sai
hostif
sai_remove_hostif_trap $trp
sai_remove_hostif_trap_group $trgp

home
sai
policer
sai_remove_policer $plcr1
'''


# packet_info = scapy_t.Ether(src="00:00:00:00:00:02",dst="00:00:00:00:00:01")/scapy_t.IP(src="195.169.0.20", dst="195.169.0.18")/scapy_t.TCP(sport=2471,dport=2472)

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.IP(src="195.169.0.44", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#
expectedData = {
       'expect1':'',
}