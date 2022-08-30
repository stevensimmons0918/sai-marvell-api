#!/usr/bin/env python
#  xpsVxlan.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#*******************************************************************************/

import sys
import time
import os
import re
import readline

#/**********************************************************************************/
# import cmd2 package
#/**********************************************************************************/
dirname, filename = os.path.split(os.path.abspath(__file__))
sys.path.append(dirname + "/../cli")
sys.path.append(dirname + "/../../cli")
from cmd2x import Cmd

import xpShellGlobals

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *


#/**********************************************************************************/
# The class object for xpsVxlan operations
#/**********************************************************************************/

class xpsVxlanObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsVxlanCreateTunnelInterfaceScope
    #/********************************************************************************/
    def do_vxlan_create_tunnel_interface_scope(self, arg):
        '''
         xpsVxlanCreateTunnelInterfaceScope: Enter [ scopeId,lclEpIpAddr,rmtEpIpAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,lclEpIpAddr,rmtEpIpAddr ]')
        else:
            args[0] = int(args[0])
            args[1] = args[1].replace(".", ":").replace(",", ":")
            lclEpIpAddrList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(lclEpIpAddrList)
            lclEpIpAddr = ipv4Addr_t()
            for ix in range(listLen-1, -1, -1):
                lclEpIpAddr[listLen - ix - 1] = int(lclEpIpAddrList[ix])
            args[2] = args[2].replace(".", ":").replace(",", ":")
            rmtEpIpAddrList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(rmtEpIpAddrList)
            rmtEpIpAddr = ipv4Addr_t()
            for ix in range(listLen-1, -1, -1):
                rmtEpIpAddr[listLen - ix - 1] = int(rmtEpIpAddrList[ix])
            tnlIntfId_Ptr_3 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, lclEpIpAddr=%s, rmtEpIpAddr=%s' % (args[0],args[1],args[2]))
            ret = xpsVxlanCreateTunnelInterfaceScope(args[0],lclEpIpAddr,rmtEpIpAddr,tnlIntfId_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('tnlIntfId = %d' % (xpsInterfaceId_tp_value(tnlIntfId_Ptr_3)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(tnlIntfId_Ptr_3))
                pass
            delete_xpsInterfaceId_tp(tnlIntfId_Ptr_3)

    #/********************************************************************************/
    # command for xpsVxlanCreateTunnelInterface
    #/********************************************************************************/
    def do_vxlan_create_tunnel_interface(self, arg):
        '''
         xpsVxlanCreateTunnelInterface: Enter [ lclEpIpAddr,rmtEpIpAddr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ lclEpIpAddr,rmtEpIpAddr ]')
        else:
            args[0] = args[0].replace(".", ":").replace(",", ":")
            lclEpIpAddrList = args[0].strip("'").strip("]").strip("[").split(":")
            listLen = len(lclEpIpAddrList)
            lclEpIpAddr = ipv4Addr_t()
            for ix in range(listLen-1, -1, -1):
                lclEpIpAddr[listLen - ix - 1] = int(lclEpIpAddrList[ix])
            args[1] = args[1].replace(".", ":").replace(",", ":")
            rmtEpIpAddrList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(rmtEpIpAddrList)
            rmtEpIpAddr = ipv4Addr_t()
            for ix in range(listLen-1, -1, -1):
                rmtEpIpAddr[listLen - ix - 1] = int(rmtEpIpAddrList[ix])
            tnlIntfId_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, lclEpIpAddr=%s, rmtEpIpAddr=%s' % (args[0],args[1]))
            ret = xpsVxlanCreateTunnelInterface(lclEpIpAddr,rmtEpIpAddr,tnlIntfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('tnlIntfId = %d' % (xpsInterfaceId_tp_value(tnlIntfId_Ptr_2)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(tnlIntfId_Ptr_2)))
                pass
            delete_xpsInterfaceId_tp(tnlIntfId_Ptr_2)

    #/********************************************************************************/
    # command for xpsVxlanAddTunnelEntry
    #/********************************************************************************/
    def do_vxlan_add_tunnel_entry(self, arg):
        '''
         xpsVxlanAddTunnelEntry: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsVxlanAddTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVxlanRemoveTunnelEntry
    #/********************************************************************************/
    def do_vxlan_remove_tunnel_entry(self, arg):
        '''
         xpsVxlanRemoveTunnelEntry: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsVxlanRemoveTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVxlanDestroyTunnelInterfaceScope
    #/********************************************************************************/
    def do_vxlan_destroy_tunnel_interface_scope(self, arg):
        '''
         xpsVxlanDestroyTunnelInterfaceScope: Enter [ scopeId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsVxlanDestroyTunnelInterfaceScope(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVxlanDestroyTunnelInterface
    #/********************************************************************************/
    def do_vxlan_destroy_tunnel_interface(self, arg):
        '''
         xpsVxlanDestroyTunnelInterface: Enter [ tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ tnlIntfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, tnlIntfId=%d' % (args[0]))
            ret = xpsVxlanDestroyTunnelInterface(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVxlanSetTunnelNextHopData
    #/********************************************************************************/
    def do_vxlan_set_tunnel_next_hop_data(self, arg):
        '''
         xpsVxlanSetTunnelNextHopData: Enter [ devId,tnlIntfId,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,nhId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, nhId=%d' % (args[0],args[1],args[2]))
            ret = xpsVxlanSetTunnelNextHopData(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVxlanUpdateTunnelNextHopData
    #/********************************************************************************/
    def do_vxlan_update_tunnel_next_hop_data(self, arg):
        '''
         xpsVxlanUpdateTunnelNextHopData: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsVxlanUpdateTunnelNextHopData(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVxlanSetUdpPort
    #/********************************************************************************/
    def do_vxlan_set_udp_port(self, arg):
        '''
         xpsVxlanSetUdpPort: Enter [ devId,udpPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,udpPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, udpPort=%d' % (args[0],args[1]))
            ret = xpsVxlanSetUdpPort(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVxlanGetUdpPort
    #/********************************************************************************/
    def do_vxlan_get_udp_port(self, arg):
        '''
         xpsVxlanGetUdpPort: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            udpPort_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsVxlanGetUdpPort(args[0],udpPort_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('udpPort = %d' % (uint32_tp_value(udpPort_Ptr_1)))
                pass
            delete_uint32_tp(udpPort_Ptr_1)

    #/********************************************************************************/
    # command for xpsVxlanAddLocalVtep
    #/********************************************************************************/
    def do_vxlan_add_local_vtep(self, arg):
        '''
         xpsVxlanAddLocalVtep: Enter [ devId,localIp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,localIp ]')
        else:
            args[0] = int(args[0])
            args[1] = args[1].replace(".", ":").replace(",", ":")
            localIpList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(localIpList)
            localIp = ipv4Addr_t()
            for ix in range(listLen-1, -1, -1):
                localIp[listLen - ix - 1] = int(localIpList[ix])
            #print('Input Arguments are, devId=%d, localIp=%s' % (args[0],args[1]))
            ret = xpsVxlanAddLocalVtep(args[0],localIp)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVxlanRemoveLocalVtep
    #/********************************************************************************/
    def do_vxlan_remove_local_vtep(self, arg):
        '''
         xpsVxlanRemoveLocalVtep: Enter [ devId,localIp ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,localIp ]')
        else:
            args[0] = int(args[0])
            args[1] = args[1].replace(".", ":").replace(",", ":")
            localIpList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(localIpList)
            localIp = ipv4Addr_t()
            for ix in range(listLen-1, -1, -1):
                localIp[listLen - ix - 1] = int(localIpList[ix])
            #print('Input Arguments are, devId=%d, localIp=%s' % (args[0],args[1]))
            ret = xpsVxlanRemoveLocalVtep(args[0],localIp)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVxlanSetTunnelConfig
    #/********************************************************************************/
    def do_vxlan_set_tunnel_config(self, arg):
        '''
         xpsVxlanSetTunnelConfig: Enter [ devId,tnlIntfId,paclEn,paclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,paclEn,paclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsVxlanTunnelConfig_t_Ptr = new_xpsVxlanTunnelConfig_tp()
            xpsVxlanTunnelConfig_t_Ptr.paclEn = eval(args[2])
            xpsVxlanTunnelConfig_t_Ptr.paclId = int(args[3])

            #print('Input Arguments are, devId=%d, tnlIntfId=%d, pktCmd=%d, paclEn=%d, paclId=%d' % (args[0],args[1],xpsVxlanTunnelConfig_t_Ptr.pktCmd,xpsVxlanTunnelConfig_t_Ptr.paclEn,xpsVxlanTunnelConfig_t_Ptr.paclId))
            ret = xpsVxlanSetTunnelConfig(args[0],args[1],xpsVxlanTunnelConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('paclEn = %d' % (xpsVxlanTunnelConfig_t_Ptr.paclEn))
                print('paclId = %d' % (xpsVxlanTunnelConfig_t_Ptr.paclId))
                pass
            delete_xpsVxlanTunnelConfig_tp(xpsVxlanTunnelConfig_t_Ptr)

    #/********************************************************************************/
    # command for xpsVxlanGetTunnelConfig
    #/********************************************************************************/
    def do_vxlan_get_tunnel_config(self, arg):
        '''
         xpsVxlanGetTunnelConfig: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsVxlanTunnelConfig_t_Ptr = new_xpsVxlanTunnelConfig_tp()

            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsVxlanGetTunnelConfig(args[0],args[1],xpsVxlanTunnelConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('paclEn = %d' % (xpsVxlanTunnelConfig_t_Ptr.paclEn))
                print('paclId = %d' % (xpsVxlanTunnelConfig_t_Ptr.paclId))
                pass
            delete_xpsVxlanTunnelConfig_tp(xpsVxlanTunnelConfig_t_Ptr)

    #/********************************************************************************/
    # command for xpsVxlanGetTunnelRemoteIp
    #/********************************************************************************/
    def do_vxlan_get_tunnel_remote_ip(self, arg):
        '''
         xpsVxlanGetTunnelRemoteIp: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            rmtIpAddrPtr = ipv4Addr_t()
            uint8_rmtIpAddrPtr = getUint8IpAddrPtr(rmtIpAddrPtr)
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsVxlanGetTunnelRemoteIp(args[0],args[1],uint8_rmtIpAddrPtr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                ipAddr = ''
                for i in range(4):
                    ipAddr += str(ipv4Addr_tp_getitem(uint8_rmtIpAddrPtr, 3-i)) + '.'
                print 'rmtEpIpAddr = ' + ipAddr.strip('.')
                pass
            del(rmtIpAddrPtr)

    #/********************************************************************************/
    # command for xpsVxlanAddVni
    #/********************************************************************************/
    def do_vxlan_add_vni(self, arg):
        '''
         xpsVxlanAddVni: Enter [ devId,vni,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vni,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vni=%d, vlanId=%d' % (args[0],args[1],args[2]))
            ret = xpsVxlanAddVni(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVxlanRemoveVni
    #/********************************************************************************/
    def do_vxlan_remove_vni(self, arg):
        '''
         xpsVxlanRemoveVni: Enter [ devId,vni ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vni ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vni=%d' % (args[0],args[1]))
            ret = xpsVxlanRemoveVni(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVxlanAddMcTunnelEntry
    #/********************************************************************************/
    def do_vxlan_add_mc_tunnel_entry(self, arg):
        '''
         xpsVxlanAddMcTunnelEntry: Enter [ devId,tnlIntfId,lclEpIpAddr,rmtEpIpAddr,l3IntfId,portIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,lclEpIpAddr,rmtEpIpAddr,l3IntfId,portIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = args[2].replace(".", ":").replace(",", ":")
            lclEpIpAddrList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(lclEpIpAddrList)
            lclEpIpAddr = ipv4Addr_t()
            for ix in range(listLen-1, -1, -1):
                lclEpIpAddr[listLen - ix - 1] = int(lclEpIpAddrList[ix])
            args[3] = args[3].replace(".", ":").replace(",", ":")
            rmtEpIpAddrList = args[3].strip("'").strip("]").strip("[").split(":")
            listLen = len(rmtEpIpAddrList)
            rmtEpIpAddr = ipv4Addr_t()
            for ix in range(listLen-1, -1, -1):
                rmtEpIpAddr[listLen - ix - 1] = int(rmtEpIpAddrList[ix])
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, lclEpIpAddr=%s, rmtEpIpAddr=%s, l3IntfId=%d, portIntfId=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsVxlanAddMcTunnelEntry(args[0],args[1],lclEpIpAddr,rmtEpIpAddr,args[4],args[5])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/*****************************************************************************************************/
    # sub-commands to display tables
    #/*****************************************************************************************************/
    def do_display_tables(self, s):
        'Display Table commands'
        i = displayTableCmds()
        i.prompt = self.prompt[:-1]+':displayTables)'
        while True:
            try:
                i.cmdloop()
                break
            except (ValueError, IndexError), e:
                print 'Invalid input format, check help <command> or ? for usage'
                continue
        del i

