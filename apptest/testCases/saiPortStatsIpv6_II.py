#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket'  : '',
    'tcName'         : 'saiPortStatsIpv6_II',
    'description'    : 'Port statistics for V6 packets',
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
sai_get_port_stats 281474976710685 SAI_PORT_STAT_IPV6_IN_RECEIVES
'''

#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
'''

#
#expectedData: This dictionary expected egress stream for each egress port.
#
expectedData = {
        'expect1': '1'
}