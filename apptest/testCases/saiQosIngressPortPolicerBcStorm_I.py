import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'L2Packet',
    'tcName' : 'saiQosPortPolicerBcStorm_I',
    'description' : 'TC to verify ingress port policer - unknown broadcast storm control with STORM_CONTROL mode',
    'ingressPort' : ['29'],
    'egressPort' : ['28','30'],
    'vlan'       : '75',
    'pktAction' : 'DROP',
    'ingressTapIntf' : 'tap1',
    'egressTapIntf'  : ['tap0','tap2'],
    'count' : 2             # expected data count
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
sai_create_policer 9288674231451648 1 2 1 > policer_id1
sai_set_policer_attribute $policer_id1 SAI_POLICER_ATTR_CIR 1000
sai_set_policer_attribute $policer_id1 SAI_POLICER_ATTR_CBS 2000
sai_set_policer_attribute $policer_id1 SAI_POLICER_ATTR_GREEN_PACKET_ACTION 1
sai_set_policer_attribute $policer_id1 SAI_POLICER_ATTR_RED_PACKET_ACTION 0
sai_create_policer 9288674231451648 1 2 1 > policer_id2
sai_set_policer_attribute $policer_id2 SAI_POLICER_ATTR_CIR 1000
sai_set_policer_attribute $policer_id2 SAI_POLICER_ATTR_CBS 2000
sai_set_policer_attribute $policer_id2 SAI_POLICER_ATTR_GREEN_PACKET_ACTION 0
sai_set_policer_attribute $policer_id2 SAI_POLICER_ATTR_RED_PACKET_ACTION 0
back
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_POLICER_ID $policer_id1
sai_get_port_attribute 281474976710685 SAI_PORT_ATTR_POLICER_ID 1
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID $policer_id2
sai_get_port_attribute 281474976710685 SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID 1
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
port
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_BROADCAST_STORM_CONTROL_POLICER_ID 0
sai_set_port_attribute 281474976710685 SAI_PORT_ATTR_POLICER_ID 0
back
policer
sai_remove_policer $policer_id1
sai_remove_policer $policer_id2
home
'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:00",dst="FF:FF:FF:FF:FF:FF")/scapy_t.Dot1Q(vlan =75)/scapy_t.IP()


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'',
       'expect2':''
}
