#!/usr/bin/env python
#  xpsNvgre.py
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
# The class object for xpsNvgre operations
#/**********************************************************************************/

class xpsNvgreObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsNvgreCreateTunnelInterfaceScope
    #/********************************************************************************/
    def do_nvgre_create_tunnel_interface_scope(self, arg):
        '''
         xpsNvgreCreateTunnelInterfaceScope: Enter [ scopeId,lclEpIpAddr,rmtEpIpAddr ]
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
            ret = xpsNvgreCreateTunnelInterfaceScope(args[0],lclEpIpAddr,rmtEpIpAddr,tnlIntfId_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('tnlIntfId = %d' % (xpsInterfaceId_tp_value(tnlIntfId_Ptr_3)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(tnlIntfId_Ptr_3)))
                pass
            delete_xpsInterfaceId_tp(tnlIntfId_Ptr_3)
			
    #/********************************************************************************/
    # command for xpsNvgreCreateTunnelInterface
    #/********************************************************************************/
    def do_nvgre_create_tunnel_interface(self, arg):
        '''
         xpsNvgreCreateTunnelInterface: Enter [ lclEpIpAddr,rmtEpIpAddr ]
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
            ret = xpsNvgreCreateTunnelInterface(lclEpIpAddr,rmtEpIpAddr,tnlIntfId_Ptr_2)
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
    # command for xpsNvgreAddTunnelEntry
    #/********************************************************************************/
    def do_nvgre_add_tunnel_entry(self, arg):
        '''
         xpsNvgreAddTunnelEntry: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsNvgreAddTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNvgreRemoveTunnelEntry
    #/********************************************************************************/
    def do_nvgre_remove_tunnel_entry(self, arg):
        '''
         xpsNvgreRemoveTunnelEntry: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsNvgreRemoveTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNvgreDestroyTunnelInterfaceScope
    #/********************************************************************************/
    def do_nvgre_destroy_tunnel_interface_scope(self, arg):
        '''
         xpsNvgreDestroyTunnelInterfaceScope: Enter [ scopeId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsNvgreDestroyTunnelInterfaceScope(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
				 
    #/********************************************************************************/
    # command for xpsNvgreDestroyTunnelInterface
    #/********************************************************************************/
    def do_nvgre_destroy_tunnel_interface(self, arg):
        '''
         xpsNvgreDestroyTunnelInterface: Enter [ tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ tnlIntfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, tnlIntfId=%d' % (args[0]))
            ret = xpsNvgreDestroyTunnelInterface(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNvgreSetTunnelNextHopData
    #/********************************************************************************/
    def do_nvgre_set_tunnel_next_hop_data(self, arg):
        '''
         xpsNvgreSetTunnelNextHopData: Enter [ devId,tnlIntfId,nhId ]
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
            ret = xpsNvgreSetTunnelNextHopData(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNvgreUpdateTunnelNextHopData
    #/********************************************************************************/
    def do_nvgre_update_tunnel_next_hop_data(self, arg):
        '''
         xpsNvgreUpdateTunnelNextHopData: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsNvgreUpdateTunnelNextHopData(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNvgreAddLocalNve
    #/********************************************************************************/
    def do_nvgre_add_local_nve(self, arg):
        '''
         xpsNvgreAddLocalNve: Enter [ devId,localIp ]
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
            ret = xpsNvgreAddLocalNve(args[0],localIp)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNvgreRemoveLocalNve
    #/********************************************************************************/
    def do_nvgre_remove_local_nve(self, arg):
        '''
         xpsNvgreRemoveLocalNve: Enter [ devId,localIp ]
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
            ret = xpsNvgreRemoveLocalNve(args[0],localIp)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNvgreSetTunnelConfig
    #/********************************************************************************/
    def do_nvgre_set_tunnel_config(self, arg):
        '''
         xpsNvgreSetTunnelConfig: Enter [ devId,tnlIntfId,paclEn,paclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,paclEn,paclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsNvgreTunnelConfig_t_Ptr = new_xpsNvgreTunnelConfig_tp()
            xpsNvgreTunnelConfig_t_Ptr.paclEn = int(args[2])
            xpsNvgreTunnelConfig_t_Ptr.paclId = int(args[3])
            
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, pktCmd=%d, paclEn=%d, paclId=%d' % (args[0],args[1],xpsNvgreTunnelConfig_t_Ptr.pktCmd,xpsNvgreTunnelConfig_t_Ptr.paclEn,xpsNvgreTunnelConfig_t_Ptr.paclId))
            ret = xpsNvgreSetTunnelConfig(args[0],args[1],xpsNvgreTunnelConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
#                print('pktCmd = %d' % (xpsNvgreTunnelConfig_t_Ptr.pktCmd))
                print('paclEn = %d' % (xpsNvgreTunnelConfig_t_Ptr.paclEn))
                print('paclId = %d' % (xpsNvgreTunnelConfig_t_Ptr.paclId))
                pass
            delete_xpsNvgreTunnelConfig_tp(xpsNvgreTunnelConfig_t_Ptr)

    #/********************************************************************************/
    # command for xpsNvgreGetTunnelConfig
    #/********************************************************************************/
    def do_nvgre_get_tunnel_config(self, arg):
        '''
         xpsNvgreGetTunnelConfig: Enter [ devId,tnlIntfId]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsNvgreTunnelConfig_t_Ptr = new_xpsNvgreTunnelConfig_tp()

            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsNvgreGetTunnelConfig(args[0],args[1],xpsNvgreTunnelConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('pktCmd = %d' % (xpsNvgreTunnelConfig_t_Ptr.pktCmd))
                print('paclEn = %d' % (xpsNvgreTunnelConfig_t_Ptr.paclEn))
                print('paclId = %d' % (xpsNvgreTunnelConfig_t_Ptr.paclId))
                pass
            delete_xpsNvgreTunnelConfig_tp(xpsNvgreTunnelConfig_t_Ptr)

    #/********************************************************************************/
    # command for xpsNvgreGetTunnelRemoteIp
    #/********************************************************************************/
    def do_nvgre_get_tunnel_remote_ip(self, arg):
        '''
         xpsNvgreGetTunnelRemoteIp: Enter [ devId,tnlIntfId ]
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
            ret = xpsNvgreGetTunnelRemoteIp(args[0],args[1],final_rmtEpIpAddr_Ptr)
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

    #/********************************************************************************/
    # command for xpsNvgreAddTni
    #/********************************************************************************/
    def do_nvgre_add_tni(self, arg):
        '''
         xpsNvgreAddTni: Enter [ devId,tni,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tni,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, tni=%d, vlanId=%d' % (args[0],args[1],args[2]))
            ret = xpsNvgreAddTni(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNvgreRemoveTni
    #/********************************************************************************/
    def do_nvgre_remove_tni(self, arg):
        '''
         xpsNvgreRemoveTni: Enter [ devId,tni ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tni ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tni=%d' % (args[0],args[1]))
            ret = xpsNvgreRemoveTni(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsNvgreAddMcTunnelEntry
    #/********************************************************************************/
    def do_nvgre_add_mc_tunnel_entry(self, arg):
        '''
         xpsNvgreAddMcTunnelEntry: Enter [ devId,tnlIntfId,lclEpIpAddr,rmtEpIpAddr,l3IntfId,portIntfId ]
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
            ret = xpsNvgreAddMcTunnelEntry(args[0],args[1],lclEpIpAddr,rmtEpIpAddr,args[4],args[5])
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

