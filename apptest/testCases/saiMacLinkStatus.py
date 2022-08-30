#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'saiMacLinkStatus',
    'description' : 'TC to check mac link status',
    'ingressPort' : [''],
    'egressPort' : [''],
    'pktAction' : '',
    'count' : 1,             # expected data count
    'counter': 'max_ports', #counter for checking Link status in all ports 
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = ''' 
home
xps
mac
mac_port_enable 0 $counter 0
mac_port_enable 0 $counter 1
mac_mac_loopback 0 $counter 4 0
mac_mac_loopback 0 $counter 4 1
mac_get_link_status 0 $counter
'''




#
#tcFlushStr: This string contains chain of xpShell commands to be used, in order to remove
#            specific networking scenario.

tcFlushStr = '''
home

'''


#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'1',
}

