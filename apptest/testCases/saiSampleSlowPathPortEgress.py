import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.
tcParams = {
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiSampleSlowPathPortIngress',
    'description' : 'TC to verify port based egress sample',
    'ingressPort' : ['29'],
    'egressPort' : ['28'],
    'pktAction' : 'TRAP',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : [],
    'count' : 1,   # expected data count
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

home
sai
hostif
sai_create_hostif_trap_group 9288674231451648 > trgp
sai_create_hostif_trap 9288674231451648 SAI_HOSTIF_TRAP_TYPE_SAMPLEPACKET SAI_PACKET_ACTION_FORWARD > trp
sai_set_hostif_trap_attribute $trp SAI_HOSTIF_TRAP_ATTR_TRAP_GROUP $trgp
sai_set_hostif_trap_group_attribute $trgp SAI_HOSTIF_TRAP_GROUP_ATTR_POLICER $plcr1

home
sai
samplepacket
sai_create_samplepacket 9288674231451648 SAI_SAMPLEPACKET_MODE_SHARED SAI_SAMPLEPACKET_TYPE_SLOW_PATH 1 > smpl

home
sai
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE $smpl
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.
tcFlushStr = '''
home
sai
policer
sai_get_policer_stats $plcr1 0

back
port
sai_set_port_attribute 281474976710684 SAI_PORT_ATTR_EGRESS_SAMPLEPACKET_ENABLE 0

back
samplepacket
sai_remove_samplepacket $smpl

back
hostif
sai_remove_hostif_trap $trp
sai_remove_hostif_trap_group $trgp

back
policer
sai_remove_policer $plcr1

back
vlan
sai_remove_vlan_member $memtap0
sai_remove_vlan_member $memtap1
sai_remove_vlan $vlan75

'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="00:00:11:00:11:23")/scapy_t.Dot1Q(vlan =75)/scapy_t.IP(src="195.169.0.20", dst="195.169.0.18")

#
#expectedData: This dictionary expected egress stream for each egress port.
#
expectedData = {
       'expect1':''
}
