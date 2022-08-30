#!/usr/bin/env python
#  xpsVpnGre.py
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
# The class object for xpsVpnGre operations
#/**********************************************************************************/

class xpsVpnGreObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsVpnGreCreateLooseModeIpTunnelInterface
    #/********************************************************************************/
    def do_vpn_gre_create_loose_mode_ip_tunnel_interface(self, arg):
        '''
         xpsVpnGreCreateLooseModeIpTunnelInterface: Enter [ lclEpIpAddr,rmtEpIpAddr ]
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
            ret = xpsVpnGreCreateLooseModeIpTunnelInterface(lclEpIpAddr,rmtEpIpAddr,tnlIntfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tnlIntfId = %d' % (xpsInterfaceId_tp_value(tnlIntfId_Ptr_2)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(tnlIntfId_Ptr_2))
                pass
            delete_xpsInterfaceId_tp(tnlIntfId_Ptr_2)

    #/********************************************************************************/
    # command for xpsVpnGreCreateLooseModeIpTunnelInterfaceScope
    #/********************************************************************************/
    def do_vpn_gre_create_loose_mode_ip_tunnel_interface_scope(self, arg):
        '''
         xpsVpnGreCreateLooseModeIpTunnelInterfaceScope: Enter [ scopeId,lclEpIpAddr,rmtEpIpAddr ]
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
            ret = xpsVpnGreCreateLooseModeIpTunnelInterfaceScope(args[0],lclEpIpAddr,rmtEpIpAddr,tnlIntfId_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tnlIntfId = %d' % (xpsInterfaceId_tp_value(tnlIntfId_Ptr_3)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(tnlIntfId_Ptr_3))
                pass
            delete_xpsInterfaceId_tp(tnlIntfId_Ptr_3)

    #/********************************************************************************/
    # command for xpsVpnGreCreateStrictModeTunnelInterface
    #/********************************************************************************/
    def do_vpn_gre_create_strict_mode_tunnel_interface(self, arg):
        '''
         xpsVpnGreCreateStrictModeTunnelInterface: Enter [ lclEpIpAddr,rmtEpIpAddr,vpnLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ lclEpIpAddr,rmtEpIpAddr,vpnLabel ]')
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
            args[2] = int(args[2])
            tnlIntfId_Ptr_3 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, lclEpIpAddr=%s, rmtEpIpAddr=%s, vpnLabel=%d' % (args[0],args[1],args[2]))
            ret = xpsVpnGreCreateStrictModeTunnelInterface(lclEpIpAddr,rmtEpIpAddr,args[2],tnlIntfId_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tnlIntfId = %d' % (xpsInterfaceId_tp_value(tnlIntfId_Ptr_3)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(tnlIntfId_Ptr_3))
                pass
            delete_xpsInterfaceId_tp(tnlIntfId_Ptr_3)

    #/********************************************************************************/
    # command for xpsVpnGreCreateStrictModeTunnelInterfaceScope
    #/********************************************************************************/
    def do_vpn_gre_create_strict_mode_tunnel_interface_scope(self, arg):
        '''
         xpsVpnGreCreateStrictModeTunnelInterfaceScope: Enter [ scopeId,lclEpIpAddr,rmtEpIpAddr,vpnLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,lclEpIpAddr,rmtEpIpAddr,vpnLabel ]')
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
            args[3] = int(args[3])
            tnlIntfId_Ptr_4 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, lclEpIpAddr=%s, rmtEpIpAddr=%s, vpnLabel=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsVpnGreCreateStrictModeTunnelInterfaceScope(args[0],lclEpIpAddr,rmtEpIpAddr,args[3],tnlIntfId_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tnlIntfId = %d' % (xpsInterfaceId_tp_value(tnlIntfId_Ptr_4)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(tnlIntfId_Ptr_4))
                pass
            delete_xpsInterfaceId_tp(tnlIntfId_Ptr_4)

    #/********************************************************************************/
    # command for xpsVpnGreAddTunnelEntry
    #/********************************************************************************/
    def do_vpn_gre_add_tunnel_entry(self, arg):
        '''
         xpsVpnGreAddTunnelEntry: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsVpnGreAddTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVpnGreRemoveTunnelEntry
    #/********************************************************************************/
    def do_vpn_gre_remove_tunnel_entry(self, arg):
        '''
         xpsVpnGreRemoveTunnelEntry: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsVpnGreRemoveTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVpnGreDestroyTunnelInterface
    #/********************************************************************************/
    def do_vpn_gre_destroy_tunnel_interface(self, arg):
        '''
         xpsVpnGreDestroyTunnelInterface: Enter [ tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ tnlIntfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, tnlIntfId=%d' % (args[0]))
            ret = xpsVpnGreDestroyTunnelInterface(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVpnGreDestroyTunnelInterfaceScope
    #/********************************************************************************/
    def do_vpn_gre_destroy_tunnel_interface_scope(self, arg):
        '''
         xpsVpnGreDestroyTunnelInterfaceScope: Enter [ scopeId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsVpnGreDestroyTunnelInterfaceScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVpnGreSetTunnelNextHopData
    #/********************************************************************************/
    def do_vpn_gre_set_tunnel_next_hop_data(self, arg):
        '''
         xpsVpnGreSetTunnelNextHopData: Enter [ devId,tnlIntfId,nhId ]
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
            ret = xpsVpnGreSetTunnelNextHopData(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVpnGreUpdateTunnelNextHopData
    #/********************************************************************************/
    def do_vpn_gre_update_tunnel_next_hop_data(self, arg):
        '''
         xpsVpnGreUpdateTunnelNextHopData: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsVpnGreUpdateTunnelNextHopData(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVpnGreSetTunnelConfig
    #/********************************************************************************/
    def do_vpn_gre_set_tunnel_config(self, arg):
        '''
         xpsVpnGreSetTunnelConfig: Enter [ devId,tnlIntfId,baclEn,baclId,raclEn,raclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,baclEn,baclId,raclEn,raclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsVpnGreTunnelConfig_t_Ptr = new_xpsVpnGreTunnelConfig_tp()
            xpsVpnGreTunnelConfig_t_Ptr.baclEn = int(args[2])
            xpsVpnGreTunnelConfig_t_Ptr.baclId = int(args[3])
            xpsVpnGreTunnelConfig_t_Ptr.raclEn = int(args[4])
            xpsVpnGreTunnelConfig_t_Ptr.raclId = int(args[5])
            
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, baclEn=%d, baclId=%d, raclEn=%d, raclId=%d' % (args[0],args[1],xpsVpnGreTunnelConfig_t_Ptr.baclEn,xpsVpnGreTunnelConfig_t_Ptr.baclId,xpsVpnGreTunnelConfig_t_Ptr.raclEn,xpsVpnGreTunnelConfig_t_Ptr.raclId))
            ret = xpsVpnGreSetTunnelConfig(args[0],args[1],xpsVpnGreTunnelConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('baclEn = %d' % (xpsVpnGreTunnelConfig_t_Ptr.baclEn))
                print('baclId = %d' % (xpsVpnGreTunnelConfig_t_Ptr.baclId))
                print('raclEn = %d' % (xpsVpnGreTunnelConfig_t_Ptr.raclEn))
                print('raclId = %d' % (xpsVpnGreTunnelConfig_t_Ptr.raclId))
                pass
            delete_xpsVpnGreTunnelConfig_tp(xpsVpnGreTunnelConfig_t_Ptr)
    #/********************************************************************************/
    # command for xpsVpnGreGetTunnelConfig
    #/********************************************************************************/
    def do_vpn_gre_get_tunnel_config(self, arg):
        '''
         xpsVpnGreGetTunnelConfig: Enter [ devId,tnlIntfId,baclEn,baclId,raclEn,raclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,baclEn,baclId,raclEn,raclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsVpnGreTunnelConfig_t_Ptr = new_xpsVpnGreTunnelConfig_tp()
            xpsVpnGreTunnelConfig_t_Ptr.baclEn = int(args[2])
            xpsVpnGreTunnelConfig_t_Ptr.baclId = int(args[3])
            xpsVpnGreTunnelConfig_t_Ptr.raclEn = int(args[4])
            xpsVpnGreTunnelConfig_t_Ptr.raclId = int(args[5])
            
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, baclEn=%d, baclId=%d, raclEn=%d, raclId=%d' % (args[0],args[1],xpsVpnGreTunnelConfig_t_Ptr.baclEn,xpsVpnGreTunnelConfig_t_Ptr.baclId,xpsVpnGreTunnelConfig_t_Ptr.raclEn,xpsVpnGreTunnelConfig_t_Ptr.raclId))
            ret = xpsVpnGreGetTunnelConfig(args[0],args[1],xpsVpnGreTunnelConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('baclEn = %d' % (xpsVpnGreTunnelConfig_t_Ptr.baclEn))
                print('baclId = %d' % (xpsVpnGreTunnelConfig_t_Ptr.baclId))
                print('raclEn = %d' % (xpsVpnGreTunnelConfig_t_Ptr.raclEn))
                print('raclId = %d' % (xpsVpnGreTunnelConfig_t_Ptr.raclId))
                pass
            delete_xpsVpnGreTunnelConfig_tp(xpsVpnGreTunnelConfig_t_Ptr)
    #/********************************************************************************/
    # command for xpsVpnGreGetTunnelRemoteIp
    #/********************************************************************************/
    def do_vpn_gre_get_tunnel_remote_ip(self, arg):
        '''
         xpsVpnGreGetTunnelRemoteIp: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            rmtEpIpAddr_Ptr_2 = ipv4Addr_t()
            final_rmtEpIpAddr_Ptr = getUint8IpAddrPtr(rmtEpIpAddr_Ptr_2)
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsVpnGreGetTunnelRemoteIp(args[0],args[1],final_rmtEpIpAddr_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                ipAddr = ''
                for i in range(4):
                    ipAddr += str(ipv4Addr_tp_getitem(final_rmtEpIpAddr_Ptr, 3-i)) + '.'
                print 'rmtEpIpAddr = ' + ipAddr.strip('.')
                pass
            del(rmtEpIpAddr_Ptr_2)
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
