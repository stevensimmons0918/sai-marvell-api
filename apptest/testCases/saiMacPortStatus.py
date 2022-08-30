#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'saiMacPortStatus',
    'description' : 'TC to check port status',
    'ingressPort' : [''],
    'egressPort' : [''],
    'pktAction' : '',
    'count' : 3,             # expected data count
    'counter' : 'max_ports', # counter for checking port status in all ports
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = ''' 
home
xps
mac
mac_mac_loopback 0 $counter 4 0
mac_mac_loopback 0 $counter 4 1
mac_get_port_status 0 $counter
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
       'expect2':'1',
       'expect3':'0',
}

