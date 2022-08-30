#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'saiMacRxMaxFrameLen_III',
    'description' : 'TC to Set and Get Mac Maximum Rx frame length',
    'ingressPort' : [''],
    'egressPort' : [''],
    'FrameLen' : 128, 
    'pktAction' : '',
    'count' : 1,             # expected data count
    'counter' : 'max_ports', # counter for cehcking frame length in all ports
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
 home
 xps
 mac
 mac_set_rx_max_frm_len 0 $counter 128
 mac_get_rx_max_frm_len 0 $counter 128
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
       'expect1':'128',
}

