#!/usr/bin/env python
#  xpsIpGre.py
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
# The class object for xpsIpGre operations
#/**********************************************************************************/

class xpsIpGreObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsIpGreCreateTunnelInterface
    #/********************************************************************************/
    def do_ip_gre_create_tunnel_interface(self, arg):
        '''
         xpsIpGreCreateTunnelInterface: Enter [ lclEpIpAddr,rmtEpIpAddr ]
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
            ret = xpsIpGreCreateTunnelInterface(lclEpIpAddr,rmtEpIpAddr,tnlIntfId_Ptr_2)
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
    # command for xpsIpGreCreateTunnelInterfaceScope
    #/********************************************************************************/
    def do_ip_gre_create_tunnel_interface_scope(self, arg):
        '''
         xpsIpGreCreateTunnelInterfaceScope: Enter [ scopeId,lclEpIpAddr,rmtEpIpAddr ]
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
            ret = xpsIpGreCreateTunnelInterfaceScope(args[0],lclEpIpAddr,rmtEpIpAddr,tnlIntfId_Ptr_3)
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
    # command for xpsIpGreAddTunnelEntry
    #/********************************************************************************/
    def do_ip_gre_add_tunnel_entry(self, arg):
        '''
         xpsIpGreAddTunnelEntry: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsIpGreAddTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIpGreRemoveTunnelEntry
    #/********************************************************************************/
    def do_ip_gre_remove_tunnel_entry(self, arg):
        '''
         xpsIpGreRemoveTunnelEntry: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsIpGreRemoveTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIpGreDestroyTunnelInterface
    #/********************************************************************************/
    def do_ip_gre_destroy_tunnel_interface(self, arg):
        '''
         xpsIpGreDestroyTunnelInterface: Enter [ tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ tnlIntfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, tnlIntfId=%d' % (args[0]))
            ret = xpsIpGreDestroyTunnelInterface(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIpGreDestroyTunnelInterfaceScope
    #/********************************************************************************/
    def do_ip_gre_destroy_tunnel_interface_scope(self, arg):
        '''
         xpsIpGreDestroyTunnelInterfaceScope: Enter [ scopeId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsIpGreDestroyTunnelInterfaceScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIpGreSetTunnelNextHopData
    #/********************************************************************************/
    def do_ip_gre_set_tunnel_next_hop_data(self, arg):
        '''
         xpsIpGreSetTunnelNextHopData: Enter [ devId,tnlIntfId,nhId ]
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
            ret = xpsIpGreSetTunnelNextHopData(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIpGreUpdateTunnelNextHopData
    #/********************************************************************************/
    def do_ip_gre_update_tunnel_next_hop_data(self, arg):
        '''
         xpsIpGreUpdateTunnelNextHopData: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsIpGreUpdateTunnelNextHopData(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIpGreSetTunnelConfig
    #/********************************************************************************/
    def do_ip_gre_set_tunnel_config(self, arg):
        '''
         xpsIpGreSetTunnelConfig: Enter [ devId,tnlIntfId,baclEn,baclId,raclEn,raclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,baclEn,baclId,raclEn,raclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsIpGreTunnelConfig_t_Ptr = new_xpsIpGreTunnelConfig_tp()
            xpsIpGreTunnelConfig_t_Ptr.baclEn = int(args[2])
            xpsIpGreTunnelConfig_t_Ptr.baclId = int(args[3])
            xpsIpGreTunnelConfig_t_Ptr.raclEn = int(args[4])
            xpsIpGreTunnelConfig_t_Ptr.raclId = int(args[5])
            
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, pktCmd=%d, baclEn=%d, baclId=%d, raclEn=%d, raclId=%d' % (args[0],args[1],xpsIpGreTunnelConfig_t_Ptr.pktCmd,xpsIpGreTunnelConfig_t_Ptr.baclEn,xpsIpGreTunnelConfig_t_Ptr.baclId,xpsIpGreTunnelConfig_t_Ptr.raclEn,xpsIpGreTunnelConfig_t_Ptr.raclId))
            ret = xpsIpGreSetTunnelConfig(args[0],args[1],xpsIpGreTunnelConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('baclEn = %d' % (xpsIpGreTunnelConfig_t_Ptr.baclEn))
                print('baclId = %d' % (xpsIpGreTunnelConfig_t_Ptr.baclId))
                print('raclEn = %d' % (xpsIpGreTunnelConfig_t_Ptr.raclEn))
                print('raclId = %d' % (xpsIpGreTunnelConfig_t_Ptr.raclId))
                pass
            delete_xpsIpGreTunnelConfig_tp(xpsIpGreTunnelConfig_t_Ptr)

    #/********************************************************************************/
    # command for xpsIpGreGetTunnelConfig
    #/********************************************************************************/
    def do_ip_gre_get_tunnel_config(self, arg):
        '''
         xpsIpGreGetTunnelConfig: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsIpGreTunnelConfig_t_Ptr = new_xpsIpGreTunnelConfig_tp()
            
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsIpGreGetTunnelConfig(args[0],args[1],xpsIpGreTunnelConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('baclEn = %d' % (xpsIpGreTunnelConfig_t_Ptr.baclEn))
                print('baclId = %d' % (xpsIpGreTunnelConfig_t_Ptr.baclId))
                print('raclEn = %d' % (xpsIpGreTunnelConfig_t_Ptr.raclEn))
                print('raclId = %d' % (xpsIpGreTunnelConfig_t_Ptr.raclId))
                pass
            delete_xpsIpGreTunnelConfig_tp(xpsIpGreTunnelConfig_t_Ptr)

    #/********************************************************************************/
    # command for xpsIpGreGetTunnelRemoteIp
    #/********************************************************************************/
    def do_ip_gre_get_tunnel_remote_ip(self, arg):
        '''
         xpsIpGreGetTunnelRemoteIp: Enter [ devId,tnlIntfId ]
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
            ret = xpsIpGreGetTunnelRemoteIp(args[0],args[1],final_rmtEpIpAddr_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                ipAddr = ''
                for i in range(4):
                    ipAddr += str(ipv4Addr_tp_getitem(final_rmtEpIpAddr_Ptr,3-i)) + '.'
                print 'rmtEpIpAddr = ' + ipAddr.strip('.')
                pass
            del rmtEpIpAddr_Ptr_2

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

