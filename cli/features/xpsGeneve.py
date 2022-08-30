#!/usr/bin/env python
#  xpsGeneve.py
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
# The class object for xpsGeneve operations
#/**********************************************************************************/

class xpsGeneveObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsGeneveRemoveTunnelEntry
    #/********************************************************************************/
    def do_geneve_remove_tunnel_entry(self, arg):
        '''
         xpsGeneveRemoveTunnelEntry: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsGeneveRemoveTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGeneveCreateTunnelInterface
    #/********************************************************************************/
    def do_geneve_create_tunnel_interface(self, arg):
        '''
         xpsGeneveCreateTunnelInterface: Enter [ lclEpIpAddr,rmtEpIpAddr,optionFormat ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ lclEpIpAddr,rmtEpIpAddr,optionFormat ]')
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
            args[2] = eval(args[2])
            intfId_Ptr_3 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, lclEpIpAddr=%s, rmtEpIpAddr=%s, optionFormat=%d' % (args[0],args[1],args[2]))
            ret = xpsGeneveCreateTunnelInterface(lclEpIpAddr,rmtEpIpAddr,args[2],intfId_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_3)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(intfId_Ptr_3)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_3)

    #/********************************************************************************/
    # command for xpsGeneveCreateTunnelInterfaceScope
    #/********************************************************************************/
    def do_geneve_create_tunnel_interface_scope(self, arg):
        '''
         xpsGeneveCreateTunnelInterfaceScope: Enter [ scopeId,lclEpIpAddr,rmtEpIpAddr,optionFormat ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,lclEpIpAddr,rmtEpIpAddr,optionFormat ]')
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
            args[3] = eval(args[3])
            intfId_Ptr_4 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, lclEpIpAddr=%s, rmtEpIpAddr=%s, optionFormat=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsGeneveCreateTunnelInterfaceScope(args[0],lclEpIpAddr,rmtEpIpAddr,args[3],intfId_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_4)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(intfId_Ptr_4))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_4)

    #/********************************************************************************/
    # command for xpsGeneveAddTunnelEntry
    #/********************************************************************************/
    def do_geneve_add_tunnel_entry(self, arg):
        '''
         xpsGeneveAddTunnelEntry: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsGeneveAddTunnelEntry(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGeneveDestroyTunnelInterface
    #/********************************************************************************/
    def do_geneve_destroy_tunnel_interface(self, arg):
        '''
         xpsGeneveDestroyTunnelInterface: Enter [ tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ tnlIntfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, tnlIntfId=%d' % (args[0]))
            ret = xpsGeneveDestroyTunnelInterface(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGeneveDestroyTunnelInterfaceScope
    #/********************************************************************************/
    def do_geneve_destroy_tunnel_interface_scope(self, arg):
        '''
         xpsGeneveDestroyTunnelInterfaceScope: Enter [ scopeId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsGeneveDestroyTunnelInterfaceScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGeneveSetTunnelNextHopData
    #/********************************************************************************/
    def do_geneve_set_tunnel_next_hop_data(self, arg):
        '''
         xpsGeneveSetTunnelNextHopData: Enter [ devId,tnlIntfId,nhId ]
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
            ret = xpsGeneveSetTunnelNextHopData(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGeneveUpdateTunnelNextHopData
    #/********************************************************************************/
    def do_geneve_update_tunnel_next_hop_data(self, arg):
        '''
         xpsGeneveUpdateTunnelNextHopData: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsGeneveUpdateTunnelNextHopData(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGeneveAddLocalEndpoint
    #/********************************************************************************/
    def do_geneve_add_local_endpoint(self, arg):
        '''
         xpsGeneveAddLocalEndpoint: Enter [ devId,localIp ]
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
            ret = xpsGeneveAddLocalEndpoint(args[0],localIp)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGeneveRemoveLocalEndpoint
    #/********************************************************************************/
    def do_geneve_remove_local_endpoint(self, arg):
        '''
         xpsGeneveRemoveLocalEndpoint: Enter [ devId,localIp ]
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
            ret = xpsGeneveRemoveLocalEndpoint(args[0],localIp)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGeneveTunnelBindOption
    #/********************************************************************************/
    def do_geneve_tunnel_bind_option(self, arg):
        '''
         xpsGeneveTunnelBindOption: Enter [ devId,baseIntfId,optIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,baseIntfId,optIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, baseIntfId=%d, optIntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsGeneveTunnelBindOption(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGeneveSetTunnelConfig
    #/********************************************************************************/
    def do_geneve_set_tunnel_config(self, arg):
        '''
         xpsGeneveSetTunnelConfig: Enter [ devId,tnlIntfId,paclEn,paclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,paclEn,paclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsGeneveTunnelConfig_t_Ptr = new_xpsGeneveTunnelConfig_tp()
            xpsGeneveTunnelConfig_t_Ptr.paclEn = int(args[2])
            xpsGeneveTunnelConfig_t_Ptr.paclId = int(args[3])
            
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, pktCmd=%d, paclEn=%d, paclId=%d' % (args[0],args[1],xpsGeneveTunnelConfig_t_Ptr.pktCmd,xpsGeneveTunnelConfig_t_Ptr.paclEn,xpsGeneveTunnelConfig_t_Ptr.paclId))
            ret = xpsGeneveSetTunnelConfig(args[0],args[1],xpsGeneveTunnelConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                #print('pktCmd = %d' % (xpsGeneveTunnelConfig_t_Ptr.pktCmd))
                print('paclEn = %d' % (xpsGeneveTunnelConfig_t_Ptr.paclEn))
                print('paclId = %d' % (xpsGeneveTunnelConfig_t_Ptr.paclId))
                pass
            delete_xpsGeneveTunnelConfig_tp(xpsGeneveTunnelConfig_t_Ptr)

    #/********************************************************************************/
    # command for xpsGeneveGetTunnelConfig
    #/********************************************************************************/
    def do_geneve_get_tunnel_config(self, arg):
        '''
         xpsGeneveGetTunnelConfig: Enter [ devId,tnlIntfId,pktCmd,paclEn,paclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,pktCmd,paclEn,paclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsGeneveTunnelConfig_t_Ptr = new_xpsGeneveTunnelConfig_tp()
            xpsGeneveTunnelConfig_t_Ptr.pktCmd = eval(args[2])
            xpsGeneveTunnelConfig_t_Ptr.paclEn = int(args[3])
            xpsGeneveTunnelConfig_t_Ptr.paclId = int(args[4])
            
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, pktCmd=%d, paclEn=%d, paclId=%d' % (args[0],args[1],xpsGeneveTunnelConfig_t_Ptr.pktCmd,xpsGeneveTunnelConfig_t_Ptr.paclEn,xpsGeneveTunnelConfig_t_Ptr.paclId))
            ret = xpsGeneveGetTunnelConfig(args[0],args[1],xpsGeneveTunnelConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('pktCmd = %d' % (xpsGeneveTunnelConfig_t_Ptr.pktCmd))
                print('paclEn = %d' % (xpsGeneveTunnelConfig_t_Ptr.paclEn))
                print('paclId = %d' % (xpsGeneveTunnelConfig_t_Ptr.paclId))
                pass
            delete_xpsGeneveTunnelConfig_tp(xpsGeneveTunnelConfig_t_Ptr)

    #/********************************************************************************/
    # command for xpsGeneveGetTunnelRemoteIp
    #/********************************************************************************/
    def do_geneve_get_tunnel_remote_ip(self, arg):
        '''
         xpsGeneveGetTunnelRemoteIp: Enter [ devId,tnlIntfId ]
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
            ret = xpsGeneveGetTunnelRemoteIp(args[0],args[1],final_rmtEpIpAddr_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            ipAddr = ''
            if err == 0:
                for i in range(4):
                    ipAddr += str(ipv4Addr_tp_getitem(final_rmtEpIpAddr_Ptr, 3-i)) + '.'
                print 'rmtEpIpAddr = ' + ipAddr.strip('.')
                pass

    #/********************************************************************************/
    # command for xpsGeneveAddVni
    #/********************************************************************************/
    def do_geneve_add_vni(self, arg):
        '''
         xpsGeneveAddVni: Enter [ devId,vni,vlanId ]
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
            ret = xpsGeneveAddVni(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGeneveRemoveVni
    #/********************************************************************************/
    def do_geneve_remove_vni(self, arg):
        '''
         xpsGeneveRemoveVni: Enter [ devId,vni ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vni ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vni=%d' % (args[0],args[1]))
            ret = xpsGeneveRemoveVni(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGeneveAddMcTunnelEntry
    #/********************************************************************************/
    def do_geneve_add_mc_tunnel_entry(self, arg):
        '''
         xpsGeneveAddMcTunnelEntry: Enter [ devId,tnlIntfId,lclEpIpAddr,rmtEpIpAddr,l3IntfId,portIntfId ]
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
            ret = xpsGeneveAddMcTunnelEntry(args[0],args[1],lclEpIpAddr,rmtEpIpAddr,args[4],args[5])
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

