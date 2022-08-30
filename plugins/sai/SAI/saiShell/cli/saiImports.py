#!/usr/bin python
#  pkgImports.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#********************************************************************************/

import sys
import time
import os
import readline

dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + ".")
sys.path.append(dirname + "./sai")
sys.path.append(dirname + "./../cli")
sys.path.append(dirname + "./../cli/sai")
sys.path.append(dirname + "./../../cli")
sys.path.append(dirname + "./../../cli/sai")
sys.path.append(dirname + "/sai")
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../cli/sai")
sys.path.append(dirname + "/../../cli")
sys.path.append(dirname + "/../../cli/sai")
#for saiShellFdb.py
sys.path.append(dirname + ".")
sys.path.append(dirname + "/../../xpSai")
sys.path.append(dirname + "/../../xpSai/include")
sys.path.append(dirname + "/../../xpSai/sai")
sys.path.append(dirname + "/../../xpSai/sai/inc")
sys.path.append(dirname + "/../genScripts/saiShellPyWrappers")


from cmd2x import Cmd
from buildTarget import *

from saiShellFdb import *
from saiShellBridge import *
from saiShellVlan import *
from saiShellStp import *
from saiShellLag import *
from saiShellPort import *
from saiShellSwitch import *
from saiShellMirror import *
from saiShellAcl import *
from saiShellQueue import *
from saiShellWred import *
from saiShellPolicer import *
from saiShellScheduler import *
from saiShellBuffer import *
from saiShellQosmap import *
from saiShellUdf import *
from saiShellHash import *
from saiShellNeighbor import *
from saiShellVirtualrouter import *
from saiShellNexthop import *
from saiShellNexthopgroup import *
from saiShellRoute import *
from saiShellRouterinterface import *
from saiShellSchedulergroup import *
from saiShellHostif import *
from saiShellSamplepacket import *
from saiShellTunnel import *
from saiShellL2mc import *
from saiShellL2mcgroup import *
from saiShellMcastfdb import *
from saiShellIpmc import *
from saiShellIpmcgroup import *
from saiShellRpfgroup import *
from saiShellCounter import *


