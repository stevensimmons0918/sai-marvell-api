#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = { 
    'ingressPacket' : '',
    'tcName' : 'saiMacRxMaxFrameLen_V',
    'description' : 'TC to Set and Get Mac Maximum Rx frame length',
    'ingressPort' : [''],
    'egressPort' : [''],
    'FrameLen' : 9100, 
    'pktAction' : '',
    'count' : 1,             # expected data count
    'counter' : 'max_ports', # counter checking frame length in all ports
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
 home
 xps
 mac
 mac_set_rx_max_frm_len 0 $counter 9100
 mac_get_rx_max_frm_len 0 $counter 9100
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
       'expect1':'9100',
}

