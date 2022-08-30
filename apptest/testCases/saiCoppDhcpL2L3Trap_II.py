import scapy.all as scapy_t

#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'DHCPPacket',
    'tcName' : 'saiCoppDhcpTrap',
    'description' : 'TC to verify that the L3 unicast DHCP packet is trapped when the packet action is set to trap',
    'ingressPort' : ['30'],
    'egressPort' : [],
    'pktAction' : 'TRAP',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : '',
    'count' : 1,             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
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
home
sai
hostif
sai_remove_hostif_trap $trp
sai_remove_hostif_trap $trp2
sai_remove_hostif_table_entry $hostiftab
sai_remove_hostif_trap_group $tgrp
home
sai
route
sai_remove_route_entry 9288674231451648 $vr_id6 0 50.50.2.0 [255.255.255.0]
back
routerinterface
sai_remove_router_interface $rif_id_1
sai_remove_router_interface $rif_id_2
home
sai
virtualrouter
sai_remove_virtual_router $vr_id6
back

'''

packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:02")/scapy_t.IP(dst="50.50.2.1", src="0.0.0.0")/scapy_t.UDP(sport=68,dport=67)
#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'',
}