#/*********************************************************************************************************/
# The class object for SAI Layer operations
#/*********************************************************************************************************/
class saiLayerOperationCmds(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # sub-commands for SAI layer ACL functions
    #/*****************************************************************************************************/
    def do_acl(self, s):
        'SAI ACL Commands'
        i = saiShellAclObj()
        i.prompt = self.prompt[:-1]+':acl)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer BUFFER functions
    #/*****************************************************************************************************/
    def do_buffer(self, s):
        'SAI BUFFER Commands'
        i = saiShellBufferObj()
        i.prompt = self.prompt[:-1]+':buffer)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer FDB functions
    #/*****************************************************************************************************/
    def do_fdb(self, s):
        'SAI FDB Commands'
        i = saiShellFdbObj()
        i.prompt = self.prompt[:-1]+':fdb)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer BRIDGE functions
    #/*****************************************************************************************************/
    def do_bridge(self, s):
        'SAI BRIDGE Commands'
        i = saiShellBridgeObj()
        i.prompt = self.prompt[:-1]+':bridge)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i


    #/*****************************************************************************************************/
    # sub-commands for SAI layer HASH functions
    #/*****************************************************************************************************/
    def do_hash(self, s):
        'SAI HASH Commands'
        i = saiShellHashObj()
        i.prompt = self.prompt[:-1]+':hash)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer HOSTIF functions
    #/*****************************************************************************************************/
    def do_hostif(self, s):
        'SAI HOSTIF Commands'
        i = saiShellHostifObj()
        i.prompt = self.prompt[:-1]+':hostif)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer LAG functions
    #/*****************************************************************************************************/
    def do_lag(self, s):
        'SAI LAG Commands'
        i = saiShellLagObj()
        i.prompt = self.prompt[:-1]+':lag)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer MIRROR functions
    #/*****************************************************************************************************/
    def do_mirror(self, s):
        'SAI MIRROR Commands'
        i = saiShellMirrorObj()
        i.prompt = self.prompt[:-1]+':mirror)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer Neighbor functions
    #/*****************************************************************************************************/
    def do_neighbor(self, s):
        'SAI NEIGHBOR Commands'
        i = saiShellNeighborObj()
        i.prompt = self.prompt[:-1]+':neighbor)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer NEXTHOPGROUP functions
    #/*****************************************************************************************************/
    def do_nexthopgroup(self, s):
        'SAI NEXTHOPGROUP Commands'
        i = saiShellNexthopgroupObj()
        i.prompt = self.prompt[:-1]+':nexthopgroup)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer NEXTHOP functions
    #/*****************************************************************************************************/
    def do_nexthop(self, s):
        'SAI NEXTHOP Commands'
        i = saiShellNexthopObj()
        i.prompt = self.prompt[:-1]+':nexthop)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer OBJECT functions
    #/*****************************************************************************************************/
    def do_object(self, s):
        'SAI OBJECT Commands'
        i = saiShellObjectObj()
        i.prompt = self.prompt[:-1]+':object)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer POLICER functions
    #/*****************************************************************************************************/
    def do_policer(self, s):
        'SAI POLICER Commands'
        i = saiShellPolicerObj()
        i.prompt = self.prompt[:-1]+':policer)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer PORT functions
    #/*****************************************************************************************************/
    def do_port(self, s):
        'SAI PORT Commands'
        i = saiShellPortObj()
        i.prompt = self.prompt[:-1]+':port)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer QOSMAP functions
    #/*****************************************************************************************************/
    def do_qosmap(self, s):
        'SAI QOSMAP Commands'
        i = saiShellQosmapObj()
        i.prompt = self.prompt[:-1]+':qosmap)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer QUEUE functions
    #/*****************************************************************************************************/
    def do_queue(self, s):
        'SAI QUEUE Commands'
        i = saiShellQueueObj()
        i.prompt = self.prompt[:-1]+':queue)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer ROUTE functions
    #/*****************************************************************************************************/
    def do_route(self, s):
        'SAI ROUTE Commands'
        i = saiShellRouteObj()
        i.prompt = self.prompt[:-1]+':route)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer VIRTUALROUTER functions
    #/*****************************************************************************************************/
    def do_virtualrouter(self, s):
        'SAI VIRTUALROUTER Commands'
        i = saiShellVirtualrouterObj()
        i.prompt = self.prompt[:-1]+':virtualrouter)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer ROUTERINTERFACE functions
    #/*****************************************************************************************************/
    def do_routerinterface(self, s):
        'SAI ROUTERINTERFACE Commands'
        i = saiShellRouterinterfaceObj()
        i.prompt = self.prompt[:-1]+':routerinterface)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer SAMPLEPACKET functions
    #/*****************************************************************************************************/
    def do_samplepacket(self, s):
        'SAI SAMPLEPACKET Commands'
        i = saiShellSamplepacketObj()
        i.prompt = self.prompt[:-1]+':samplepacket)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer SCHEDULERGROUP functions
    #/*****************************************************************************************************/
    def do_schedulergroup(self, s):
        'SAI SCHEDULERGROUP Commands'
        i = saiShellSchedulergroupObj()
        i.prompt = self.prompt[:-1]+':schedulergroup)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer SCHEDULER functions
    #/*****************************************************************************************************/
    def do_scheduler(self, s):
        'SAI SCHEDULER Commands'
        i = saiShellSchedulerObj()
        i.prompt = self.prompt[:-1]+':scheduler)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer STATUS functions
    #/*****************************************************************************************************/
    def do_status(self, s):
        'SAI STATUS Commands'
        i = saiShellStatusObj()
        i.prompt = self.prompt[:-1]+':status)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer STP functions
    #/*****************************************************************************************************/
    def do_stp(self, s):
        'SAI STP Commands'
        i = saiShellStpObj()
        i.prompt = self.prompt[:-1]+':stp)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer SWITCH functions
    #/*****************************************************************************************************/
    def do_switch(self, s):
        'SAI SWITCH Commands'
        i = saiShellSwitchObj()
        i.prompt = self.prompt[:-1]+':switch)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer TUNNEL functions
    #/*****************************************************************************************************/
    def do_tunnel(self, s):
        'SAI TUNNEL Commands'
        i = saiShellTunnelObj()
        i.prompt = self.prompt[:-1]+':tunnel)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer TYPES functions
    #/*****************************************************************************************************/
    def do_types(self, s):
        'SAI TYPES Commands'
        i = saiShellTypesObj()
        i.prompt = self.prompt[:-1]+':types)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer UDF functions
    #/*****************************************************************************************************/
    def do_udf(self, s):
        'SAI UDF Commands'
        i = saiShellUdfObj()
        i.prompt = self.prompt[:-1]+':udf)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer VLAN functions
    #/*****************************************************************************************************/
    def do_vlan(self, s):
        'SAI VLAN Commands'
        i = saiShellVlanObj()
        i.prompt = self.prompt[:-1]+':vlan)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer WRED functions
    #/*****************************************************************************************************/
    def do_wred(self, s):
        'SAI WRED Commands'
        i = saiShellWredObj()
        i.prompt = self.prompt[:-1]+':wred)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer L2 Multicast functions
    #/*****************************************************************************************************/
    def do_l2mc(self, s):
        'SAI L2Mc Commands'
        i = saiShellL2mcObj()
        i.prompt = self.prompt[:-1]+':l2mc)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer L2 Multicast Group functions
    #/*****************************************************************************************************/
    def do_l2mcgroup(self, s):
        'SAI L2McGroup Commands'
        i = saiShellL2mcgroupObj()
        i.prompt = self.prompt[:-1]+':l2mcgroup)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer Multicast FDB functions
    #/*****************************************************************************************************/
    def do_mcastfdb(self, s):
        'SAI McastFDB Commands'
        i = saiShellMcastfdbObj()
        i.prompt = self.prompt[:-1]+':mcastfdb)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer IP Multicast functions
    #/*****************************************************************************************************/
    def do_ipmc(self, s):
        'SAI Ipmc Commands'
        i = saiShellIpmcObj()
        i.prompt = self.prompt[:-1]+':ipmc)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer IP Multicast Group functions
    #/*****************************************************************************************************/
    def do_ipmcgroup(self, s):
        'SAI IpmcGroup Commands'
        i = saiShellIpmcgroupObj()
        i.prompt = self.prompt[:-1]+':ipmcgroup)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer RPF Group functions
    #/*****************************************************************************************************/
    def do_rpfgroup(self, s):
        'SAI RpfGroup Commands'
        i = saiShellRpfgroupObj()
        i.prompt = self.prompt[:-1]+':rpfgroup)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i

    #/*****************************************************************************************************/
    # sub-commands for SAI layer Counter functions
    #/*****************************************************************************************************/
    def do_counter(self, s):
        'SAI Counter Commands'
        i = saiShellCounterObj()
        i.prompt = self.prompt[:-1]+':counter)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid inputs format, check help <command> or ? for usage'
                continue
        del i
