#!/usr/bin/env python
#  xpsErspanGre.py
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
# The class object for xpsErspanGre operations
#/**********************************************************************************/

class xpsErspanGreObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsErspanGreCreateTunnelInterfaceScope
    #/********************************************************************************/
    def do_erspan_gre_create_tunnel_interface_scope(self, arg):
        '''
         xpsErspanGreCreateTunnelInterfaceScope: Enter [ scopeId,lclEpIpAddr,rmtEpIpAddr ]
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
            ret = xpsErspanGreCreateTunnelInterfaceScope(args[0],lclEpIpAddr,rmtEpIpAddr,tnlIntfId_Ptr_3)
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
    # command for xpsErspanGreAddTunnelEntry
    #/********************************************************************************/
    def do_erspan_gre_add_tunnel_entry(self, arg):
        '''
         xpsErspanGreAddTunnelEntry: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsErspanGreAddTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsErspanGreRemoveTunnelEntry
    #/********************************************************************************/
    def do_erspan_gre_remove_tunnel_entry(self, arg):
        '''
         xpsErspanGreRemoveTunnelEntry: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsErspanGreRemoveTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsErspanGreDestroyTunnelInterfaceScope
    #/********************************************************************************/
    def do_erspan_gre_destroy_tunnel_interface_scope(self, arg):
        '''
         xpsErspanGreDestroyTunnelInterfaceScope: Enter [ scopeId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsErspanGreDestroyTunnelInterfaceScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsErspanGreSetTunnelNextHopData
    #/********************************************************************************/
    def do_erspan_gre_set_tunnel_next_hop_data(self, arg):
        '''
         xpsErspanGreSetTunnelNextHopData: Enter [ devId,tnlIntfId,nhId ]
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
            ret = xpsErspanGreSetTunnelNextHopData(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsErspanGreUpdateTunnelNextHopData
    #/********************************************************************************/
    def do_erspan_gre_update_tunnel_next_hop_data(self, arg):
        '''
         xpsErspanGreUpdateTunnelNextHopData: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsErspanGreUpdateTunnelNextHopData(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsErspanGreCreateTunnelInterface
    #/********************************************************************************/
    def do_erspan_gre_create_tunnel_interface(self, arg):
        '''
         xpsErspanGreCreateTunnelInterface: Enter [ lclEpIpAddr,rmtEpIpAddr ]
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
            ret = xpsErspanGreCreateTunnelInterface(lclEpIpAddr,rmtEpIpAddr,tnlIntfId_Ptr_2)
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
    # command for xpsErspanGreDestroyTunnelInterface
    #/********************************************************************************/
    def do_erspan_gre_destroy_tunnel_interface(self, arg):
        '''
         xpsErspanGreDestroyTunnelInterface: Enter [ tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ tnlIntfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, tnlIntfId=%d' % (args[0]))
            ret = xpsErspanGreDestroyTunnelInterface(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
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
