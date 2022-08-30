#!/usr/bin/env python
#  xps8021Br.py
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
import enumDict

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *


#/**********************************************************************************/
# The class object for xps8021Br operations
#/**********************************************************************************/

class xps8021BrObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xps8021BrCreatePortExtenderGroup
    #/********************************************************************************/
    def do_xps8021_br_create_port_extender_group(self, arg):
        '''
         xps8021BrCreatePortExtenderGroup: Enter [ numOfPorts ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ numOfPorts ]')
        else:
            args[0] = int(args[0])
            groupId_Ptr_1 = new_xpsPeg_tp()
            #print('Input Arguments are, numOfPorts=%d' % (args[0]))
            ret = xps8021BrCreatePortExtenderGroup(args[0],groupId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('groupId = %d' % (xpsPeg_tp_value(groupId_Ptr_1)))
                pass
            delete_xpsPeg_tp(groupId_Ptr_1)
    #/********************************************************************************/
    # command for xps8021BrCreatePortExtenderGroupScope
    #/********************************************************************************/
    def do_xps8021_br_create_port_extender_group_scope(self, arg):
        '''
         xps8021BrCreatePortExtenderGroupScope: Enter [ scopeId,numOfPorts ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,numOfPorts ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            groupId_Ptr_2 = new_xpsPeg_tp()
            #print('Input Arguments are, scopeId=%d, numOfPorts=%d' % (args[0],args[1]))
            ret = xps8021BrCreatePortExtenderGroupScope(args[0],args[1],groupId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('groupId = %d' % (xpsPeg_tp_value(groupId_Ptr_2)))
                pass
            delete_xpsPeg_tp(groupId_Ptr_2)
    #/********************************************************************************/
    # command for xps8021BrDeletePortExtenderGroup
    #/********************************************************************************/
    def do_xps8021_br_delete_port_extender_group(self, arg):
        '''
         xps8021BrDeletePortExtenderGroup: Enter [ groupId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ groupId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, groupId=%d' % (args[0]))
            ret = xps8021BrDeletePortExtenderGroup(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xps8021BrDeletePortExtenderGroupScope
    #/********************************************************************************/
    def do_xps8021_br_delete_port_extender_group_scope(self, arg):
        '''
         xps8021BrDeletePortExtenderGroupScope: Enter [ scopeId,groupId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,groupId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, groupId=%d' % (args[0],args[1]))
            ret = xps8021BrDeletePortExtenderGroupScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xps8021BrCreateExtendedPort
    #/********************************************************************************/
    def do_xps8021_br_create_extended_port(self, arg):
        '''
         xps8021BrCreateExtendedPort: Enter [ groupId,maxCascadePorts ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ groupId,maxCascadePorts ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            intfId_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, groupId=%d, maxCascadePorts=%d' % (args[0],args[1]))
            ret = xps8021BrCreateExtendedPort(args[0],args[1],intfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_2)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(intfId_Ptr_2)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_2)
    #/********************************************************************************/
    # command for xps8021BrInitExtendedPort
    #/********************************************************************************/
    def do_xps8021_br_init_extended_port(self, arg):
        '''
         xps8021BrInitExtendedPort: Enter [ devId,groupId,extendedPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,groupId,extendedPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, groupId=%d, extendedPort=%d' % (args[0],args[1],args[2]))
            ret = xps8021BrInitExtendedPort(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xps8021BrBindExtendedPort
    #/********************************************************************************/
    def do_xps8021_br_bind_extended_port(self, arg):
        '''
         xps8021BrBindExtendedPort: Enter [ groupId,extendedPort,cascadePort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ groupId,extendedPort,cascadePort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, groupId=%d, extendedPort=%d, cascadePort=%d' % (args[0],args[1],args[2]))
            ret = xps8021BrBindExtendedPort(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xps8021BrUnBindExtendedPort
    #/********************************************************************************/
    def do_xps8021_br_un_bind_extended_port(self, arg):
        '''
         xps8021BrUnBindExtendedPort: Enter [ groupId,extendedPort,cascadePort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ groupId,extendedPort,cascadePort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, groupId=%d, extendedPort=%d, cascadePort=%d' % (args[0],args[1],args[2]))
            ret = xps8021BrUnBindExtendedPort(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xps8021BrDeleteExtendedPort
    #/********************************************************************************/
    def do_xps8021_br_delete_extended_port(self, arg):
        '''
         xps8021BrDeleteExtendedPort: Enter [ groupId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ groupId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, groupId=%d, intfId=%d' % (args[0],args[1]))
            ret = xps8021BrDeleteExtendedPort(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xps8021BrIsExtendedPortValid
    #/********************************************************************************/
    def do_xps8021_br_is_extended_port_valid(self, arg):
        '''
         xps8021BrIsExtendedPortValid: Enter [ groupId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ groupId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, groupId=%d, intfId=%d' % (args[0],args[1]))
            ret = xps8021BrIsExtendedPortValid(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xps8021BrIsCascadePortBindToExtendedPort
    #/********************************************************************************/
    def do_xps8021_br_is_cascade_port_bind_to_extended_port(self, arg):
        '''
         xps8021BrIsCascadePortBindToExtendedPort: Enter [ extendedPort,cascadePort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ extendedPort,cascadePort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, extendedPort=%d, cascadePort=%d' % (args[0],args[1]))
            ret = xps8021BrIsCascadePortBindToExtendedPort(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
											            
    #/********************************************************************************/
    # Auto completion for l2EncapType enumerations
    #/********************************************************************************/
    def complete_xps8021_br_get_vlan_multicast_interface(self, text, line, begidx, endidx):
        tempDict = { 2 : 'l2EncapType'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xps8021BrGetVlanMulticastInterface
    #/********************************************************************************/
    def do_xps8021_br_get_vlan_multicast_interface(self, arg):
        '''
         xps8021BrGetVlanMulticastInterface: Enter [ vlan,encapType ]
         Valid values for encapType : <l2EncapType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ vlan,encapType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            mcIntf_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, vlan=%d, encapType=%d' % (args[0],args[1]))
            ret = xps8021BrGetVlanMulticastInterface(args[0],args[1],mcIntf_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcIntf = %d' % (xpsInterfaceId_tp_value(mcIntf_Ptr_2)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(mcIntf_Ptr_2)))
                pass
            delete_xpsInterfaceId_tp(mcIntf_Ptr_2)

    #/********************************************************************************/
    # Auto completion for l2EncapType enumerations
    #/********************************************************************************/
    def complete_xps8021_br_get_l2_multicast_interface(self, text, line, begidx, endidx):
        tempDict = { 2 : 'l2EncapType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xps8021BrGetL2MulticastInterface
    #/********************************************************************************/
    def do_xps8021_br_get_l2_multicast_interface(self, arg):
        '''
         xps8021BrGetL2MulticastInterface: Enter [ ifListId,encapType ]
         Valid values for encapType : <l2EncapType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ifListId,encapType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            mcIntf_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, ifListId=%d, encapType=%d' % (args[0],args[1]))
            ret = xps8021BrGetL2MulticastInterface(args[0],args[1],mcIntf_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcIntf = %d' % (xpsInterfaceId_tp_value(mcIntf_Ptr_2)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(mcIntf_Ptr_2)))
                pass
            delete_xpsInterfaceId_tp(mcIntf_Ptr_2)
    #/********************************************************************************/
    # command for xps8021BrGetL3MulticastInterface
    #/********************************************************************************/
    def do_xps8021_br_get_l3_multicast_interface(self, arg):
        '''
         xps8021BrGetL3MulticastInterface: Enter [ ifListId,l3IntfId,encapType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ifListId,l3IntfId,encapType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            mcIntf_Ptr_3 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, ifListId=%d, l3IntfId=%d, encapType=%d' % (args[0],args[1],args[2]))
            ret = xps8021BrGetL3MulticastInterface(args[0],args[1],args[2],mcIntf_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcIntf = %d' % (xpsInterfaceId_tp_value(mcIntf_Ptr_3)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(mcIntf_Ptr_3)))
                pass
            delete_xpsInterfaceId_tp(mcIntf_Ptr_3)

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

    #/********************************************************************************/
    # command for xps8021BrCreateExtendedPortScope
    #/********************************************************************************/
    def do_xps8021_br_create_extended_port_scope(self, arg):
        '''
         xps8021BrCreateExtendedPortScope: Enter [ scopeId,groupId,maxCascadePorts ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,groupId,maxCascadePorts ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            intfId_Ptr_3 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, groupId=%d, maxCascadePorts=%d' % (args[0],args[1],args[2]))
            ret = xps8021BrCreateExtendedPortScope(args[0],args[1],args[2],intfId_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_3)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(intfId_Ptr_3))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_3)
    #/********************************************************************************/
    # command for xps8021BrBindExtendedPortScope
    #/********************************************************************************/
    def do_xps8021_br_bind_extended_port_scope(self, arg):
        '''
         xps8021BrBindExtendedPortScope: Enter [ scopeId,groupId,extendedPort,cascadePort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,groupId,extendedPort,cascadePort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, scopeId=%d, groupId=%d, extendedPort=%d, cascadePort=%d' % (args[0],args[1],args[2],args[3]))
            ret = xps8021BrBindExtendedPortScope(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xps8021BrUnBindExtendedPortScope
    #/********************************************************************************/
    def do_xps8021_br_un_bind_extended_port_scope(self, arg):
        '''
         xps8021BrUnBindExtendedPortScope: Enter [ scopeId,groupId,extendedPort,cascadePort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,groupId,extendedPort,cascadePort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, scopeId=%d, groupId=%d, extendedPort=%d, cascadePort=%d' % (args[0],args[1],args[2],args[3]))
            ret = xps8021BrUnBindExtendedPortScope(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xps8021BrDeleteExtendedPortScope
    #/********************************************************************************/
    def do_xps8021_br_delete_extended_port_scope(self, arg):
        '''
         xps8021BrDeleteExtendedPortScope: Enter [ scopeId,groupId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,groupId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, groupId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xps8021BrDeleteExtendedPortScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xps8021BrIsExtendedPortValidScope
    #/********************************************************************************/
    def do_xps8021_br_is_extended_port_valid_scope(self, arg):
        '''
         xps8021BrIsExtendedPortValidScope: Enter [ scopeId,groupId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,groupId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, groupId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xps8021BrIsExtendedPortValidScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xps8021BrIsCascadePortBindToExtendedPortScope
    #/********************************************************************************/
    def do_xps8021_br_is_cascade_port_bind_to_extended_port_scope(self, arg):
        '''
         xps8021BrIsCascadePortBindToExtendedPortScope: Enter [ scopeId,extendedPort,cascadePort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,extendedPort,cascadePort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, extendedPort=%d, cascadePort=%d' % (args[0],args[1],args[2]))
            ret = xps8021BrIsCascadePortBindToExtendedPortScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for l2EncapType enumerations
    #/********************************************************************************/
    def complete_xps8021_br_get_vlan_multicast_interface_scope(self, text, line, begidx, endidx):
        tempDict = { 3 : 'l2EncapType'}
        return  enumDict.autoCompletion(line, text, tempDict)
    
    #/********************************************************************************/
    # command for xps8021BrGetVlanMulticastInterfaceScope
    #/********************************************************************************/
    def do_xps8021_br_get_vlan_multicast_interface_scope(self, arg):
        '''
         xps8021BrGetVlanMulticastInterfaceScope: Enter [ scopeId,vlan,encapType ]
         Valid values for encapType : <l2EncapType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vlan,encapType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            mcIntf_Ptr_3 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, vlan=%d, encapType=%d' % (args[0],args[1],args[2]))
            ret = xps8021BrGetVlanMulticastInterfaceScope(args[0],args[1],args[2],mcIntf_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcIntf = %d' % (xpsInterfaceId_tp_value(mcIntf_Ptr_3)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(mcIntf_Ptr_3))
                pass
            delete_xpsInterfaceId_tp(mcIntf_Ptr_3)

    #/********************************************************************************/
    # Auto completion for l2EncapType enumerations
    #/********************************************************************************/
    def complete_xps8021_br_get_l2_multicast_interface_scope(self, text, line, begidx, endidx):
        tempDict = { 3 : 'l2EncapType'}
        return  enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xps8021BrGetL2MulticastInterfaceScope
    #/********************************************************************************/
    def do_xps8021_br_get_l2_multicast_interface_scope(self, arg):
        '''
         xps8021BrGetL2MulticastInterfaceScope: Enter [ scopeId,ifListId,encapType ]
         Valid values for encapType : <l2EncapType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,ifListId,encapType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            mcIntf_Ptr_3 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, ifListId=%d, encapType=%d' % (args[0],args[1],args[2]))
            ret = xps8021BrGetL2MulticastInterfaceScope(args[0],args[1],args[2],mcIntf_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcIntf = %d' % (xpsInterfaceId_tp_value(mcIntf_Ptr_3)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(mcIntf_Ptr_3))
                pass
            delete_xpsInterfaceId_tp(mcIntf_Ptr_3)
    #/********************************************************************************/
    # command for xps8021BrGetL3MulticastInterfaceScope
    #/********************************************************************************/
    def do_xps8021_br_get_l3_multicast_interface_scope(self, arg):
        '''
         xps8021BrGetL3MulticastInterfaceScope: Enter [ scopeId,ifListId,l3IntfId,encapType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,ifListId,l3IntfId,encapType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            mcIntf_Ptr_4 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, ifListId=%d, l3IntfId=%d, encapType=%d' % (args[0],args[1],args[2],args[3]))
            ret = xps8021BrGetL3MulticastInterfaceScope(args[0],args[1],args[2],args[3],mcIntf_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcIntf = %d' % (xpsInterfaceId_tp_value(mcIntf_Ptr_4)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(mcIntf_Ptr_4))
                pass
            delete_xpsInterfaceId_tp(mcIntf_Ptr_4)
