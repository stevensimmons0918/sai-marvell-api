#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'saiMacPortLoopbackEnable',
    'description' : 'TC to verify enabling loopback on port',
    'ingressPort' : [''],
    'egressPort' : [''],
    'pktAction' : '',
    'count' : 1,             # expected data count
    'counter' : 'max_ports', # Counter for  checking loopback in all ports
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
xps 
mac
mac_port_loopback_enable 0 $counter 0
mac_port_loopback_enable 0 $counter 1
mac_port_loopback_enable_get 0 $counter
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

