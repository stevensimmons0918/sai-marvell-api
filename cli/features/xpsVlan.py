#!/usr/bin/env python
#  xpsVlan.py
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
# The class object for xpsVlan operations
#/**********************************************************************************/

class xpsVlanObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsVlanInitScope
    #/********************************************************************************/
    def do_vlan_init_scope(self, arg):
        '''
         xpsVlanInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsVlanInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanInit
    #/********************************************************************************/
    def do_vlan_init(self, arg):
        '''
         xpsVlanInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsVlanInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanDeInitScope
    #/********************************************************************************/
    def do_vlan_de_init_scope(self, arg):
        '''
         xpsVlanDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsVlanDeInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanDeInit
    #/********************************************************************************/
    def do_vlan_de_init(self, arg):
        '''
         xpsVlanDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsVlanDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanSetRehashLevel
    #/********************************************************************************/
    def do_vlan_set_rehash_level(self, arg):
        '''
         xpsVlanSetRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsVlanSetRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanGetRehashLevel
    #/********************************************************************************/
    def do_vlan_get_rehash_level(self, arg):
        '''
         xpsVlanGetRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsVlanGetRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)

    #/********************************************************************************/
    # command for xpsVlanAddDevice
    #/********************************************************************************/
    def do_vlan_add_device(self, arg):
        '''
         xpsVlanAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsVlanAddDevice(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanRemoveDevice
    #/********************************************************************************/
    def do_vlan_remove_device(self, arg):
        '''
         xpsVlanRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsVlanRemoveDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanIsExistOnDevice
    #/********************************************************************************/
    def do_vlan_is_exist_on_device(self, arg):
        '''
         xpsVlanIsExistOnDevice: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanIsExistOnDevice(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanShow
    #/********************************************************************************/
    def do_vlan_show(self, arg):
        '''
         xpsVlanShow: Enter [ devId,vlanId ]
        '''
        args = re.split(';| |,',arg)
        numArgsReq =1
        XP_VLAN_SHOW_ALL=5000
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, in vlan show Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            if ( len(args)==1):
              ret = xpsVlanShow(args[0],XP_VLAN_SHOW_ALL)
              #print('Input Arguments are, devId=%d ' % (args[0]))
            else:
              args[1]=int(args[1])
              ret = xpsVlanShow(args[0],args[1])
              #print('Input Arguments are, devId=%d vlanId=%d' % (args[0],args[1]))
            if ret != 0:
              print('Error returned = %d' % (ret))
            else:
              pass
    #/********************************************************************************/
    # command for xpsVlanShowDetail
    #/********************************************************************************/
    def do_vlan_show_detail(self, arg):
        '''
         xpsVlanShowDetail: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        XP_VLAN_SHOW_ALL=5000
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            if ( len(args)==1):
              ret = xpsVlanShowDetail(args[0],XP_VLAN_SHOW_ALL)
            else:
              args[1] = int(args[1])
              ret = xpsVlanShowDetail(args[0],args[1])
              #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            if ret != 0 and ret != XP_ERR_NOT_FOUND:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanIsEndpointExist
    #/********************************************************************************/
    def do_vlan_is_endpoint_exist(self, arg):
        '''
         xpsVlanIsEndpointExist: Enter [ devId,vlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanIsEndpointExist(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanIsIntfExist
    #/********************************************************************************/
    def do_vlan_is_intf_exist(self, arg):
        '''
         xpsVlanIsIntfExist: Enter [ devId,vlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanIsIntfExist(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanSetDefault
    #/********************************************************************************/
    def do_vlan_set_default(self, arg):
        '''
         xpsVlanSetDefault: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanSetDefault(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetDefault
    #/********************************************************************************/
    def do_vlan_get_default(self, arg):
        '''
         xpsVlanGetDefault: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            vlanId_Ptr_1 = new_xpsVlan_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsVlanGetDefault(args[0],vlanId_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (xpsVlan_tp_value(vlanId_Ptr_1)))
                pass
            delete_xpsVlan_tp(vlanId_Ptr_1)

    #/********************************************************************************/
    # command for xpsVlanCreate
    #/********************************************************************************/
    def do_vlan_create(self, arg):
        '''
         xpsVlanCreate: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanCreate(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanCreateBd
    #/********************************************************************************/
    def do_vlan_create_bd(self, arg):
        '''
         xpsVlanCreateBd: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            vlanPtr = new_xpsVlan_tp()
            #print('Input Arguments are, devId=%d % (args[0]))
            ret = xpsVlanCreateBd(args[0], vlanPtr)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('Bridge Domain Id : %d' % (xpsVlan_tp_value(vlanPtr)))
                xpShellGlobals.cmdRetVal = ((xpsVlan_tp_value(vlanPtr)))
                pass
            delete_xpsVlan_tp(vlanPtr)
    #/********************************************************************************/
    # command for xpsVlanDestroy
    #/********************************************************************************/
    def do_vlan_destroy(self, arg):
        '''
         xpsVlanDestroy: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanDestroy(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for l2EncapType enumerations
    #/********************************************************************************/
    def complete_vlan_add_endpoint(self, text, line, begidx, endidx):
        tempDict = { 4 : 'l2EncapType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanAddEndpoint
    #/********************************************************************************/
    def do_vlan_add_endpoint(self, arg):
        '''
         xpsVlanAddEndpoint: Enter [ devId,vlanId,intfId,tagType,data ]
		 Valid values for tagType  : <l2EncapType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId,tagType,data ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d, tagType=%d, data=0x%x' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsVlanAddEndpoint(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for l2EncapType enumerations
    #/********************************************************************************/
    def complete_vlan_add_interface(self, text, line, begidx, endidx):
        tempDict = { 4 : 'l2EncapType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanAddInterface
    #/********************************************************************************/
    def do_vlan_add_interface(self, arg):
        '''
         xpsVlanAddInterface: Enter [ devId,vlanId,intfId,tagType ]
		 Valid values for tagType  : <l2EncapType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId,tagType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d, tagType=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsVlanAddInterface(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for l2EncapType enumerations
    #/********************************************************************************/
    def complete_vlan_set_redirect_to_interface(self, text, line, begidx, endidx):
        tempDict = { 6 : 'l2EncapType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetRedirectToInterface
    #/********************************************************************************/
    def do_vlan_set_redirect_to_interface(self, arg):
        '''
         xpsVlanAddInterface: Enter [ devId,vlanId,intfId,enRedirect,destIntfId,encapType ]
		 Valid values for encapType  : <l2EncapType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId,enRedirect,destIntfId,encapType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = eval(args[5])
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d, tagType=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsVlanSetRedirectToInterface(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanRemoveEndpoint
    #/********************************************************************************/
    def do_vlan_remove_endpoint(self, arg):
        '''
         xpsVlanRemoveEndpoint: Enter [ devId,vlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanRemoveEndpoint(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanRemoveInterface
    #/********************************************************************************/
    def do_vlan_remove_interface(self, arg):
        '''
         xpsVlanRemoveInterface: Enter [ devId,vlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanRemoveInterface(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetInterfaceList
    #/********************************************************************************/
    def do_vlan_get_interface_list(self, arg):
        '''
         xpsVlanGetInterfaceList: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            intfList_Ptr_2 = new_xpsInterfaceId_tpp(1)
            intfList_Ptr_1 = new_arrUint32(32)
            xpsInterfaceId_tpp_setitem(intfList_Ptr_2, 0, intfList_Ptr_1)
            numOfIntfs_Ptr_3 = new_uint16_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetInterfaceList(args[0],args[1],intfList_Ptr_2,numOfIntfs_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                intfList = []
                for i in range(0, uint16_tp_value(numOfIntfs_Ptr_3)):
                    intfList.append(arrUint32_getitem(xpsInterfaceId_tpp_getitem(intfList_Ptr_2, 0), i))
                if not intfList:
                    print('numOfIntfs = %d' % (uint16_tp_value(numOfIntfs_Ptr_3)))
                else:
                    print('intfList = %s' % (str(intfList).replace('L', '').replace('[', "").replace(']', "")))
                    print('numOfIntfs = %d' % (uint16_tp_value(numOfIntfs_Ptr_3)))
                pass
            delete_uint16_tp(numOfIntfs_Ptr_3)
            delete_uint32_tp(intfList_Ptr_1)
            delete_xpsInterfaceId_tpp(intfList_Ptr_2)

    #/********************************************************************************/
    # Auto completion for vlanBridgeMcMode enumerations
    #/********************************************************************************/
    def complete_vlan_set_ipv4_mc_bridge_mode(self, text, line, begidx, endidx):
        tempDict = { 3 : 'vlanBridgeMcMode'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetIpv4McBridgeMode
    #/********************************************************************************/
    def do_vlan_set_ipv4_mc_bridge_mode(self, arg):
        '''
         xpsVlanSetIpv4McBridgeMode: Enter [ devId,vlanId,bridgeMode ]
		 Valid values for bridgeMode : <vlanBridgeMcMode>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,bridgeMode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, bridgeMode=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetIpv4McBridgeMode(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for vlanBridgeMcMode enumerations
    #/********************************************************************************/
    def complete_vlan_set_ipv6_mc_bridge_mode(self, text, line, begidx, endidx):
        tempDict = { 3 : 'vlanBridgeMcMode'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetIpv6McBridgeMode
    #/********************************************************************************/
    def do_vlan_set_ipv6_mc_bridge_mode(self, arg):
        '''
         xpsVlanSetIpv6McBridgeMode: Enter [ devId,vlanId,bridgeMode ]
		 Valid values for bridgeMode : <vlanBridgeMcMode>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,bridgeMode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, bridgeMode=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetIpv6McBridgeMode(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass


    #/********************************************************************************/
    # Auto completion for pktCmd enumerations
    #/********************************************************************************/
    def complete_vlan_set_unreg_mc_cmd(self, text, line, begidx, endidx):
        tempDict = { 3 : 'pktCmd'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetUnregMcCmd
    #/********************************************************************************/
    def do_vlan_set_unreg_mc_cmd(self, arg):
        '''
         xpsVlanSetUnregMcCmd: Enter [ devId,vlanId,unRegMcCmd ]
		 Valid values for unRegMcCmd : <pktCmd>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,unRegMcCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, unRegMcCmd=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetUnregMcCmd(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetIpv4McBridgeMode
    #/********************************************************************************/
    def do_vlan_get_ipv4_mc_bridge_mode(self, arg):
        '''
         xpsVlanGetIpv4McBridgeMode: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            bridgeMode_Ptr_2 = new_xpsVlanBridgeMcMode_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetIpv4McBridgeMode(args[0],args[1],bridgeMode_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('bridgeMode = %d' % (xpsVlanBridgeMcMode_ep_value(bridgeMode_Ptr_2)))
                pass
            delete_xpsVlanBridgeMcMode_ep(bridgeMode_Ptr_2)

    #/********************************************************************************/
    # command for xpsVlanGetIpv6McBridgeMode
    #/********************************************************************************/
    def do_vlan_get_ipv6_mc_bridge_mode(self, arg):
        '''
         xpsVlanGetIpv6McBridgeMode: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            bridgeMode_Ptr_2 = new_xpsVlanBridgeMcMode_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetIpv6McBridgeMode(args[0],args[1],bridgeMode_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('bridgeMode = %d' % (xpsVlanBridgeMcMode_ep_value(bridgeMode_Ptr_2)))
                pass
            delete_xpsVlanBridgeMcMode_ep(bridgeMode_Ptr_2)

    #/********************************************************************************/
    # command for xpsVlanGetUnregMcCmd
    #/********************************************************************************/
    def do_vlan_get_unreg_mc_cmd(self, arg):
        '''
         xpsVlanGetUnregMcCmd: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            unRegMcCmd_Ptr_2 = new_xpsPktCmd_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetUnregMcCmd(args[0],args[1],unRegMcCmd_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('unRegMcCmd = %d' % (xpsPktCmd_ep_value(unRegMcCmd_Ptr_2)))
                pass
            delete_xpsPktCmd_ep(unRegMcCmd_Ptr_2)

    #/********************************************************************************/
    # Auto completion for pktCmd enumerations
    #/********************************************************************************/
    def complete_vlan_set_arp_bc_cmd(self, text, line, begidx, endidx):
        tempDict = { 3 : 'pktCmd'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetArpBcCmd
    #/********************************************************************************/
    def do_vlan_set_arp_bc_cmd(self, arg):
        '''
         xpsVlanSetArpBcCmd: Enter [ devId,vlanId,arpBcCmd ]
		 Valid values for arpBcCmd : <pktCmd>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,arpBcCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, arpBcCmd=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetArpBcCmd(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetArpBcCmd
    #/********************************************************************************/
    def do_vlan_get_arp_bc_cmd(self, arg):
        '''
         xpsVlanGetArpBcCmd: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            arpBcCmd_Ptr_2 = new_xpsPktCmd_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetArpBcCmd(args[0],args[1],arpBcCmd_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('arpBcCmd = %d' % (xpsPktCmd_ep_value(arpBcCmd_Ptr_2)))
                pass
            delete_xpsPktCmd_ep(arpBcCmd_Ptr_2)

    #/********************************************************************************/
    # Auto completion for pktCmd enumerations
    #/********************************************************************************/
    def complete_vlan_set_unknown_uc_cmd(self, text, line, begidx, endidx):
        tempDict = { 3 : 'pktCmd'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetUnknownUcCmd
    #/********************************************************************************/
    def do_vlan_set_unknown_uc_cmd(self, arg):
        '''
         xpsVlanSetUnknownUcCmd: Enter [ devId,vlanId,unknownUcCmd ]
		 Valid values for unknownUcCmd : <pktCmd>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,unknownUcCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, unknownUcCmd=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetUnknownUcCmd(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetUnknownUcCmd
    #/********************************************************************************/
    def do_vlan_get_unknown_uc_cmd(self, arg):
        '''
         xpsVlanGetUnknownUcCmd: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            unknownUcCmd_Ptr_2 = new_xpsPktCmd_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetUnknownUcCmd(args[0],args[1],unknownUcCmd_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('unknownUcCmd = %d' % (xpsPktCmd_ep_value(unknownUcCmd_Ptr_2)))
                pass
            delete_xpsPktCmd_ep(unknownUcCmd_Ptr_2)

    #/********************************************************************************/
    # Auto completion for pktCmd enumerations
    #/********************************************************************************/
    def complete_vlan_set_bc_cmd(self, text, line, begidx, endidx):
        tempDict = { 3 : 'pktCmd'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetBcCmd
    #/********************************************************************************/
    def do_vlan_set_bc_cmd(self, arg):
        '''
         xpsVlanSetBcCmd: Enter [ devId,vlanId,bcCmd ]
		 Valid values for bcCmd : <pktCmd>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,bcCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, bcCmd=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetBcCmd(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetBcCmd
    #/********************************************************************************/
    def do_vlan_get_bc_cmd(self, arg):
        '''
         xpsVlanGetBcCmd: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            bcCmd_Ptr_2 = new_xpsPktCmd_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetBcCmd(args[0],args[1],bcCmd_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('bcCmd = %d' % (xpsPktCmd_ep_value(bcCmd_Ptr_2)))
                pass
            delete_xpsPktCmd_ep(bcCmd_Ptr_2)

    #/********************************************************************************/
    # Auto completion for pktCmd enumerations
    #/********************************************************************************/
    def complete_vlan_set_unknown_sa_cmd(self, text, line, begidx, endidx):
        tempDict = { 3 : 'pktCmd'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetUnknownSaCmd
    #/********************************************************************************/
    def do_vlan_set_unknown_sa_cmd(self, arg):
        '''
         xpsVlanSetUnknownSaCmd: Enter [ devId,vlanId,saMissCmd ]
		 Valid values for saMissCmd : <pktCmd>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,saMissCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, saMissCmd=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetUnknownSaCmd(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetUnknownSaCmd
    #/********************************************************************************/
    def do_vlan_get_unknown_sa_cmd(self, arg):
        '''
         xpsVlanGetUnknownSaCmd: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            saMissCmd_Ptr_2 = new_xpsPktCmd_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetUnknownSaCmd(args[0],args[1],saMissCmd_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('saMissCmd = %d' % (xpsPktCmd_ep_value(saMissCmd_Ptr_2)))
                pass
            delete_xpsPktCmd_ep(saMissCmd_Ptr_2)

    #/********************************************************************************/
    # command for xpsVlanSetMirrorToAnalyzer
    #/********************************************************************************/
    def do_vlan_set_mirror_to_analyzer(self, arg):
        '''
         xpsVlanSetMirrorToAnalyzer: Enter [ devId,vlanId,enable,mirrorAnalyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,enable,mirrorAnalyzerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, vlanId=%d, enable=%d, mirrorAnalyzerId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsVlanSetMirrorToAnalyzer(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    def do_vlan_get_mirror_analyzer_id(self, arg):
        '''
         xpsVlanGetMirrorAnalyzerId: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            analyzerId_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetMirrorAnalyzerId(args[0],args[1],enable_Ptr_2,analyzerId_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                print('analyzerId = %d' % (uint32_tp_value(analyzerId_Ptr_3)))
                pass
            delete_uint32_tp(analyzerId_Ptr_3)
            delete_uint32_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsVlanSetConfig
    #/********************************************************************************/
    def do_vlan_set_config(self, arg):
        '''
         xpsVlanSetConfig: Enter [ devId,vlanId,stpId,countMode,enableMirror,mirrorAnalyzerId,saMissCmd,bcCmd,unknownUcCmd,arpBcCmd,ipv4McbridgeMode,ipv6McbridgeMode,unRegMcCmd ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 13
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,stpId,countMode,enableMirror,mirrorAnalyzerId,saMissCmd,bcCmd,unknownUcCmd,arpBcCmd,ipv4McbridgeMode,ipv6McbridgeMode,unRegMcCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsVlanConfig_t_Ptr = new_xpsVlanConfig_tp()
            xpsVlanConfig_t_Ptr.stpId = int(args[2])
            xpsVlanConfig_t_Ptr.countMode = eval(args[3])
            xpsVlanConfig_t_Ptr.enableMirror = int(args[4])
            xpsVlanConfig_t_Ptr.mirrorAnalyzerId = int(args[5])
            xpsVlanConfig_t_Ptr.saMissCmd = eval(args[6])
            xpsVlanConfig_t_Ptr.bcCmd = eval(args[7])
            xpsVlanConfig_t_Ptr.unknownUcCmd = eval(args[8])
            xpsVlanConfig_t_Ptr.arpBcCmd = eval(args[9])
            xpsVlanConfig_t_Ptr.ipv4McbridgeMode = eval(args[10])
            xpsVlanConfig_t_Ptr.ipv6McbridgeMode = eval(args[11])
            xpsVlanConfig_t_Ptr.unRegMcCmd = eval(args[12])

            #print('Input Arguments are, devId=%d, vlanId=%d, stpId=%d, countMode=%d, enableMirror=%d, mirrorAnalyzerId=%d, saMissCmd=%d, bcCmd=%d, unknownUcCmd=%d, arpBcCmd=%d, ipv4McbridgeMode=%d, ipv6McbridgeMode=%d, unRegMcCmd=%d' % (args[0],args[1],xpsVlanConfig_t_Ptr.stpId,xpsVlanConfig_t_Ptr.countMode,xpsVlanConfig_t_Ptr.enableMirror,xpsVlanConfig_t_Ptr.mirrorAnalyzerId,xpsVlanConfig_t_Ptr.saMissCmd,xpsVlanConfig_t_Ptr.bcCmd,xpsVlanConfig_t_Ptr.unknownUcCmd,xpsVlanConfig_t_Ptr.arpBcCmd,xpsVlanConfig_t_Ptr.ipv4McbridgeMode,xpsVlanConfig_t_Ptr.ipv6McbridgeMode,xpsVlanConfig_t_Ptr.unRegMcCmd))
            ret = xpsVlanSetConfig(args[0],args[1],xpsVlanConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('stpId = %d' % (xpsVlanConfig_t_Ptr.stpId))
                print('countMode = %d' % (xpsVlanConfig_t_Ptr.countMode))
                print('enableMirror = %d' % (xpsVlanConfig_t_Ptr.enableMirror))
                print('mirrorAnalyzerId = %d' % (xpsVlanConfig_t_Ptr.mirrorAnalyzerId))
                print('saMissCmd = %d' % (xpsVlanConfig_t_Ptr.saMissCmd))
                print('bcCmd = %d' % (xpsVlanConfig_t_Ptr.bcCmd))
                print('unknownUcCmd = %d' % (xpsVlanConfig_t_Ptr.unknownUcCmd))
                print('arpBcCmd = %d' % (xpsVlanConfig_t_Ptr.arpBcCmd))
                print('ipv4McbridgeMode = %d' % (xpsVlanConfig_t_Ptr.ipv4McbridgeMode))
                print('ipv6McbridgeMode = %d' % (xpsVlanConfig_t_Ptr.ipv6McbridgeMode))
                print('unRegMcCmd = %d' % (xpsVlanConfig_t_Ptr.unRegMcCmd))
                pass
            delete_xpsVlanConfig_tp(xpsVlanConfig_t_Ptr)

    #/********************************************************************************/
    # command for xpsVlanGetConfig
    #/********************************************************************************/
    def do_vlan_get_config(self, arg):
        '''
         xpsVlanGetConfig: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsVlanConfig_t_Ptr = new_xpsVlanConfig_tp()

            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0], args[1]))
            ret = xpsVlanGetConfig(args[0],args[1],xpsVlanConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('stpId = %d' % (xpsVlanConfig_t_Ptr.stpId))
                print('countMode = %d' % (xpsVlanConfig_t_Ptr.countMode))
                print('enableMirror = %d' % (xpsVlanConfig_t_Ptr.enableMirror))
                print('mirrorAnalyzerId = %d' % (xpsVlanConfig_t_Ptr.mirrorAnalyzerId))
                print('saMissCmd = %d' % (xpsVlanConfig_t_Ptr.saMissCmd))
                print('bcCmd = %d' % (xpsVlanConfig_t_Ptr.bcCmd))
                print('unknownUcCmd = %d' % (xpsVlanConfig_t_Ptr.unknownUcCmd))
                print('arpBcCmd = %d' % (xpsVlanConfig_t_Ptr.arpBcCmd))
                print('ipv4McbridgeMode = %d' % (xpsVlanConfig_t_Ptr.ipv4McbridgeMode))
                print('ipv6McbridgeMode = %d' % (xpsVlanConfig_t_Ptr.ipv6McbridgeMode))
                print('unRegMcCmd = %d' % (xpsVlanConfig_t_Ptr.unRegMcCmd))
                pass
            delete_xpsVlanConfig_tp(xpsVlanConfig_t_Ptr)

    #/********************************************************************************/
    # Auto completion for countMode enumerations
    #/********************************************************************************/
    def complete_vlan_set_intf_count_mode(self, text, line, begidx, endidx):
        tempDict = { 4 : 'countMode'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetIntfCountMode
    #/********************************************************************************/
    def do_vlan_set_intf_count_mode(self, arg):
        '''
         xpsVlanSetIntfCountMode: Enter [ devId,vlanId,intfId,countMode ]
		 Valid values for countMode : <countMode>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId,countMode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d, countMode=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsVlanSetIntfCountMode(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetIntfCountMode
    #/********************************************************************************/
    def do_vlan_get_intf_count_mode(self, arg):
        '''
         xpsVlanGetIntfCountMode: Enter [ devId,vlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            countMode_Ptr_3 = new_xpsCountMode_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanGetIntfCountMode(args[0],args[1],args[2],countMode_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('countMode = %d' % (xpsCountMode_ep_value(countMode_Ptr_3)))
                pass
            delete_xpsCountMode_ep(countMode_Ptr_3)

    #/********************************************************************************/
    # command for xpsVlanSetHairpin
    #/********************************************************************************/
    def do_vlan_set_hairpin(self, arg):
        '''
         xpsVlanSetHairpin: Enter [ devId,vlanId,intfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsVlanSetHairpin(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPVlanCreatePrimary
    #/********************************************************************************/
    def do_p_vlan_create_primary(self, arg):
        '''
         xpsPVlanCreatePrimary: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsPVlanCreatePrimary(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPVlanDestroyPrimary
    #/********************************************************************************/
    def do_p_vlan_destroy_primary(self, arg):
        '''
         xpsPVlanDestroyPrimary: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsPVlanDestroyPrimary(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPVlanCreateSecondary
    #/********************************************************************************/
    def do_p_vlan_create_secondary(self, arg):
        '''
         xpsPVlanCreateSecondary: Enter [ devId,secVlanId,priVlanId,vlanType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,secVlanId,priVlanId,vlanType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, secVlanId=%d, priVlanId=%d, vlanType=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPVlanCreateSecondary(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPVlanDestroySecondary
    #/********************************************************************************/
    def do_p_vlan_destroy_secondary(self, arg):
        '''
         xpsPVlanDestroySecondary: Enter [ devId,secVlanId,priVlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,secVlanId,priVlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, secVlanId=%d, priVlanId=%d' % (args[0],args[1],args[2]))
            ret = xpsPVlanDestroySecondary(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for l2EncapType enumerations
    #/********************************************************************************/
    def complete_p_vlan_add_interface_primary(self, text, line, begidx, endidx):
        tempDict = { 4 : 'l2EncapType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsPVlanAddInterfacePrimary
    #/********************************************************************************/
    def do_p_vlan_add_interface_primary(self, arg):
        '''
         xpsPVlanAddInterfacePrimary: Enter [ devId,priVlanId,intfId,tagType ]
		 Valid values for tagType : <l2EncapType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,priVlanId,intfId,tagType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, priVlanId=%d, intfId=%d, tagType=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPVlanAddInterfacePrimary(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPVlanRemoveInterfacePrimary
    #/********************************************************************************/
    def do_p_vlan_remove_interface_primary(self, arg):
        '''
         xpsPVlanRemoveInterfacePrimary: Enter [ devId,priVlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,priVlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, priVlanId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsPVlanRemoveInterfacePrimary(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for l2EncapType enumerations
    #/********************************************************************************/
    def complete_p_vlan_add_interface_community(self, text, line, begidx, endidx):
        tempDict = { 5 : 'l2EncapType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsPVlanAddInterfaceCommunity
    #/********************************************************************************/
    def do_p_vlan_add_interface_community(self, arg):
        '''
         xpsPVlanAddInterfaceCommunity: Enter [ devId,priVlanId,secVlanId,intfId,tagType ]
		 Valid values for tagType : <l2EncapType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,priVlanId,secVlanId,intfId,tagType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = eval(args[4])
            #print('Input Arguments are, devId=%d, priVlanId=%d, secVlanId=%d, intfId=%d, tagType=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsPVlanAddInterfaceCommunity(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPVlanRemoveInterfaceCommunity
    #/********************************************************************************/
    def do_p_vlan_remove_interface_community(self, arg):
        '''
         xpsPVlanRemoveInterfaceCommunity: Enter [ devId,priVlanId,secVlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,priVlanId,secVlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, priVlanId=%d, secVlanId=%d, intfId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPVlanRemoveInterfaceCommunity(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for l2EncapType enumerations
    #/********************************************************************************/
    def complete_p_vlan_add_interface_isolated(self, text, line, begidx, endidx):
        tempDict = { 5 : 'l2EncapType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsPVlanAddInterfaceIsolated
    #/********************************************************************************/
    def do_p_vlan_add_interface_isolated(self, arg):
        '''
         xpsPVlanAddInterfaceIsolated: Enter [ devId,priVlanId,secVlanId,intfId,tagType ]
		 Valid values for tagType : <l2EncapType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,priVlanId,secVlanId,intfId,tagType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = eval(args[4])
            #print('Input Arguments are, devId=%d, priVlanId=%d, secVlanId=%d, intfId=%d, tagType=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsPVlanAddInterfaceIsolated(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPVlanRemoveInterfaceIsolated
    #/********************************************************************************/
    def do_p_vlan_remove_interface_isolated(self, arg):
        '''
         xpsPVlanRemoveInterfaceIsolated: Enter [ devId,priVlanId,secVlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,priVlanId,secVlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, priVlanId=%d, secVlanId=%d, intfId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPVlanRemoveInterfaceIsolated(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanBindStp
    #/********************************************************************************/
    def do_vlan_bind_stp(self, arg):
        '''
         xpsVlanBindStp: Enter [ devId,vlanId,stpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,stpId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, stpId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanBindStp(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanUnbindStp
    #/********************************************************************************/
    def do_vlan_unbind_stp(self, arg):
        '''
         xpsVlanUnbindStp: Enter [ devId,vlanId,stpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,stpId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, stpId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanUnbindStp(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetStp
    #/********************************************************************************/
    def do_vlan_get_stp(self, arg):
        '''
         xpsVlanGetStp: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            stpId_Ptr_2 = new_xpsStp_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetStp(args[0],args[1],stpId_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('stpId = %d' % (xpsStp_tp_value(stpId_Ptr_2)))
                xpShellGlobals.cmdRetVal = ((xpsStp_tp_value(stpId_Ptr_2)))
                pass
            delete_xpsStp_tp(stpId_Ptr_2)

    #/********************************************************************************/
    # Auto completion for vlanStgState enumerations
    #/********************************************************************************/
    def complete_vlan_set_stp_state(self, text, line, begidx, endidx):
        tempDict = { 4 : 'vlanStgState'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetStpState
    #/********************************************************************************/
    def do_vlan_set_stp_state(self, arg):
        '''
         xpsVlanSetStpState: Enter [ devId,vlanId,intfId,stpState ]
		 Valid values for stpState : <vlanStgState>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId,stpState ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d, stpState=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsVlanSetStpState(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetStpState
    #/********************************************************************************/
    def do_vlan_get_stp_state(self, arg):
        '''
         xpsVlanGetStpState: Enter [ devId,vlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            stpState_Ptr_3 = new_xpsStpState_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanGetStpState(args[0],args[1],args[2],stpState_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('stpState = %d' % (xpsStpState_ep_value(stpState_Ptr_3)))
                pass
            delete_xpsStpState_ep(stpState_Ptr_3)

    #/********************************************************************************/
    # command for xpsVlanServiceInstanceCreate
    #/********************************************************************************/
    def do_vlan_service_instance_create(self, arg):
        '''
         xpsVlanServiceInstanceCreate: Enter [ devId,serviceId,type,vlanId,baclEn,raclEn,baclId,raclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,serviceId,type,vlanId,baclEn,raclEn,baclId,raclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            xpsServiceIdData_t_Ptr = new_xpsServiceIdData_tp()
            xpsServiceIdData_t_Ptr.vlanId = int(args[3])
            xpsServiceIdData_t_Ptr.baclEn = int(args[4])
            xpsServiceIdData_t_Ptr.raclEn = int(args[5])
            xpsServiceIdData_t_Ptr.baclId = int(args[6])
            xpsServiceIdData_t_Ptr.raclId = int(args[7])

            #print('Input Arguments are, devId=%d, serviceId=%d, type=%d, vlanId=%d, baclEn=%d, raclEn=%d, baclId=%d, raclId=%d' % (args[0],args[1],args[2],xpsServiceIdData_t_Ptr.vlanId,xpsServiceIdData_t_Ptr.baclEn,xpsServiceIdData_t_Ptr.raclEn,xpsServiceIdData_t_Ptr.baclId,xpsServiceIdData_t_Ptr.raclId))
            ret = xpsVlanServiceInstanceCreate(args[0],args[1],args[2],xpsServiceIdData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (xpsServiceIdData_t_Ptr.vlanId))
                print('baclEn = %d' % (xpsServiceIdData_t_Ptr.baclEn))
                print('raclEn = %d' % (xpsServiceIdData_t_Ptr.raclEn))
                print('baclId = %d' % (xpsServiceIdData_t_Ptr.baclId))
                print('raclId = %d' % (xpsServiceIdData_t_Ptr.raclId))
                pass
            delete_xpsServiceIdData_tp(xpsServiceIdData_t_Ptr)

    #/********************************************************************************/
    # command for xpsVlanServiceInstanceRemove
    #/********************************************************************************/
    def do_vlan_service_instance_remove(self, arg):
        '''
         xpsVlanServiceInstanceRemove: Enter [ devId,serviceId,type ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,serviceId,type ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, serviceId=%d, type=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanServiceInstanceRemove(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanServiceInstanceSetConfig
    #/********************************************************************************/
    def do_vlan_service_instance_set_config(self, arg):
        '''
         xpsVlanServiceInstanceSetConfig: Enter [ devId,serviceId,type,vlanId,baclEn,raclEn,baclId,raclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,serviceId,type,vlanId,baclEn,raclEn,baclId,raclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            xpsServiceIdData_t_Ptr = new_xpsServiceIdData_tp()
            xpsServiceIdData_t_Ptr.vlanId = int(args[3])
            xpsServiceIdData_t_Ptr.baclEn = int(args[4])
            xpsServiceIdData_t_Ptr.raclEn = int(args[5])
            xpsServiceIdData_t_Ptr.baclId = int(args[6])
            xpsServiceIdData_t_Ptr.raclId = int(args[7])

            #print('Input Arguments are, devId=%d, serviceId=%d, type=%d, vlanId=%d, baclEn=%d, raclEn=%d, baclId=%d, raclId=%d' % (args[0],args[1],args[2],xpsServiceIdData_t_Ptr.vlanId,xpsServiceIdData_t_Ptr.baclEn,xpsServiceIdData_t_Ptr.raclEn,xpsServiceIdData_t_Ptr.baclId,xpsServiceIdData_t_Ptr.raclId))
            ret = xpsVlanServiceInstanceSetConfig(args[0],args[1],args[2],xpsServiceIdData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (xpsServiceIdData_t_Ptr.vlanId))
                print('baclEn = %d' % (xpsServiceIdData_t_Ptr.baclEn))
                print('raclEn = %d' % (xpsServiceIdData_t_Ptr.raclEn))
                print('baclId = %d' % (xpsServiceIdData_t_Ptr.baclId))
                print('raclId = %d' % (xpsServiceIdData_t_Ptr.raclId))
                pass
            delete_xpsServiceIdData_tp(xpsServiceIdData_t_Ptr)

    #/********************************************************************************/
    # command for xpsVlanServiceInstanceGetConfig
    #/********************************************************************************/
    def do_vlan_service_instance_get_config(self, arg):
        '''
         xpsVlanServiceInstanceGetConfig: Enter [ devId,serviceId,type,vlanId,baclEn,raclEn,baclId,raclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,serviceId,type,vlanId,baclEn,raclEn,baclId,raclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            xpsServiceIdData_t_Ptr = new_xpsServiceIdData_tp()
            xpsServiceIdData_t_Ptr.vlanId = int(args[3])
            xpsServiceIdData_t_Ptr.baclEn = int(args[4])
            xpsServiceIdData_t_Ptr.raclEn = int(args[5])
            xpsServiceIdData_t_Ptr.baclId = int(args[6])
            xpsServiceIdData_t_Ptr.raclId = int(args[7])

            #print('Input Arguments are, devId=%d, serviceId=%d, type=%d, vlanId=%d, baclEn=%d, raclEn=%d, baclId=%d, raclId=%d' % (args[0],args[1],args[2],xpsServiceIdData_t_Ptr.vlanId,xpsServiceIdData_t_Ptr.baclEn,xpsServiceIdData_t_Ptr.raclEn,xpsServiceIdData_t_Ptr.baclId,xpsServiceIdData_t_Ptr.raclId))
            ret = xpsVlanServiceInstanceGetConfig(args[0],args[1],args[2],xpsServiceIdData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (xpsServiceIdData_t_Ptr.vlanId))
                print('baclEn = %d' % (xpsServiceIdData_t_Ptr.baclEn))
                print('raclEn = %d' % (xpsServiceIdData_t_Ptr.raclEn))
                print('baclId = %d' % (xpsServiceIdData_t_Ptr.baclId))
                print('raclId = %d' % (xpsServiceIdData_t_Ptr.raclId))
                pass
            delete_xpsServiceIdData_tp(xpsServiceIdData_t_Ptr)

    #/********************************************************************************/
    # command for xpsVlanSetPerVlanControlMac
    #/********************************************************************************/
    def do_vlan_set_per_vlan_control_mac(self, arg):
        '''
         xpsVlanSetPerVlanControlMac: Enter [ devId,vlanId,mac ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,mac ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = args[2].replace(".", ":").replace(",", ":")
            macList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(macList)
            mac = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                mac[listLen - ix - 1] = int(macList[ix], 16)
            #print('Input Arguments are, devId=%d, vlanId=%d, mac=%s' % (args[0],args[1],args[2]))
            ret = xpsVlanSetPerVlanControlMac(args[0],args[1],mac)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanClearPerVlanControlMac
    #/********************************************************************************/
    def do_vlan_clear_per_vlan_control_mac(self, arg):
        '''
         xpsVlanClearPerVlanControlMac: Enter [ devId,vlanId,mac ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,mac ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = args[2].replace(".", ":").replace(",", ":")
            macList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(macList)
            mac = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                mac[listLen - ix - 1] = int(macList[ix], 16)
            #print('Input Arguments are, devId=%d, vlanId=%d, mac=%s' % (args[0],args[1],args[2]))
            ret = xpsVlanClearPerVlanControlMac(args[0],args[1],mac)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanSetGlobalControlMac
    #/********************************************************************************/
    def do_vlan_set_global_control_mac(self, arg):
        '''
         xpsVlanSetGlobalControlMac: Enter [ devId,mac ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,mac ]')
        else:
            args[0] = int(args[0])
            args[1] = args[1].replace(".", ":").replace(",", ":")
            macList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(macList)
            mac = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                mac[listLen - ix - 1] = int(macList[ix], 16)
            #print('Input Arguments are, devId=%d, mac=%s' % (args[0],args[1]))
            ret = xpsVlanSetGlobalControlMac(args[0],mac)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanSetGlobalControlMacWithAction
    #/********************************************************************************/
    def do_vlan_set_global_control_mac_with_action(self, arg):
        '''
         xpsVlanSetGlobalControlMacWithAction: Enter [ devId,mac ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,mac ]')
        else:
            args[0] = int(args[0])
            args[1] = args[1].replace(".", ":").replace(",", ":")
            macList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(macList)
            mac = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                mac[listLen - ix - 1] = int(macList[ix], 16)
            #print('Input Arguments are, devId=%d, mac=%s' % (args[0],args[1]))
            ret = xpsVlanSetGlobalControlMacWithAction(args[0],mac)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanClearGlobalControlMac
    #/********************************************************************************/
    def do_vlan_clear_global_control_mac(self, arg):
        '''
         xpsVlanClearGlobalControlMac: Enter [ devId,mac ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,mac ]')
        else:
            args[0] = int(args[0])
            args[1] = args[1].replace(".", ":").replace(",", ":")
            macList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(macList)
            mac = macAddr_t()
            for ix in range(listLen-1, -1, -1):
                mac[listLen - ix - 1] = int(macList[ix], 16)
            #print('Input Arguments are, devId=%d, mac=%s' % (args[0],args[1]))
            ret = xpsVlanClearGlobalControlMac(args[0],mac)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanSetBridgeAclEnable
    #/********************************************************************************/
    def do_vlan_set_bridge_acl_enable(self, arg):
        '''
         xpsVlanSetBridgeAclEnable: Enter [ devId,vlanId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetBridgeAclEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetBridgeAclEnable
    #/********************************************************************************/
    def do_vlan_get_bridge_acl_enable(self, arg):
        '''
         xpsVlanGetBridgeAclEnable: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetBridgeAclEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)

    #/********************************************************************************/
    # command for xpsVlanSetBridgeAclId
    #/********************************************************************************/
    def do_vlan_set_bridge_acl_id(self, arg):
        '''
         xpsVlanSetBridgeAclId: Enter [ devId,vlanId,aclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,aclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, aclId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetBridgeAclId(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetBridgeAclId
    #/********************************************************************************/
    def do_vlan_get_bridge_acl_id(self, arg):
        '''
         xpsVlanGetBridgeAclId: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            aclId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetBridgeAclId(args[0],args[1],aclId_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('aclId = %d' % (uint32_tp_value(aclId_Ptr_2)))
                pass
            delete_uint32_tp(aclId_Ptr_2)

    #/********************************************************************************/
    # command for xpsVlanGetFirst
    #/********************************************************************************/
    def do_vlan_get_first(self, arg):
        '''
         xpsVlanGetFirst: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            vlanId_Ptr_1 = new_xpsVlan_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsVlanGetFirst(args[0],vlanId_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (xpsVlan_tp_value(vlanId_Ptr_1)))
                pass
            delete_xpsVlan_tp(vlanId_Ptr_1)

    #/********************************************************************************/
    # command for xpsVlanGetNext
    #/********************************************************************************/
    def do_vlan_get_next(self, arg):
        '''
         xpsVlanGetNext: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            vlanIdNext_Ptr_2 = new_xpsVlan_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetNext(args[0],args[1],vlanIdNext_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanIdNext = %d' % (xpsVlan_tp_value(vlanIdNext_Ptr_2)))
                pass
            delete_xpsVlan_tp(vlanIdNext_Ptr_2)
    #/********************************************************************************/
    # command for xpsVlanGetFdbCount
    #/********************************************************************************/
    def do_vlan_get_fdb_count(self, arg):
        '''
         xpsVlanGetFdbCount: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            numFdbEntries_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetFdbCount(args[0],args[1],numFdbEntries_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('numFdbEntries = %d' % (uint32_tp_value(numFdbEntries_Ptr_2)))
                pass
            delete_uint32_tp(numFdbEntries_Ptr_2)


    #/********************************************************************************/
    # command for xpsVlanSetFdbLimit
    #/********************************************************************************/
    def do_vlan_set_fdb_limit(self, arg):
        '''
         xpsVlanSetFdbLimit: Enter [ devId,vlanId,fdbLimit ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,fdbLimit ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, fdbLimit=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetFdbLimit(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass


    #/********************************************************************************/
    # command for xpsVlanGetFdbLimit
    #/********************************************************************************/
    def do_vlan_get_fdb_limit(self, arg):
        '''
         xpsVlanGetFdbLimit: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) or (len(args) > numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fdbLimit_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetFdbLimit(args[0],args[1],fdbLimit_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('fdbLimit = %d' % (uint32_tp_value(fdbLimit_Ptr_2)))
                pass
            delete_uint32_tp(fdbLimit_Ptr_2)
    #/********************************************************************************/
    # command for xpsVlanSetBumPolicerEn
    #/********************************************************************************/
    def do_vlan_set_bum_policer_en(self, arg):
        '''
         xpsVlanSetBumPolicerEn: Enter [ devId,vlanId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetBumPolicerEn(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanGetBumPolicerEn
    #/********************************************************************************/
    def do_vlan_get_bum_policer_en(self, arg):
        '''
         xpsVlanGetBumPolicerEn: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetBumPolicerEn(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsVlanSetBumPolicerId
    #/********************************************************************************/
    def do_vlan_set_bum_policer_id(self, arg):
        '''
         xpsVlanSetBumPolicerId: Enter [ devId,vlanId,policerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,policerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, policerId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetBumPolicerId(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanGetBumPolicerId
    #/********************************************************************************/
    def do_vlan_get_bum_policer_id(self, arg):
        '''
         xpsVlanGetBumPolicerId: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            policerId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetBumPolicerId(args[0],args[1],policerId_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('policerId = %d' % (uint32_tp_value(policerId_Ptr_2)))
                pass
            delete_uint32_tp(policerId_Ptr_2)
    #/********************************************************************************/
    # command for xpsVlanGetStagTpid
    #/********************************************************************************/
    def do_vlan_get_stag_tpid(self, arg):
        '''
         xpsVlanGetStagTpid: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            tpId_Ptr_1 = new_uint16_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsVlanGetStagTpid(args[0],tpId_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('tpId = 0x%x' % (uint16_tp_value(tpId_Ptr_1)))
                pass
            delete_uint16_tp(tpId_Ptr_1)
    #/********************************************************************************/
    # command for xpsVlanSetStagTpid
    #/********************************************************************************/
    def do_vlan_set_stag_tpid(self, arg):
        '''
         xpsVlanSetStagTpid: Enter [ devId,tpId,dir ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tpId,dir ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1], 16)
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, tpId=%d, dir=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetStagTpid(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for bdCountMode enumerations
    #/********************************************************************************/
    def complete_vlan_set_count_mode(self, text, line, begidx, endidx):
        tempDict = { 3 : 'bdCountMode'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetCountMode
    #/********************************************************************************/
    def do_vlan_set_count_mode(self, arg):
        '''
         xpsVlanSetCountMode: Enter [ devId,vlanId,mode ]
		 Valid values for mode : <bdCountMode>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,mode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, mode=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetCountMode(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanGetCountMode
    #/********************************************************************************/
    def do_vlan_get_count_mode(self, arg):
        '''
         xpsVlanGetCountMode: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            mode_Ptr_2 = new_xpsVlanCountMode_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetCountMode(args[0],args[1],mode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mode = %d' % (xpsVlanCountMode_ep_value(mode_Ptr_2)))
                pass
            delete_xpsVlanCountMode_ep(mode_Ptr_2)
    #/********************************************************************************/
    # command for xpsVlanSetEgressCounterId
    #/********************************************************************************/
    def do_vlan_set_egress_counter_id(self, arg):
        '''
         xpsVlanSetEgressCounterId: Enter [ devId,vlanId,counterId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,counterId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, counterId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetEgressCounterId(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanGetEgressCounterId
    #/********************************************************************************/
    def do_vlan_get_egress_counter_id(self, arg):
        '''
         xpsVlanGetEgressCounterId: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            counterId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetEgressCounterId(args[0],args[1],counterId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('counterId = %d' % (uint32_tp_value(counterId_Ptr_2)))
                pass
            delete_uint32_tp(counterId_Ptr_2)
    #/********************************************************************************/
    # command for xpsVlanSetCountingEnable
    #/********************************************************************************/
    def do_vlan_set_counting_enable(self, arg):
        '''
         xpsVlanSetCountingEnable: Enter [ devId,dir,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,dir,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, dir=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetCountingEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanSetIntfCountingEnable
    #/********************************************************************************/
    def do_vlan_set_intf_counting_enable(self, arg):
        '''
         xpsVlanSetIntfCountingEnable: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsVlanSetIntfCountingEnable(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanSetBumPolicingEnable
    #/********************************************************************************/
    def do_vlan_set_bum_policing_enable(self, arg):
        '''
         xpsVlanSetBumPolicingEnable: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsVlanSetBumPolicingEnable(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanSetIPv4UnregMcastL2InterfaceList
    #/********************************************************************************/
    def do_vlan_set_i_pv4_unreg_mcast_l2_interface_list(self, arg):
        '''
         xpsVlanSetIPv4UnregMcastL2InterfaceList: Enter [ devId,vlanId,listId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,listId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, listId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetIPv4UnregMcastL2InterfaceList(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanGetIPv4UnregMcastL2InterfaceList
    #/********************************************************************************/
    def do_vlan_get_i_pv4_unreg_mcast_l2_interface_list(self, arg):
        '''
         xpsVlanGetIPv4UnregMcastL2InterfaceList: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            listId_Ptr_2 = new_xpsMcL2InterfaceListId_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetIPv4UnregMcastL2InterfaceList(args[0],args[1],listId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('listId = %d' % (xpsMcL2InterfaceListId_tp_value(listId_Ptr_2)))
                xpShellGlobals.cmdRetVal = (xpsMcL2InterfaceListId_tp_value(listId_Ptr_2))
                pass
            delete_xpsMcL2InterfaceListId_tp(listId_Ptr_2)
    #/********************************************************************************/
    # command for xpsVlanResetIPv4UnregMcastL2InterfaceList
    #/********************************************************************************/
    def do_vlan_reset_i_pv4_unreg_mcast_l2_interface_list(self, arg):
        '''
         xpsVlanResetIPv4UnregMcastL2InterfaceList: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanResetIPv4UnregMcastL2InterfaceList(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanSetIPv6UnregMcastL2InterfaceList
    #/********************************************************************************/
    def do_vlan_set_i_pv6_unreg_mcast_l2_interface_list(self, arg):
        '''
         xpsVlanSetIPv6UnregMcastL2InterfaceList: Enter [ devId,vlanId,listId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,listId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, listId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetIPv6UnregMcastL2InterfaceList(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanGetIPv6UnregMcastL2InterfaceList
    #/********************************************************************************/
    def do_vlan_get_i_pv6_unreg_mcast_l2_interface_list(self, arg):
        '''
         xpsVlanGetIPv6UnregMcastL2InterfaceList: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            listId_Ptr_2 = new_xpsMcL2InterfaceListId_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetIPv6UnregMcastL2InterfaceList(args[0],args[1],listId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('listId = %d' % (xpsMcL2InterfaceListId_tp_value(listId_Ptr_2)))
                xpShellGlobals.cmdRetVal = (xpsMcL2InterfaceListId_tp_value(listId_Ptr_2))
                pass
            delete_xpsMcL2InterfaceListId_tp(listId_Ptr_2)
    #/********************************************************************************/
    # command for xpsVlanResetIPv6UnregMcastL2InterfaceList
    #/********************************************************************************/
    def do_vlan_reset_i_pv6_unreg_mcast_l2_interface_list(self, arg):
        '''
         xpsVlanResetIPv6UnregMcastL2InterfaceList: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanResetIPv6UnregMcastL2InterfaceList(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for pktCmd enumerations
    #/********************************************************************************/
    def complete_vlan_set_igmp_cmd(self, text, line, begidx, endidx):
        tempDict = { 3 : 'pktCmd'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetIgmpCmd
    #/********************************************************************************/
    def do_vlan_set_igmp_cmd(self, arg):
        '''
        xpsVlanSetIgmpCmd: Enter [ devId,vlanId,igmpCmd ]
		Valid values for igmpCmd : <pktCmd>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,igmpCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, igmpCmd=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetIgmpCmd(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetIgmpCmd
    #/********************************************************************************/
    def do_vlan_get_igmp_cmd(self, arg):
        '''
        xpsVlanGetIgmpCmd: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            igmpCmd_Ptr_2 = new_xpsPktCmd_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetIgmpCmd(args[0],args[1],igmpCmd_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('igmpCmd = %d' % (xpsPktCmd_ep_value(igmpCmd_Ptr_2)))
                pass
            delete_xpsPktCmd_ep(igmpCmd_Ptr_2)

    #/********************************************************************************/
    # Auto completion for pktCmd enumerations
    #/********************************************************************************/
    def complete_vlan_set_icmpv6_cmd(self, text, line, begidx, endidx):
        tempDict = { 3 : 'pktCmd'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetIcmpv6Cmd
    #/********************************************************************************/
    def do_vlan_set_icmpv6_cmd(self, arg):
        '''
        xpsVlanSetIcmpv6Cmd: Enter [ devId,vlanId,icmpv6Cmd ]
		Valid values for icmpv6Cmd : <pktCmd>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,icmpv6Cmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, icmpv6Cmd=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetIcmpv6Cmd(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetIcmpCmd
    #/********************************************************************************/
    def do_vlan_get_icmpv6_cmd(self, arg):
        '''
        xpsVlanGetIcmpv6Cmd: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            icmpv6Cmd_Ptr_2 = new_xpsPktCmd_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetIcmpv6Cmd(args[0],args[1],icmpv6Cmd_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('icmpCmd = %d' % (xpsPktCmd_ep_value(icmpv6Cmd_Ptr_2)))
                pass
            delete_xpsPktCmd_ep(icmpv6Cmd_Ptr_2)
     #/********************************************************************************/
    # command for xpsVlanGetPvid
    #/********************************************************************************/
    def do_vlan_get_pvid(self, arg):
        '''
         xpsVlanGetPvid: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pvid_Ptr = new_xpsVlan_tp() 
            #print('Input Arguments are, devId=%d, intfId=%d'  % (args[0],args[1]))
            ret = xpsVlanGetPvid(args[0],args[1],pvid_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err ==0:
                print('Pvid = %d' % (xpsVlan_tp_value(pvid_Ptr)))
                pass
            xpsVlan_tp_value(pvid_Ptr)
    #/********************************************************************************/
    # command for xpsVlanUpdatePvid
    #/********************************************************************************/
    def do_vlan_update_pvid(self, arg):
        '''
         xpsVlanUpdatePvid: Enter [ devId,intfId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, intfId=%d, vlanId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanUpdatePvid(args[0],args[1],args[2])
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
	#/********************************************************************************/
    # command for xpsVlanGetVlanFloodVIF
    #/********************************************************************************/
    def do_vlan_get_vlan_flood_vif(self, arg):
        '''
         xpsVlanGetVlanFloodVIF: Enter [ scopeId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            floodVif_Ptr_2 = new_xpVif_tp()
            #print('Input Arguments are, scopeId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetVlanFloodVIF(args[0],args[1],floodVif_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('floodVif = %d' % (xpVif_tp_value(floodVif_Ptr_2)))
                pass
            delete_xpVif_tp(floodVif_Ptr_2)
