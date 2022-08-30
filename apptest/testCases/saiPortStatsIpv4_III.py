#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : '',
    'tcName'         : 'saiPortStatsIpv4_III',
    'description'    : 'Port statistics for V4 packets',
    'ingressPort' : [''],
    'egressPort' : [''],
    'count'  : 1,             # expected data count
}

#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
port
sai_get_port_stats 281474976710686 SAI_PORT_STAT_IP_OUT_OCTETS
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
sai_remove_vlan $vlan75
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#
expectedData = {
        'expect1': '56'
}