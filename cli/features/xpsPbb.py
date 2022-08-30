#!/usr/bin/env python
#  xpsPbb.py
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
# The class object for xpsPbb operations
#/**********************************************************************************/

class xpsPbbObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsPbbCreateTunnelInterface
    #/********************************************************************************/
    def do_pbb_create_tunnel_interface(self, arg):
        '''
         xpsPbbCreateTunnelInterface: Enter [ bSa,bDa,bTag ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ bSa,bDa,bTag ]')
        else:
            args[0] = args[0].replace(".", ":").replace(",", ":")
            bSaList = args[0].strip("'").strip("]").strip("[").split(":")
            listLen = len(bSaList)
            bSa = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                bSa[listLen - ix - 1] = int(bSaList[ix], 16)
            args[1] = args[1].replace(".", ":").replace(",", ":")
            bDaList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(bDaList)
            bDa = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                bDa[listLen - ix - 1] = int(bDaList[ix], 16)
            args[2] = int(args[2])
            pbbTnlId_Ptr_3 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, bSa=%s, bDa=%s, bTag=%d' % (args[0],args[1],args[2]))
            ret = xpsPbbCreateTunnelInterface(bSa,bDa,args[2],pbbTnlId_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pbbTnlId = %d' % (xpsInterfaceId_tp_value(pbbTnlId_Ptr_3)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(pbbTnlId_Ptr_3))
                pass
            delete_xpsInterfaceId_tp(pbbTnlId_Ptr_3)
    #/********************************************************************************/
    # command for xpsPbbCreateTunnelInterfaceScope
    #/********************************************************************************/
    def do_pbb_create_tunnel_interface_scope(self, arg):
        '''
         xpsPbbCreateTunnelInterfaceScope: Enter [ scopeId,bSa,bDa,bTag ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,bSa,bDa,bTag ]')
        else:
            args[0] = int(args[0])
            args[1] = args[1].replace(".", ":").replace(",", ":")
            bSaList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(bSaList)
            bSa = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                bSa[listLen - ix - 1] = int(bSaList[ix], 16)
            args[2] = args[2].replace(".", ":").replace(",", ":")
            bDaList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(bDaList)
            bDa = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                bDa[listLen - ix - 1] = int(bDaList[ix], 16)
            args[3] = int(args[3])
            pbbTnlId_Ptr_4 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, bSa=%s, bDa=%s, bTag=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPbbCreateTunnelInterfaceScope(args[0],bSa,bDa,args[3],pbbTnlId_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pbbTnlId = %d' % (xpsInterfaceId_tp_value(pbbTnlId_Ptr_4)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(pbbTnlId_Ptr_4))
                pass
            delete_xpsInterfaceId_tp(pbbTnlId_Ptr_4)
    #/********************************************************************************/
    # command for xpsPbbAddTunnelEntry
    #/********************************************************************************/
    def do_pbb_add_tunnel_entry(self, arg):
        '''
         xpsPbbAddTunnelEntry: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsPbbAddTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPbbBindPortToTunnel
    #/********************************************************************************/
    def do_pbb_bind_port_to_tunnel(self, arg):
        '''
         xpsPbbBindPortToTunnel: Enter [ devId,pbbTnlId,port ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pbbTnlId,port ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, pbbTnlId=%d, port=%d' % (args[0],args[1],args[2]))
            ret = xpsPbbBindPortToTunnel(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPbbRemoveTunnelEntry
    #/********************************************************************************/
    def do_pbb_remove_tunnel_entry(self, arg):
        '''
         xpsPbbRemoveTunnelEntry: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsPbbRemoveTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPbbDestroyTunnelInterfaceScope
    #/********************************************************************************/
    def do_pbb_destroy_tunnel_interface_scope(self, arg):
        '''
         xpsPbbDestroyTunnelInterfaceScope: Enter [ scopeId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsPbbDestroyTunnelInterfaceScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPbbAddIsid
    #/********************************************************************************/
    def do_pbb_add_isid(self, arg):
        '''
         xpsPbbAddIsid: Enter [ devId,isid,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,isid,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, isid=%d, vlanId=%d' % (args[0],args[1],args[2]))
            ret = xpsPbbAddIsid(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPbbRemoveIsid
    #/********************************************************************************/
    def do_pbb_remove_isid(self, arg):
        '''
         xpsPbbRemoveIsid: Enter [ devId,isid ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,isid ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, isid=%d' % (args[0],args[1]))
            ret = xpsPbbRemoveIsid(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPbbDestroyTunnelInterface
    #/********************************************************************************/
    def do_pbb_destroy_tunnel_interface(self, arg):
        '''
         xpsPbbDestroyTunnelInterface: Enter [ tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ tnlIntfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, tnlIntfId=%d' % (args[0]))
            ret = xpsPbbDestroyTunnelInterface(args[0])
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
