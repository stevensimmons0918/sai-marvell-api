#!/usr/bin/env python
#  xpsInterface.py
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
# The class object for xpsInterface operations
#/**********************************************************************************/

class xpsInterfaceObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsInterfaceInit
    #/********************************************************************************/
    def do_interface_init(self, arg):
        '''
         xpsInterfaceInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsInterfaceInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceInitScope
    #/********************************************************************************/
    def do_interface_init_scope(self, arg):
        '''
         xpsInterfaceInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsInterfaceInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceAddDevice
    #/********************************************************************************/
    def do_interface_add_device(self, arg):
        '''
         xpsInterfaceAddDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsInterfaceAddDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceRemoveDevice
    #/********************************************************************************/
    def do_interface_remove_device(self, arg):
        '''
         xpsInterfaceRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsInterfaceRemoveDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceDeInit
    #/********************************************************************************/
    def do_interface_de_init(self, arg):
        '''
         xpsInterfaceDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsInterfaceDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceDeInitScope
    #/********************************************************************************/
    def do_interface_de_init_scope(self, arg):
        '''
         xpsInterfaceDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsInterfaceDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceIsExist
    #/********************************************************************************/
    def do_interface_is_exist(self, arg):
        '''
         xpsInterfaceIsExist: Enter [ intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ intfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, intfId=%d' % (args[0]))
            ret = xpsInterfaceIsExist(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceIsExistScope
    #/********************************************************************************/
    def do_interface_is_exist_scope(self, arg):
        '''
         xpsInterfaceIsExistScope: Enter [ scopeId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsInterfaceIsExistScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for interfaceType enumerations
    #/********************************************************************************/
    def complete_interface_create(self, text, line, begidx, endidx):
        tempDict = { 1 : 'interfaceType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsInterfaceCreate
    #/********************************************************************************/
    def do_interface_create(self, arg):
        '''
         xpsInterfaceCreate: Enter [ type ]
         Valid values for type : <interfaceType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ type ]')
        else:
            args[0] = eval(args[0])
            intfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, type=%d' % (args[0]))
            ret = xpsInterfaceCreate(args[0],intfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_1)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(intfId_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_1)

    #/********************************************************************************/
    # Auto completion for interfaceType enumerations
    #/********************************************************************************/
    def complete_interface_create_bridge_port(self, text, line, begidx, endidx):
        tempDict = { 2 : 'interfaceType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsInterfaceCreateBridgePort
    #/********************************************************************************/
    def do_interface_create_bridge_port(self, arg):
        '''
         xpsInterfaceCreateBridgePort: Enter [ intfId, vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ intfId, vlanId ]')
        else:
            args[0] = eval(args[0])
            args[1] = eval(args[1])
            intfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, intfId:%d vlanId:%d' % (args[0], args[1]))
            ret = xpsInterfaceCreateBridgePort(args[0], args[1], intfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_1)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(intfId_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_1)

    #/********************************************************************************/
    # Auto completion for interfaceType enumerations
    #/********************************************************************************/
    def complete_interface_create_scope(self, text, line, begidx, endidx):
        tempDict = { 3 : 'interfaceType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsInterfaceCreateScope
    #/********************************************************************************/
    def do_interface_create_scope(self, arg):
        '''
         xpsInterfaceCreateScope: Enter [ scopeId,type ]
         Valid values for type : <interfaceType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,type ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            intfId_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, type=%d' % (args[0],args[1]))
            ret = xpsInterfaceCreateScope(args[0],args[1],intfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_2)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(intfId_Ptr_2))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_2)

    #/********************************************************************************/
    # command for xpsInterfaceCreateRouterOverVlan
    #/********************************************************************************/
    def do_interface_create_router_over_vlan(self, arg):
        '''
         xpsInterfaceCreateRouterOverVlan: Enter [ vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ vlanId ]')
        else:
            args[0] = int(args[0])
            intfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, vlanId=%d' % (args[0]))
            ret = xpsInterfaceCreateRouterOverVlan(args[0],intfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_1)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(intfId_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_1)

    #/********************************************************************************/
    # command for xpsInterfaceCreateRouterOverVlanScope
    #/********************************************************************************/
    def do_interface_create_router_over_vlan_scope(self, arg):
        '''
         xpsInterfaceCreateRouterOverVlanScope: Enter [ scopeId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            intfId_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsInterfaceCreateRouterOverVlanScope(args[0],args[1],intfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_2)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(intfId_Ptr_2))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_2)

    #/********************************************************************************/
    # command for xpsInterfaceDestroyBridgePort
    #/********************************************************************************/
    def do_interface_destroy_bridge_port(self, arg):
        '''
         xpsInterfaceDestroyBridgePort: Enter [ bridgePort intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ bridgePort intfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, intfId=%d' % (args[0]))
            ret = xpsInterfaceDestroyBridgePort(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceDestroy
    #/********************************************************************************/
    def do_interface_destroy(self, arg):
        '''
         xpsInterfaceDestroy: Enter [ intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ intfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, intfId=%d' % (args[0]))
            ret = xpsInterfaceDestroy(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceDestroyScope
    #/********************************************************************************/
    def do_interface_destroy_scope(self, arg):
        '''
         xpsInterfaceDestroyScope: Enter [ scopeId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsInterfaceDestroyScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceDestroyRouterOverVlan
    #/********************************************************************************/
    def do_interface_destroy_router_over_vlan(self, arg):
        '''
         xpsInterfaceDestroyRouterOverVlan: Enter [ vlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ vlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, vlanId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsInterfaceDestroyRouterOverVlan(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceDestroyRouterOverVlanScope
    #/********************************************************************************/
    def do_interface_destroy_router_over_vlan_scope(self, arg):
        '''
         xpsInterfaceDestroyRouterOverVlanScope: Enter [ scopeId,vlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, vlanId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsInterfaceDestroyRouterOverVlanScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceGetInfo
    #/********************************************************************************/
    def do_interface_get_info(self, arg):
        '''
         xpsInterfaceGetInfo: Enter [ intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ intfId,keyIntfId,type,l2EcmpSize ]')
        else:
            args[0] = int(args[0])
            xpsInterfaceInfo_t_Ptr = new_xpsInterfaceInfo_tp(1)
            
            #print('Input Arguments are, intfId=%d' % (args[0]))
            ret = xpsInterfaceGetInfo(args[0],xpsInterfaceInfo_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('keyIntfId = %d' % (xpsInterfaceInfo_tp_getitem(xpsInterfaceInfo_t_Ptr, 0).keyIntfId))
                print('type = %d' % (xpsInterfaceInfo_tp_getitem(xpsInterfaceInfo_t_Ptr, 0).type))
                print('l2EcmpSize = %d' % (xpsInterfaceInfo_tp_getitem(xpsInterfaceInfo_t_Ptr, 0).l2EcmpSize))
                pass
            delete_xpsInterfaceInfo_tp(xpsInterfaceInfo_t_Ptr)

    #/********************************************************************************/
    # command for xpsInterfaceGetInfoScope
    #/********************************************************************************/
    def do_interface_get_info_scope(self, arg):
        '''
         xpsInterfaceGetInfoScope: Enter [ scopeId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,intfId ]')
        else:
            args[0] = int(args[0])
            xpsInterfaceInfo_t_Ptr = new_xpsInterfaceInfo_tp()

            #print('Input Arguments are, scopeId=%d, intfId=%d,' % (args[0],args[1]))
            ret = xpsInterfaceGetInfoScope(args[0],args[1],xpsInterfaceInfo_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('keyIntfId = %d' % (xpsInterfaceInfo_t_Ptr.keyIntfId))
                print('type = %d' % (xpsInterfaceInfo_t_Ptr.type))
                print('l2EcmpSize = %d' % (xpsInterfaceInfo_t_Ptr.l2EcmpSize))
                pass
            delete_xpsInterfaceInfo_tp(xpsInterfaceInfo_t_Ptr)

    #/********************************************************************************/
    # command for xpsInterfaceGetType
    #/********************************************************************************/
    def do_interface_get_type(self, arg):
        '''
         xpsInterfaceGetType: Enter [ intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ intfId ]')
        else:
            args[0] = int(args[0])
            type_Ptr_1 = new_xpsInterfaceType_ep()
            #print('Input Arguments are, intfId=%d' % (args[0]))
            ret = xpsInterfaceGetType(args[0],type_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('type = %d' % (xpsInterfaceType_ep_value(type_Ptr_1)))
                pass
            delete_xpsInterfaceType_ep(type_Ptr_1)

    #/********************************************************************************/
    # command for xpsInterfaceGetTypeScope
    #/********************************************************************************/
    def do_interface_get_type_scope(self, arg):
        '''
         xpsInterfaceGetTypeScope: Enter [ scopeId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            type_Ptr_2 = new_xpsInterfaceType_ep()
            #print('Input Arguments are, scopeId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsInterfaceGetTypeScope(args[0],args[1],type_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('type = %d' % (xpsInterfaceType_ep_value(type_Ptr_2)))
                pass
            delete_xpsInterfaceType_ep(type_Ptr_2)

    #/********************************************************************************/
    # command for xpsInterfaceSetL2EcmpSize
    #/********************************************************************************/
    def do_interface_set_l2_ecmp_size(self, arg):
        '''
         xpsInterfaceSetL2EcmpSize: Enter [ intfId,l2EcmpSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ intfId,l2EcmpSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, intfId=%d, l2EcmpSize=%d' % (args[0],args[1]))
            ret = xpsInterfaceSetL2EcmpSize(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceSetL2EcmpSizeScope
    #/********************************************************************************/
    def do_interface_set_l2_ecmp_size_scope(self, arg):
        '''
         xpsInterfaceSetL2EcmpSizeScope: Enter [ scopeId,intfId,l2EcmpSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,intfId,l2EcmpSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, intfId=%d, l2EcmpSize=%d' % (args[0],args[1],args[2]))
            ret = xpsInterfaceSetL2EcmpSizeScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceGetL2EcmpSize
    #/********************************************************************************/
    def do_interface_get_l2_ecmp_size(self, arg):
        '''
         xpsInterfaceGetL2EcmpSize: Enter [ intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ intfId ]')
        else:
            args[0] = int(args[0])
            l2EcmpSize_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, intfId=%d' % (args[0]))
            ret = xpsInterfaceGetL2EcmpSize(args[0],l2EcmpSize_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('l2EcmpSize = %d' % (uint32_tp_value(l2EcmpSize_Ptr_1)))
                pass
            delete_uint32_tp(l2EcmpSize_Ptr_1)

    #/********************************************************************************/
    # command for xpsInterfaceGetL2EcmpSizeScope
    #/********************************************************************************/
    def do_interface_get_l2_ecmp_size_scope(self, arg):
        '''
         xpsInterfaceGetL2EcmpSizeScope: Enter [ scopeId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            l2EcmpSize_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsInterfaceGetL2EcmpSizeScope(args[0],args[1],l2EcmpSize_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l2EcmpSize = %d' % (uint32_tp_value(l2EcmpSize_Ptr_2)))
                pass
            delete_uint32_tp(l2EcmpSize_Ptr_2)

    #/********************************************************************************/
    # command for xpsInterfaceSetIngressPorts
    #/********************************************************************************/
    def do_interface_set_ingress_ports(self, arg):
        '''
         xpsInterfaceSetIngressPorts: Enter [ devId,intfId,size,portList ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,size,portList ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsPortList_t_Ptr = new_xpsPortList_tp()
            xpsPortList_t_Ptr.size = int(args[2])

            pList = eval(args[3])
            if len(pList) != xpsPortList_t_Ptr.size:
                err = 1
                print('Invalid Arguments')
            else:
                for i, value in enumerate(pList):
                    xpsPortList_t_Ptr.portList[i] = int(value)

                #print('Input Arguments are, devId=%d, intfId=%d, size=%d' % (args[0],args[1],xpsPortList_t_Ptr.size))
                ret = xpsInterfaceSetIngressPorts(args[0],args[1],xpsPortList_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpsPortList_tp(xpsPortList_t_Ptr)

    #/********************************************************************************/
    # command for xpsInterfaceSetEgressPorts
    #/********************************************************************************/
    def do_interface_set_egress_ports(self, arg):
        '''
         xpsInterfaceSetEgressPorts: Enter [ devId,intfId,size,portList ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,size,portList ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsPortList_t_Ptr = new_xpsPortList_tp()
            xpsPortList_t_Ptr.size = int(args[2])

            pList = eval(args[3])
            if len(pList) != xpsPortList_t_Ptr.size:
                err = 1
                print('Invalid Arguments')
            else:
                for i, value in enumerate(pList):
                    xpsPortList_t_Ptr.portList[i] = int(value)

                #print('Input Arguments are, devId=%d, intfId=%d, size=%d' % (args[0],args[1],xpsPortList_t_Ptr.size))
                ret = xpsInterfaceSetEgressPorts(args[0],args[1],xpsPortList_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpsPortList_tp(xpsPortList_t_Ptr)

    #/********************************************************************************/
    # command for xpsInterfaceSetPorts
    #/********************************************************************************/
    def do_interface_set_ports(self, arg):
        '''
         xpsInterfaceSetPorts: Enter [ devId,intfId,size,portList ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,size,portList ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsPortList_t_Ptr = new_xpsPortList_tp()
            xpsPortList_t_Ptr.size = int(args[2])
            
            pList = eval(args[3])
            if len(pList) != xpsPortList_t_Ptr.size:
                err = 1
                print('Invalid Arguments')
            else:
                for i, value in enumerate(pList):
                    xpsPortList_t_Ptr.portList[i] = int(value)
            
                #print('Input Arguments are, devId=%d, intfId=%d, size=%d' % (args[0],args[1],xpsPortList_t_Ptr.size))
                ret = xpsInterfaceSetPorts(args[0],args[1],xpsPortList_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpsPortList_tp(xpsPortList_t_Ptr)

    #/********************************************************************************/
    # command for xpsInterfaceGetIngressPorts
    #/********************************************************************************/
    def do_interface_get_ingress_ports(self, arg):
        '''
         xpsInterfaceGetIngressPorts: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsPortList_t_Ptr = new_xpsPortList_tp()
            #print('Input Arguments are, devId=%d, intfId=%d ' % (args[0],args[1]))
            ret = xpsInterfaceGetIngressPorts(args[0],args[1],xpsPortList_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('size = %d' % (xpsPortList_t_Ptr.size))
                print"portList = ",
                for i in range (0, xpsPortList_t_Ptr.size):
                    print xpsPortList_t_Ptr.portList[i],
                print('')
                pass
            delete_xpsPortList_tp(xpsPortList_t_Ptr)

    #/********************************************************************************/
    # command for xpsInterfaceGetPorts
    #/********************************************************************************/
    def do_interface_get_ports(self, arg):
        '''
         xpsInterfaceGetPorts: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsPortList_t_Ptr = new_xpsPortList_tp()
            #print('Input Arguments are, devId=%d, intfId=%d ' % (args[0],args[1]))
            ret = xpsInterfaceGetPorts(args[0],args[1],xpsPortList_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('size = %d' % (xpsPortList_t_Ptr.size))              
                print"portList = ",
                for i in range (0, xpsPortList_t_Ptr.size):
                    print xpsPortList_t_Ptr.portList[i],
                print('')
                pass
            delete_xpsPortList_tp(xpsPortList_t_Ptr)

    #/********************************************************************************/
    # command for xpsInterfaceGetFirst
    #/********************************************************************************/
    def do_interface_get_first(self, arg):
        '''
         xpsInterfaceGetFirst: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            intfId_Ptr_0 = new_xpsInterfaceId_tp()
            #print('Input Arguments are' % ())
            ret = xpsInterfaceGetFirst(intfId_Ptr_0)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_0)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(intfId_Ptr_0)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_0)

    #/********************************************************************************/
    # command for xpsInterfaceGetFirstScope
    #/********************************************************************************/
    def do_interface_get_first_scope(self, arg):
        '''
         xpsInterfaceGetFirstScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            intfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsInterfaceGetFirstScope(args[0],intfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_1)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(intfId_Ptr_1))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_1)

    #/********************************************************************************/
    # command for xpsInterfaceGetNext
    #/********************************************************************************/
    def do_interface_get_next(self, arg):
        '''
         xpsInterfaceGetNext: Enter [ intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ intfId ]')
        else:
            args[0] = int(args[0])
            intfIdNext_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, intfId=%d' % (args[0]))
            ret = xpsInterfaceGetNext(args[0],intfIdNext_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('intfIdNext = %d' % (xpsInterfaceId_tp_value(intfIdNext_Ptr_1)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(intfIdNext_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(intfIdNext_Ptr_1)

    #/********************************************************************************/
    # command for xpsInterfaceGetNextScope
    #/********************************************************************************/
    def do_interface_get_next_scope(self, arg):
        '''
         xpsInterfaceGetNextScope: Enter [ scopeId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            intfIdNext_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsInterfaceGetNextScope(args[0],args[1],intfIdNext_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('intfIdNext = %d' % (xpsInterfaceId_tp_value(intfIdNext_Ptr_2)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(intfIdNext_Ptr_2))
                pass
            delete_xpsInterfaceId_tp(intfIdNext_Ptr_2)

    #/********************************************************************************/
    # command for xpsInterfaceAddToDB
    #/********************************************************************************/
    def do_interface_add_to_db(self, arg):
        '''
         xpsInterfaceAddToDB: Enter [ keyIntfId,type,l2EcmpSize,pMetadata ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ keyIntfId,type,l2EcmpSize,pMetadata ]')
        else:
            xpsInterfaceInfo_t_Ptr = new_xpsInterfaceInfo_tp()
            xpsInterfaceInfo_t_Ptr.keyIntfId = int(args[0])
            xpsInterfaceInfo_t_Ptr.type = eval(args[1])
            xpsInterfaceInfo_t_Ptr.l2EcmpSize = int(args[2])
            xpsInterfaceInfo_t_Ptr.pMetadata = int(args[3])
            
            #print('Input Arguments are, keyIntfId=%d, type=%d, l2EcmpSize=%d' % (xpsInterfaceInfo_t_Ptr.keyIntfId,xpsInterfaceInfo_t_Ptr.type,xpsInterfaceInfo_t_Ptr.l2EcmpSize,xpsInterfaceInfo_t_Ptr.pMetadata))
            ret = xpsInterfaceAddToDB(xpsInterfaceInfo_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('keyIntfId = %d' % (xpsInterfaceInfo_t_Ptr.keyIntfId))
                print('type = %d' % (xpsInterfaceInfo_t_Ptr.type))
                print('l2EcmpSize = %d' % (xpsInterfaceInfo_t_Ptr.l2EcmpSize))
                print('pMetadata = %d' % (xpsInterfaceInfo_t_Ptr.pMetadata))
                pass
            delete_xpsInterfaceInfo_tp(xpsInterfaceInfo_t_Ptr)
    #/********************************************************************************/
    # command for xpsInterfaceRemoveFromDB
    #/********************************************************************************/
    def do_interface_remove_from_db(self, arg):
        '''
         xpsInterfaceRemoveFromDB: Enter [ intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ intfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, intfId=%d' % (args[0]))
            ret = xpsInterfaceRemoveFromDB(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceStripIncomingEtag
    #/********************************************************************************/
    def do_interface_strip_incoming_etag(self, arg):
        '''
         xpsInterfaceStripIncomingEtag: Enter [ devId,vifId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vifId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vifId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsInterfaceStripIncomingEtag(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsInterfaceSetEtagNextEngineProperty
    #/********************************************************************************/
    def do_interface_set_etag_next_engine_property(self, arg):
        '''
         xpsInterfaceSetEtagNextEngineProperty: Enter [ devId,vifId,etagNxtEngEn,etagNxtEng ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vifId,etagNxtEngEn,etagNxtEng ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, vifId=%d, etagNxtEngEn=%d, etagNxtEng=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsInterfaceSetEtagNextEngineProperty(args[0],args[1],args[2],args[3])
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

    #/*****************************************************************************************************/
    # command for xpsInterfaceUpdateParserNextEngineEntry
    #/*****************************************************************************************************/
    def do_interface_update_parser_next_engine_entry(self, arg):
        'xpsInterfaceUpdateParserNextEngineEntry: Enter [ devId,portVifId,egressVif,pktCmd,nextEngine ]'
        args =  arg.split()
        if  len(args) < 5:
            print 'Invalid input, Enter [ devId,portVifId,egressVif,pktCmd,nextEngine ]'
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            oldNextEngine_Ptr_1 = new_uint8_tp()
            oldEgressVif_Ptr_1 = new_uint8_tp()
            print 'Input Arguments are devId=%d, portVifId=%d, egressVif=%d, pktCmd=%d, nextEngine=%d ' % (args[0],args[1],args[2],args[3],args[4])
            ret = xpsInterfaceUpdateParserNextEngineEntry(args[0], args[1], args[2], args[3], args[4], oldNextEngine_Ptr_1, oldEgressVif_Ptr_1)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('oldNextEngine = %d' % (uint8_tp_value(oldNextEngine_Ptr_1)))
                print('oldEgressVif = %d' % (uint8_tp_value(oldEgressVif_Ptr_1)))
                pass
            delete_uint8_tp(oldNextEngine_Ptr_1)
            delete_uint8_tp(oldEgressVif_Ptr_1)

