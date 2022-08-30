import scapy.all as scapy_t
#
#tcParams: This dictionary contains parameters to be used, in order to configure specific
#          networking scenario, in future it can be used to auto generate spirent streams.

tcParams = {
    'ingressPacket' : 'L3Packet',
    'tcName' : 'saiL3BasicUTRouting_II',
    'description' : 'Basic Routing',
    'ingressPort' : ['30'],
    'egressPort' : ['28'],
    'pktAction' : 'FORWARD',
    'ingressTapIntf' : 'tap2',
    'egressTapIntf'  : ['tap0'],
    'count' : 1             # expected data count
}


#
#tcProgramStr: This string contains chain of xpShell commands to be used, in order to configure
#              specific networking scenario.

tcProgramStr = '''
home
sai
routerinterface
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710684 $vr_id > rif_i_prt_01
sai_create_router_interface 9288674231451648 0 SAI_ROUTER_INTERFACE_TYPE_PORT 281474976710686 $vr_id > rif_i_prt_02
back
nexthop
sai_create_next_hop 9288674231451648 0 $rif_i_prt_01 SAI_NEXT_HOP_TYPE_IP 0 192.168.2.100 > nh0
back
neighbor
sai_create_neighbor_entry 9288674231451648 $rif_i_prt_01 0 192.168.2.100 00:04:04:04:04:49
back
route
sai_create_route_entry 9288674231451648 $vr_id 0 192.168.2.10 255.255.255.0
sai_set_route_entry_attribute 9288674231451648 $vr_id 0 192.168.2.10 255.255.255.0 SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID $nh0
back
'''
#
#tcFlushStr: This string contains chain of saiShell commands to be used for get/flush specific
# networking scenario

tcFlushStr = '''
'''
packet_info = scapy_t.Ether(src="00:00:11:00:11:23",dst="00:00:01:11:22:02")/scapy_t.IP(dst="192.168.2.10", src="192.168.1.10")

#
#expectedData: This dictionary expected egress stream for each egress port.
#

expectedData = {
       'expect1':'''
'tap0': [<Ether  dst=00:04:04:04:04:49 src=00:00:01:11:22:02 type=IPv4 |<IP  version=4 ihl=5 tos=0x0 len=20 id=1 flags= frag=0 ttl=63 proto=hopopt chksum=0xf784 src=192.168.1.10 dst=192.168.2.10 |
 '''
}

