#!/usr/bin/env python
#  xpsMulticast.py
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
# The class object for xpsMulticast operations
#/**********************************************************************************/

class xpsMulticastObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsMulticastInitScope
    #/********************************************************************************/
    def do_multicast_init_scope(self, arg):
        '''
         xpsMulticastInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsMulticastInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMulticastInit
    #/********************************************************************************/
    def do_multicast_init(self, arg):
        '''
         xpsMulticastInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsMulticastInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMulticastDeInitScope
    #/********************************************************************************/
    def do_multicast_de_init_scope(self, arg):
        '''
         xpsMulticastDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsMulticastDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMulticastDeInit
    #/********************************************************************************/
    def do_multicast_de_init(self, arg):
        '''
         xpsMulticastDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsMulticastDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastAddDevice
    #/********************************************************************************/
    def do_multicast_add_device(self, arg):
        '''
         xpsMulticastAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsMulticastAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastRemoveDevice
    #/********************************************************************************/
    def do_multicast_remove_device(self, arg):
        '''
         xpsMulticastRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMulticastCreateL2InterfaceListScope
    #/********************************************************************************/
    def do_multicast_create_l2_interface_list_scope(self, arg):
        '''
         xpsMulticastCreateL2InterfaceListScope: Enter [ scopeId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            ifListId_Ptr_2 = new_xpsMcL2InterfaceListId_tp()
            #print('Input Arguments are, scopeId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsMulticastCreateL2InterfaceListScope(args[0],args[1],ifListId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('ifListId = %d' % (xpsMcL2InterfaceListId_tp_value(ifListId_Ptr_2)))
                xpShellGlobals.cmdRetVal = ((xpsMcL2InterfaceListId_tp_value(ifListId_Ptr_2)))
                pass
            delete_xpsMcL2InterfaceListId_tp(ifListId_Ptr_2)

    #/********************************************************************************/
    # command for xpsMulticastCreateL2InterfaceList
    #/********************************************************************************/
    def do_multicast_create_l2_interface_list(self, arg):
        '''
         xpsMulticastCreateL2InterfaceList: Enter [ vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ vlanId ]')
        else:
            args[0] = int(args[0])
            ifListId_Ptr_1 = new_xpsMcL2InterfaceListId_tp()
            #print('Input Arguments are, vlanId=%d' % (args[0]))
            ret = xpsMulticastCreateL2InterfaceList(args[0],ifListId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('ifListId = %d' % (xpsMcL2InterfaceListId_tp_value(ifListId_Ptr_1)))
                xpShellGlobals.cmdRetVal = ((xpsMcL2InterfaceListId_tp_value(ifListId_Ptr_1)))
                pass
            delete_xpsMcL2InterfaceListId_tp(ifListId_Ptr_1)

     #/********************************************************************************/
    # command for xpsMulticastCreateL3InterfaceListScope
    #/********************************************************************************/
    def do_multicast_create_l3_interface_list_scope(self, arg):
        '''
         xpsMulticastCreateL3InterfaceListScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            ifListId_Ptr_1 = new_xpsMcL3InterfaceListId_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsMulticastCreateL3InterfaceListScope(args[0],ifListId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('ifListId = %d' % (xpsMcL3InterfaceListId_tp_value(ifListId_Ptr_1)))
                xpShellGlobals.cmdRetVal = ((xpsMcL3InterfaceListId_tp_value(ifListId_Ptr_1)))
                pass
            delete_xpsMcL3InterfaceListId_tp(ifListId_Ptr_1)

    #/********************************************************************************/
    # command for xpsMulticastCreateL3InterfaceList
    #/********************************************************************************/
    def do_multicast_create_l3_interface_list(self, arg):
        '''
         xpsMulticastCreateL3InterfaceList: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            ifListId_Ptr_0 = new_xpsMcL3InterfaceListId_tp()
            #print('Input Arguments are' % ())
            ret = xpsMulticastCreateL3InterfaceList(ifListId_Ptr_0)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('ifListId = %d' % (xpsMcL3InterfaceListId_tp_value(ifListId_Ptr_0)))
                xpShellGlobals.cmdRetVal = ((xpsMcL3InterfaceListId_tp_value(ifListId_Ptr_0)))
                pass
            delete_xpsMcL3InterfaceListId_tp(ifListId_Ptr_0)

    #/********************************************************************************/
    # command for xpsMulticastDestroyL2InterfaceListScope
    #/********************************************************************************/
    def do_multicast_destroy_l2_interface_list_scope(self, arg):
        '''
         xpsMulticastDestroyL2InterfaceListScope: Enter [ scopeId,ifListId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,ifListId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, ifListId=%d' % (args[0],args[1]))
            ret = xpsMulticastDestroyL2InterfaceListScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMulticastDestroyL2InterfaceList
    #/********************************************************************************/
    def do_multicast_destroy_l2_interface_list(self, arg):
        '''
         xpsMulticastDestroyL2InterfaceList: Enter [ ifListId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ifListId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, ifListId=%d' % (args[0]))
            ret = xpsMulticastDestroyL2InterfaceList(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMulticastDestroyL3InterfaceListScope
    #/********************************************************************************/
    def do_multicast_destroy_l3_interface_list_scope(self, arg):
        '''
         xpsMulticastDestroyL3InterfaceListScope: Enter [ scopeId,ifListId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,ifListId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, ifListId=%d' % (args[0],args[1]))
            ret = xpsMulticastDestroyL3InterfaceListScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMulticastDestroyL3InterfaceList
    #/********************************************************************************/
    def do_multicast_destroy_l3_interface_list(self, arg):
        '''
         xpsMulticastDestroyL3InterfaceList: Enter [ ifListId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ifListId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, ifListId=%d' % (args[0]))
            ret = xpsMulticastDestroyL3InterfaceList(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastAddL2InterfaceListToDevice
    #/********************************************************************************/
    def do_multicast_add_l2_interface_list_to_device(self, arg):
        '''
         xpsMulticastAddL2InterfaceListToDevice: Enter [ devId,l2IntfListId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l2IntfListId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, l2IntfListId=%d' % (args[0],args[1]))
            ret = xpsMulticastAddL2InterfaceListToDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastAddL3InterfaceListToDevice
    #/********************************************************************************/
    def do_multicast_add_l3_interface_list_to_device(self, arg):
        '''
         xpsMulticastAddL3InterfaceListToDevice: Enter [ devId,l3IntfListId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfListId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, l3IntfListId=%d' % (args[0],args[1]))
            ret = xpsMulticastAddL3InterfaceListToDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastRemoveL2InterfaceListFromDevice
    #/********************************************************************************/
    def do_multicast_remove_l2_interface_list_from_device(self, arg):
        '''
         xpsMulticastRemoveL2InterfaceListFromDevice: Enter [ devId,l2IntfListId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l2IntfListId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, l2IntfListId=%d' % (args[0],args[1]))
            ret = xpsMulticastRemoveL2InterfaceListFromDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastRemoveL3InterfaceListFromDevice
    #/********************************************************************************/
    def do_multicast_remove_l3_interface_list_from_device(self, arg):
        '''
         xpsMulticastRemoveL3InterfaceListFromDevice: Enter [ devId,l3IntfListId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfListId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, l3IntfListId=%d' % (args[0],args[1]))
            ret = xpsMulticastRemoveL3InterfaceListFromDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastAddInterfaceToL2InterfaceList
    #/********************************************************************************/
    def do_multicast_add_interface_to_l2_interface_list(self, arg):
        '''
         xpsMulticastAddInterfaceToL2InterfaceList: Enter [ devId,l2IntfListId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l2IntfListId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l2IntfListId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastAddInterfaceToL2InterfaceList(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastAddInterfaceToL3InterfaceList
    #/********************************************************************************/
    def do_multicast_add_interface_to_l3_interface_list(self, arg):
        '''
         xpsMulticastAddInterfaceToL3InterfaceList: Enter [ devId,l3IntfListId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfListId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfListId=%d, l3IntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastAddInterfaceToL3InterfaceList(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastAddBridgingInterfaceToL3InterfaceList
    #/********************************************************************************/
    def do_multicast_add_bridging_interface_to_l3_interface_list(self, arg):
        '''
         xpsMulticastAddBridgingInterfaceToL3InterfaceList: Enter [ devId,l3IntfListId,l2IntfListId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfListId,l2IntfListId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfListId=%d, l2IntfListId=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastAddBridgingInterfaceToL3InterfaceList(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastRemoveInterfaceFromL2InterfaceList
    #/********************************************************************************/
    def do_multicast_remove_interface_from_l2_interface_list(self, arg):
        '''
         xpsMulticastRemoveInterfaceFromL2InterfaceList: Enter [ devId,ifListId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,ifListId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, ifListId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastRemoveInterfaceFromL2InterfaceList(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastRemoveInterfaceFromL3InterfaceList
    #/********************************************************************************/
    def do_multicast_remove_interface_from_l3_interface_list(self, arg):
        '''
         xpsMulticastRemoveInterfaceFromL3InterfaceList: Enter [ devId,l3IntfListId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfListId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfListId=%d, l3IntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastRemoveInterfaceFromL3InterfaceList(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastRemoveBridgingInterfaceFromL3InterfaceList
    #/********************************************************************************/
    def do_multicast_remove_bridging_interface_from_l3_interface_list(self, arg):
        '''
         xpsMulticastRemoveBridgingInterfaceFromL3InterfaceList: Enter [ devId,l3IntfListId,l2IntfListId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfListId,l2IntfListId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfListId=%d, l2IntfListId=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastRemoveBridgingInterfaceFromL3InterfaceList(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastAddIPv4BridgeEntry
    #/********************************************************************************/
    def do_multicast_add_i_pv4_bridge_entry(self, arg):
        '''
         xpsMulticastAddIPv4BridgeEntry: Enter [ devId,bdId,vlanId,sourceAddress,groupAddress,multicastVifIdx,mirrorMask,countMode,counterIdx,isControl,isStatic,pktCmd ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 12
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,bdId,vlanId,sourceAddress,groupAddress,multicastVifIdx,mirrorMask,countMode,counterIdx,isControl,isStatic,pktCmd ]')
        else:
            args[0] = int(args[0])
            xpsMulticastIPv4BridgeEntry_t_Ptr = new_xpsMulticastIPv4BridgeEntry_tp()
            xpsMulticastIPv4BridgeEntry_t_Ptr.bdId = int(args[1])
            xpsMulticastIPv4BridgeEntry_t_Ptr.vlanId = int(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsMulticastIPv4BridgeEntry_t_Ptr.sourceAddress[listLen - ix - 1] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsMulticastIPv4BridgeEntry_t_Ptr.groupAddress[listLen - ix - 1] = int(postList[ix])
            xpsMulticastIPv4BridgeEntry_t_Ptr.multicastVifIdx = int(args[5])
            xpsMulticastIPv4BridgeEntry_t_Ptr.mirrorMask = int(args[6])
            xpsMulticastIPv4BridgeEntry_t_Ptr.countMode = int(args[7])
            xpsMulticastIPv4BridgeEntry_t_Ptr.counterIdx = int(args[8])
            xpsMulticastIPv4BridgeEntry_t_Ptr.isControl = int(args[9])
            xpsMulticastIPv4BridgeEntry_t_Ptr.isStatic = int(args[10])
            xpsMulticastIPv4BridgeEntry_t_Ptr.pktCmd = eval(args[11])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, bdId=%d, vlanId=%d, sourceAddress=%s, groupAddress=%s, multicastVifIdx=%d, mirrorMask=%d, countMode=%d, counterIdx=%d, isControl=%d, isStatic=%d, pktCmd=%d' % (args[0],xpsMulticastIPv4BridgeEntry_t_Ptr.bdId,xpsMulticastIPv4BridgeEntry_t_Ptr.vlanId,args[3],args[4],xpsMulticastIPv4BridgeEntry_t_Ptr.multicastVifIdx,xpsMulticastIPv4BridgeEntry_t_Ptr.mirrorMask,xpsMulticastIPv4BridgeEntry_t_Ptr.countMode,xpsMulticastIPv4BridgeEntry_t_Ptr.counterIdx,xpsMulticastIPv4BridgeEntry_t_Ptr.isControl,xpsMulticastIPv4BridgeEntry_t_Ptr.isStatic,xpsMulticastIPv4BridgeEntry_t_Ptr.pktCmd))
            ret = xpsMulticastAddIPv4BridgeEntry(args[0],xpsMulticastIPv4BridgeEntry_t_Ptr,xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('bdId = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.bdId))
                print('vlanId = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.vlanId))
                print('sourceAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv4BridgeEntry_t_Ptr.sourceAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('groupAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv4BridgeEntry_t_Ptr.groupAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('multicastVifIdx = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.multicastVifIdx))
                print('mirrorMask = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.mirrorMask))
                print('countMode = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.countMode))
                print('counterIdx = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.counterIdx))
                print('isControl = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.isControl))
                print('isStatic = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.isStatic))
                print('pktCmd = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.pktCmd))
                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                xpShellGlobals.cmdRetVal = xpsHashIndexList_t_Ptr.index[0]
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
            delete_xpsMulticastIPv4BridgeEntry_tp(xpsMulticastIPv4BridgeEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastAddIPv4RouteEntry
    #/********************************************************************************/
    def do_multicast_add_i_pv4_route_entry(self, arg):
        '''
         xpsMulticastAddIPv4RouteEntry: Enter [ devId,vrfIdx,sourceAddress,groupAddress,multicastVifIdx,mirrorMask,countMode,rpfValue,pktCmd,rpfFailCmd,rpfType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 10
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfIdx,sourceAddress,groupAddress,multicastVifIdx,mirrorMask,countMode,rpfValue,pktCmd,rpfFailCmd,rpfType ]')
        else:
            args[0] = int(args[0])
            xpsMulticastIPv4RouteEntry_t_Ptr = new_xpsMulticastIPv4RouteEntry_tp()
            xpsMulticastIPv4RouteEntry_t_Ptr.vrfIdx = int(args[1])

            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsMulticastIPv4RouteEntry_t_Ptr.sourceAddress[listLen - ix - 1] = int(postList[ix])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsMulticastIPv4RouteEntry_t_Ptr.groupAddress[listLen - ix - 1] = int(postList[ix])
            xpsMulticastIPv4RouteEntry_t_Ptr.multicastVifIdx = int(args[4])
            xpsMulticastIPv4RouteEntry_t_Ptr.mirrorMask = int(args[5])
            xpsMulticastIPv4RouteEntry_t_Ptr.countMode = int(args[6])
            xpsMulticastIPv4RouteEntry_t_Ptr.rpfValue = int(args[7])
            xpsMulticastIPv4RouteEntry_t_Ptr.pktCmd = eval(args[8])
            xpsMulticastIPv4RouteEntry_t_Ptr.rpfFailCmd = eval(args[9])
            xpsMulticastIPv4RouteEntry_t_Ptr.rpfType = eval(args[10])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, vrfIdx=%d, sourceAddress=%s, groupAddress=%s, multicastVifIdx=%d, mirrorMask=%d, countMode=%d, rpfValue=0x%x, pktCmd=%d, rpfFailCmd=%d, rpfType=%d' % (args[0],xpsMulticastIPv4RouteEntry_t_Ptr.vrfIdx,args[2],args[3],xpsMulticastIPv4RouteEntry_t_Ptr.multicastVifIdx,xpsMulticastIPv4RouteEntry_t_Ptr.mirrorMask,xpsMulticastIPv4RouteEntry_t_Ptr.countMode,xpsMulticastIPv4RouteEntry_t_Ptr.rpfValue,xpsMulticastIPv4RouteEntry_t_Ptr.pktCmd,xpsMulticastIPv4RouteEntry_t_Ptr.rpfFailCmd,xpsMulticastIPv4RouteEntry_t_Ptr.rpfType))
            ret = xpsMulticastAddIPv4RouteEntry(args[0],xpsMulticastIPv4RouteEntry_t_Ptr,xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfIdx = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.vrfIdx))
                print('sourceAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv4RouteEntry_t_Ptr.sourceAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('groupAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv4RouteEntry_t_Ptr.groupAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('multicastVifIdx = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.multicastVifIdx))
                print('mirrorMask = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.mirrorMask))
                print('countMode = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.countMode))
                print('rpfValue = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.rpfValue))
                print('pktCmd = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.pktCmd))
                print('rpfFailCmd = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.rpfFailCmd))
                print('rpfType = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.rpfType))
                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                xpShellGlobals.cmdRetVal = xpsHashIndexList_t_Ptr.index[0]
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
            delete_xpsMulticastIPv4RouteEntry_tp(xpsMulticastIPv4RouteEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastAddIPv4PimBiDirRPFEntry
    #/********************************************************************************/
    def do_multicast_add_i_pv4_pim_bi_dir_rpf_entry(self, arg):
        '''
         xpsMulticastAddIPv4PimBiDirRPFEntry: Enter [ devId,vrfIdx,bdId,groupAddress,rpfValue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfIdx,bdId,groupAddress,rpfValue ]')
        else:
            args[0] = int(args[0])
            xpsMulticastIPv4PimBidirRpfEntry_t_Ptr = new_xpsMulticastIPv4PimBidirRpfEntry_tp()
            xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.vrfIdx = int(args[1])
            xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.bdId = int(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.groupAddress[listLen - ix - 1] = int(postList[ix])
            xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.rpfValue = int(args[4])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, vrfIdx=%d, bdId=%d, groupAddress=%s, rpfValue=0x%x' % (args[0],xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.vrfIdx,xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.bdId,args[3],xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.rpfValue))
            ret = xpsMulticastAddIPv4PimBiDirRPFEntry(args[0],xpsMulticastIPv4PimBidirRpfEntry_t_Ptr,xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfIdx = %d' % (xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.vrfIdx))
                print('bdId = %d' % (xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.bdId))
                print('groupAddress = '),
                listLen=4
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.groupAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('rpfValue = %d' % (xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.rpfValue))
                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
            delete_xpsMulticastIPv4PimBidirRpfEntry_tp(xpsMulticastIPv4PimBidirRpfEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastRemoveIPv4BridgeEntry
    #/********************************************************************************/
    def do_multicast_remove_i_pv4_bridge_entry(self, arg):
        '''
         xpsMulticastRemoveIPv4BridgeEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastRemoveIPv4BridgeEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastRemoveIPv4RouteEntry
    #/********************************************************************************/
    def do_multicast_remove_i_pv4_route_entry(self, arg):
        '''
         xpsMulticastRemoveIPv4RouteEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastRemoveIPv4RouteEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastRemoveIPv4PimBiDirRPFEntry
    #/********************************************************************************/
    def do_multicast_remove_i_pv4_pim_bi_dir_rpf_entry(self, arg):
        '''
         xpsMulticastRemoveIPv4PimBiDirRPFEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastRemoveIPv4PimBiDirRPFEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIPv4BridgeEntryIndex
    #/********************************************************************************/
    def do_multicast_get_ipv4_bridge_entry(self, arg):
        '''
         xpsMulticastGetIPv4BridgeEntryIndex: Enter [devId,bdId,vlanId,sourceAddress,groupAddress]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [devId,bdId,vlanId,sourceAddress,groupAddress]')
        else:
            args[0] = int(args[0])
            xpsMulticastIPv4BridgeEntry_t_Ptr = new_xpsMulticastIPv4BridgeEntry_tp()
            xpsMulticastIPv4BridgeEntry_t_Ptr.bdId = int(args[1])
            xpsMulticastIPv4BridgeEntry_t_Ptr.vlanId = int(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsMulticastIPv4BridgeEntry_t_Ptr.sourceAddress[listLen - ix - 1] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsMulticastIPv4BridgeEntry_t_Ptr.groupAddress[listLen - ix - 1] = int(postList[ix])
            int32_t_Ptr = new_uint32_tp()
            #print('Input Arguments are, devId=%d, bdId=%d, vlanId=%d, sourceAddress=%s, groupAddress=%s' % (args[0],xpsMulticastIPv4BridgeEntry_t_Ptr.bdId,xpsMulticastIPv4BridgeEntry_t_Ptr.vlanId,args[3],args[4]))
            ret = xpsMulticastGetIPv4BridgeEntryIndex(args[0],xpsMulticastIPv4BridgeEntry_t_Ptr,int32_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('bdId = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.bdId))
                print('vlanId = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.vlanId))
                print('sourceAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv4BridgeEntry_t_Ptr.sourceAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('groupAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv4BridgeEntry_t_Ptr.groupAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('Index = %d' % uint32_tp_value(int32_t_Ptr))
                print('')
                xpShellGlobals.cmdRetVal = uint32_tp_value(int32_t_Ptr)
                pass
            delete_uint32_tp(int32_t_Ptr)
            delete_xpsMulticastIPv4BridgeEntry_tp(xpsMulticastIPv4BridgeEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastGetIPv4BridgeEntry
    #/********************************************************************************/
    def do_multicast_get_i_pv4_bridge_entry(self, arg):
        '''
         xpsMulticastGetIPv4BridgeEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsMulticastIPv4BridgeEntry_t_Ptr = new_xpsMulticastIPv4BridgeEntry_tp()

            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastGetIPv4BridgeEntry(args[0],args[1],xpsMulticastIPv4BridgeEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('bdId = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.bdId))
                print('sourceAddress = '),
                listLen=4
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv4BridgeEntry_t_Ptr.sourceAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('groupAddress = '),
                listLen=4
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv4BridgeEntry_t_Ptr.groupAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('multicastVifIdx = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.multicastVifIdx))
                print('mirrorMask = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.mirrorMask))
                print('countMode = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.countMode))
                print('counterIdx = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.counterIdx))
                print('isControl = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.isControl))
                print('isStatic = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.isStatic))
                print('pktCmd = %d' % (xpsMulticastIPv4BridgeEntry_t_Ptr.pktCmd))
                pass
            delete_xpsMulticastIPv4BridgeEntry_tp(xpsMulticastIPv4BridgeEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastGetIPv4RouteEntry
    #/********************************************************************************/
    def do_multicast_get_i_pv4_route_entry(self, arg):
        '''
         xpsMulticastGetIPv4RouteEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsMulticastIPv4RouteEntry_t_Ptr = new_xpsMulticastIPv4RouteEntry_tp()

            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastGetIPv4RouteEntry(args[0],args[1],xpsMulticastIPv4RouteEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfIdx = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.vrfIdx))
                print('sourceAddress = '),
                listLen=4
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv4RouteEntry_t_Ptr.sourceAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('groupAddress = '),
                listLen=4
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv4RouteEntry_t_Ptr.groupAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('multicastVifIdx = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.multicastVifIdx))
                print('mirrorMask = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.mirrorMask))
                print('countMode = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.countMode))
                print('rpfValue = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.rpfValue))
                print('pktCmd = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.pktCmd))
                print('rpfFailCmd = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.rpfFailCmd))
                print('rpfType = %d' % (xpsMulticastIPv4RouteEntry_t_Ptr.rpfType))
                pass
            delete_xpsMulticastIPv4RouteEntry_tp(xpsMulticastIPv4RouteEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastGetIPv4PimBiDirRPFEntry
    #/********************************************************************************/
    def do_multicast_get_i_pv4_pim_bi_dir_rpf_entry(self, arg):
        '''
         xpsMulticastGetIPv4PimBiDirRPFEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsMulticastIPv4PimBidirRpfEntry_t_Ptr = new_xpsMulticastIPv4PimBidirRpfEntry_tp()

            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastGetIPv4PimBiDirRPFEntry(args[0],args[1],xpsMulticastIPv4PimBidirRpfEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfIdx = %d' % (xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.vrfIdx))
                print('bdId = %d' % (xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.bdId))
                print('groupAddress = '),
                listLen=4
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.groupAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('rpfValue = %d' % (xpsMulticastIPv4PimBidirRpfEntry_t_Ptr.rpfValue))
                pass
            delete_xpsMulticastIPv4PimBidirRpfEntry_tp(xpsMulticastIPv4PimBidirRpfEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastAddIPv6BridgeEntry
    #/********************************************************************************/
    def do_multicast_add_i_pv6_bridge_entry(self, arg):
        '''
         xpsMulticastAddIPv6BridgeEntry: Enter [ devId,bdId,vlanId,ipv6McL2DomainId,sourceAddress,groupAddress,multicastVifIdx,mirrorMask,countMode,counterIdx,isControl,isStatic,pktCmd ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 12
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,bdId,vlanId,ipv6McL2DomainId,sourceAddress,groupAddress,multicastVifIdx,mirrorMask,countMode,counterIdx,isControl,isStatic,pktCmd ]')
        else:
            args[0] = int(args[0])
            xpsMulticastIPv6BridgeEntry_t_Ptr = new_xpsMulticastIPv6BridgeEntry_tp()
            xpsMulticastIPv6BridgeEntry_t_Ptr.bdId = int(args[1])
            xpsMulticastIPv6BridgeEntry_t_Ptr.vlanId = int(args[2])
            xpsMulticastIPv6BridgeEntry_t_Ptr.ipv6McL2DomainId = int(args[3])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsMulticastIPv6BridgeEntry_t_Ptr.sourceAddress[listLen - ix - 1] = int(postList[ix], 16)

            args[5] = args[5].replace(".",":").replace(",",":")
            postList = args[5].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsMulticastIPv6BridgeEntry_t_Ptr.groupAddress[listLen - ix - 1] = int(postList[ix], 16)
            xpsMulticastIPv6BridgeEntry_t_Ptr.multicastVifIdx = int(args[6])
            xpsMulticastIPv6BridgeEntry_t_Ptr.mirrorMask = int(args[7])
            xpsMulticastIPv6BridgeEntry_t_Ptr.countMode = int(args[8])
            xpsMulticastIPv6BridgeEntry_t_Ptr.counterIdx = int(args[9])
            xpsMulticastIPv6BridgeEntry_t_Ptr.isControl = int(args[10])
            xpsMulticastIPv6BridgeEntry_t_Ptr.isStatic = int(args[11])
            xpsMulticastIPv6BridgeEntry_t_Ptr.pktCmd = eval(args[12])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, bdId=%d, vlanId=%d, ipv6McL2DomainId=%d, sourceAddress=%s, groupAddress=%s, multicastVifIdx=%d, mirrorMask=%d, countMode=%d, counterIdx=%d, isControl=%d, isStatic=%d, pktCmd=%d' % (args[0],xpsMulticastIPv6BridgeEntry_t_Ptr.bdId,xpsMulticastIPv6BridgeEntry_t_Ptr.vlanId,xpsMulticastIPv6BridgeEntry_t_Ptr.ipv6McL2DomainId,args[3],args[4],xpsMulticastIPv6BridgeEntry_t_Ptr.multicastVifIdx,xpsMulticastIPv6BridgeEntry_t_Ptr.mirrorMask,xpsMulticastIPv6BridgeEntry_t_Ptr.countMode,xpsMulticastIPv6BridgeEntry_t_Ptr.counterIdx,xpsMulticastIPv6BridgeEntry_t_Ptr.isControl,xpsMulticastIPv6BridgeEntry_t_Ptr.isStatic,xpsMulticastIPv6BridgeEntry_t_Ptr.pktCmd))
            ret = xpsMulticastAddIPv6BridgeEntry(args[0],xpsMulticastIPv6BridgeEntry_t_Ptr,xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('bdId = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.bdId))
                print('vlanId = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.vlanId))
                print('ipv6McL2DomainId = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.ipv6McL2DomainId))
                print('sourceAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv6BridgeEntry_t_Ptr.sourceAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('groupAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv6BridgeEntry_t_Ptr.groupAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('multicastVifIdx = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.multicastVifIdx))
                print('mirrorMask = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.mirrorMask))
                print('countMode = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.countMode))
                print('counterIdx = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.counterIdx))
                print('isControl = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.isControl))
                print('isStatic = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.isStatic))
                print('pktCmd = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.pktCmd))
                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
            delete_xpsMulticastIPv6BridgeEntry_tp(xpsMulticastIPv6BridgeEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastAddIPv6RouteEntry
    #/********************************************************************************/
    def do_multicast_add_i_pv6_route_entry(self, arg):
        '''
         xpsMulticastAddIPv6RouteEntry: Enter [ devId,vrfIdx,ipv6McL3DomainId,sourceAddress,groupAddress,multicastVifIdx,mirrorMask,countMode,rpfValue,pktCmd,rpfFailCmd,rpfType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 11
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfIdx,ipv6McL3DomainId,sourceAddress,groupAddress,multicastVifIdx,mirrorMask,countMode,rpfValue,pktCmd,rpfFailCmd,rpfType ]')
        else:
            args[0] = int(args[0])
            xpsMulticastIPv6RouteEntry_t_Ptr = new_xpsMulticastIPv6RouteEntry_tp()
            xpsMulticastIPv6RouteEntry_t_Ptr.vrfIdx = int(args[1])
            xpsMulticastIPv6RouteEntry_t_Ptr.ipv6McL3DomainId = int(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsMulticastIPv6RouteEntry_t_Ptr.sourceAddress[listLen - ix - 1] = int(postList[ix], 16)

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsMulticastIPv6RouteEntry_t_Ptr.groupAddress[listLen - ix - 1] = int(postList[ix], 16)
            xpsMulticastIPv6RouteEntry_t_Ptr.multicastVifIdx = int(args[5])
            xpsMulticastIPv6RouteEntry_t_Ptr.mirrorMask = int(args[6])
            xpsMulticastIPv6RouteEntry_t_Ptr.countMode = int(args[7])
            xpsMulticastIPv6RouteEntry_t_Ptr.rpfValue = int(args[8])
            xpsMulticastIPv6RouteEntry_t_Ptr.pktCmd = eval(args[9])
            xpsMulticastIPv6RouteEntry_t_Ptr.rpfFailCmd = eval(args[10])
            xpsMulticastIPv6RouteEntry_t_Ptr.rpfType = eval(args[11])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, vrfIdx=%d, ipv6McL3DomainId=%d, sourceAddress=%s, groupAddress=%s, multicastVifIdx=%d, mirrorMask=%d, countMode=%d, rpfValue=0x%x, pktCmd=%d, rpfFailCmd=%d, rpfType=%d' % (args[0],xpsMulticastIPv6RouteEntry_t_Ptr.vrfIdx,xpsMulticastIPv6RouteEntry_t_Ptr.ipv6McL3DomainId,args[3],args[4],xpsMulticastIPv6RouteEntry_t_Ptr.multicastVifIdx,xpsMulticastIPv6RouteEntry_t_Ptr.mirrorMask,xpsMulticastIPv6RouteEntry_t_Ptr.countMode,xpsMulticastIPv6RouteEntry_t_Ptr.rpfValue,xpsMulticastIPv6RouteEntry_t_Ptr.pktCmd,xpsMulticastIPv6RouteEntry_t_Ptr.rpfFailCmd,xpsMulticastIPv6RouteEntry_t_Ptr.rpfType))
            ret = xpsMulticastAddIPv6RouteEntry(args[0],xpsMulticastIPv6RouteEntry_t_Ptr,xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfIdx = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.vrfIdx))
                print('ipv6McL3DomainId = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.ipv6McL3DomainId))
                print('sourceAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv6RouteEntry_t_Ptr.sourceAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('groupAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv6RouteEntry_t_Ptr.groupAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('multicastVifIdx = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.multicastVifIdx))
                print('mirrorMask = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.mirrorMask))
                print('countMode = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.countMode))
                print('rpfValue = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.rpfValue))
                print('pktCmd = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.pktCmd))
                print('rpfFailCmd = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.rpfFailCmd))
                print('rpfType = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.rpfType))
                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
            delete_xpsMulticastIPv6RouteEntry_tp(xpsMulticastIPv6RouteEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastRemoveIPv6BridgeEntry
    #/********************************************************************************/
    def do_multicast_remove_i_pv6_bridge_entry(self, arg):
        '''
         xpsMulticastRemoveIPv6BridgeEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastRemoveIPv6BridgeEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastRemoveIPv6RouteEntry
    #/********************************************************************************/
    def do_multicast_remove_i_pv6_route_entry(self, arg):
        '''
         xpsMulticastRemoveIPv6RouteEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastRemoveIPv6RouteEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIPv6BridgeEntry
    #/********************************************************************************/
    def do_multicast_get_i_pv6_bridge_entry(self, arg):
        '''
         xpsMulticastGetIPv6BridgeEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsMulticastIPv6BridgeEntry_t_Ptr = new_xpsMulticastIPv6BridgeEntry_tp()
            ret = xpsMulticastGetIPv6BridgeEntry(args[0],args[1],xpsMulticastIPv6BridgeEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('bdId = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.bdId))
                print('ipv6McL2DomainId = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.ipv6McL2DomainId))
                print('sourceAddress = '),
                listLen = 6
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv6BridgeEntry_t_Ptr.sourceAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('groupAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv6BridgeEntry_t_Ptr.groupAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('multicastVifIdx = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.multicastVifIdx))
                print('mirrorMask = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.mirrorMask))
                print('countMode = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.countMode))
                print('counterIdx = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.counterIdx))
                print('isControl = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.isControl))
                print('isStatic = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.isStatic))
                print('pktCmd = %d' % (xpsMulticastIPv6BridgeEntry_t_Ptr.pktCmd))
                pass
            delete_xpsMulticastIPv6BridgeEntry_tp(xpsMulticastIPv6BridgeEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastGetIPv6RouteEntry
    #/********************************************************************************/
    def do_multicast_get_i_pv6_route_entry(self, arg):
        '''
         xpsMulticastGetIPv6RouteEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsMulticastIPv6RouteEntry_t_Ptr = new_xpsMulticastIPv6RouteEntry_tp()
            ret = xpsMulticastGetIPv6RouteEntry(args[0],args[1],xpsMulticastIPv6RouteEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfIdx = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.vrfIdx))
                print('ipv6McL3DomainId = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.ipv6McL3DomainId))
                print('sourceAddress = '),
                listLen = 6
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv6RouteEntry_t_Ptr.sourceAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('groupAddress = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsMulticastIPv6RouteEntry_t_Ptr.groupAddress[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('multicastVifIdx = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.multicastVifIdx))
                print('mirrorMask = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.mirrorMask))
                print('countMode = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.countMode))
                print('rpfValue = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.rpfValue))
                print('pktCmd = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.pktCmd))
                print('rpfFailCmd = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.rpfFailCmd))
                print('rpfType = %d' % (xpsMulticastIPv6RouteEntry_t_Ptr.rpfType))
                pass
            delete_xpsMulticastIPv6RouteEntry_tp(xpsMulticastIPv6RouteEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastTriggerIpv4BridgeTableAging
    #/********************************************************************************/
    def do_multicast_trigger_ipv4_bridge_table_aging(self, arg):
        '''
         xpsMulticastTriggerIpv4BridgeTableAging: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastTriggerIpv4BridgeTableAging(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastConfigureIpv4BridgeTableAging
    #/********************************************************************************/
    def do_multicast_configure_ipv4_bridge_table_aging(self, arg):
        '''
         xpsMulticastConfigureIpv4BridgeTableAging: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsMulticastConfigureIpv4BridgeTableAging(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastConfigureIpv4BridgeEntryAging
    #/********************************************************************************/
    def do_multicast_configure_ipv4_bridge_entry_aging(self, arg):
        '''
         xpsMulticastConfigureIpv4BridgeEntryAging: Enter [ devId,enable,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, enable=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastConfigureIpv4BridgeEntryAging(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv4BridgeTableAgingStatus
    #/********************************************************************************/
    def do_multicast_get_ipv4_bridge_table_aging_status(self, arg):
        '''
         xpsMulticastGetIpv4BridgeTableAgingStatus: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_boolp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv4BridgeTableAgingStatus(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (boolp_value(enable_Ptr_1)))
                pass
            delete_boolp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastGetIpv4BridgeEntryAgingStatus
    #/********************************************************************************/
    def do_multicast_get_ipv4_bridge_entry_aging_status(self, arg):
        '''
         xpsMulticastGetIpv4BridgeEntryAgingStatus: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_boolp()
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastGetIpv4BridgeEntryAgingStatus(args[0],enable_Ptr_1,args[1])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (boolp_value(enable_Ptr_1)))
                pass
            delete_boolp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastSetIpv4BridgeTableAgingCycle
    #/********************************************************************************/
    def do_multicast_set_ipv4_bridge_table_aging_cycle(self, arg):
        '''
         xpsMulticastSetIpv4BridgeTableAgingCycle: Enter [ devId,sec ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sec ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, sec=%d' % (args[0],args[1]))
            ret = xpsMulticastSetIpv4BridgeTableAgingCycle(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv4BridgeTableAgingCycle
    #/********************************************************************************/
    def do_multicast_get_ipv4_bridge_table_aging_cycle(self, arg):
        '''
         xpsMulticastGetIpv4BridgeTableAgingCycle: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            sec_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv4BridgeTableAgingCycle(args[0],sec_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('sec = %d' % (uint32_tp_value(sec_Ptr_1)))
                pass
            delete_uint32_tp(sec_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastRegisterIpv4BridgeTableAgingHandler
    #/********************************************************************************/
    def do_multicast_register_ipv4_bridge_table_aging_handler(self, arg):
        '''
         xpsMulticastRegisterIpv4BridgeTableAgingHandler: Enter [ devId,func ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,func ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            userData_Ptr_2 = new_voidp()
            #print('Input Arguments are, devId=%d, func=%d' % (args[0],args[1]))
            ret = xpsMulticastRegisterIpv4BridgeTableAgingHandler(args[0],args[1],userData_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('userData = %d' % (voidp_value(userData_Ptr_2)))
                pass
            delete_voidp(userData_Ptr_2)
    #/********************************************************************************/
    # command for xpsMulticastRegisterIpv4BridgeTableDefaultAgingHandler
    #/********************************************************************************/
    def do_multicast_register_ipv4_bridge_table_default_aging_handler(self, arg):
        '''
         xpsMulticastRegisterIpv4BridgeTableDefaultAgingHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastRegisterIpv4BridgeTableDefaultAgingHandler(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastUnregisterIpv4BridgeTableAgingHandler
    #/********************************************************************************/
    def do_multicast_unregister_ipv4_bridge_table_aging_handler(self, arg):
        '''
         xpsMulticastUnregisterIpv4BridgeTableAgingHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastUnregisterIpv4BridgeTableAgingHandler(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastTriggerIpv4RouteTableAging
    #/********************************************************************************/
    def do_multicast_trigger_ipv4_route_table_aging(self, arg):
        '''
         xpsMulticastTriggerIpv4RouteTableAging: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastTriggerIpv4RouteTableAging(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastConfigureIpv4RouteTableAging
    #/********************************************************************************/
    def do_multicast_configure_ipv4_route_table_aging(self, arg):
        '''
         xpsMulticastConfigureIpv4RouteTableAging: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsMulticastConfigureIpv4RouteTableAging(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastConfigureIpv4RouteEntryAging
    #/********************************************************************************/
    def do_multicast_configure_ipv4_route_entry_aging(self, arg):
        '''
         xpsMulticastConfigureIpv4RouteEntryAging: Enter [ devId,enable,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, enable=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastConfigureIpv4RouteEntryAging(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv4RouteTableAgingStatus
    #/********************************************************************************/
    def do_multicast_get_ipv4_route_table_aging_status(self, arg):
        '''
         xpsMulticastGetIpv4RouteTableAgingStatus: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_boolp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv4RouteTableAgingStatus(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (boolp_value(enable_Ptr_1)))
                pass
            delete_boolp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastGetIpv4RouteEntryAgingStatus
    #/********************************************************************************/
    def do_multicast_get_ipv4_route_entry_aging_status(self, arg):
        '''
         xpsMulticastGetIpv4RouteEntryAgingStatus: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_boolp()
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastGetIpv4RouteEntryAgingStatus(args[0],enable_Ptr_1,args[1])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (boolp_value(enable_Ptr_1)))
                pass
            delete_boolp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastSetIpv4RouteTableAgingCycle
    #/********************************************************************************/
    def do_multicast_set_ipv4_route_table_aging_cycle(self, arg):
        '''
         xpsMulticastSetIpv4RouteTableAgingCycle: Enter [ devId,sec ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sec ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, sec=%d' % (args[0],args[1]))
            ret = xpsMulticastSetIpv4RouteTableAgingCycle(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv4RouteTableAgingCycle
    #/********************************************************************************/
    def do_multicast_get_ipv4_route_table_aging_cycle(self, arg):
        '''
         xpsMulticastGetIpv4RouteTableAgingCycle: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            sec_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv4RouteTableAgingCycle(args[0],sec_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('sec = %d' % (uint32_tp_value(sec_Ptr_1)))
                pass
            delete_uint32_tp(sec_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastRegisterIpv4RouteTableAgingHandler
    #/********************************************************************************/
    def do_multicast_register_ipv4_route_table_aging_handler(self, arg):
        '''
         xpsMulticastRegisterIpv4RouteTableAgingHandler: Enter [ devId,func ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,func ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            userData_Ptr_2 = new_voidp()
            #print('Input Arguments are, devId=%d, func=%d' % (args[0],args[1]))
            ret = xpsMulticastRegisterIpv4RouteTableAgingHandler(args[0],args[1],userData_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('userData = %d' % (voidp_value(userData_Ptr_2)))
                pass
            delete_voidp(userData_Ptr_2)
    #/********************************************************************************/
    # command for xpsMulticastRegisterIpv4RouteTableDefaultAgingHandler
    #/********************************************************************************/
    def do_multicast_register_ipv4_route_table_default_aging_handler(self, arg):
        '''
         xpsMulticastRegisterIpv4RouteTableDefaultAgingHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastRegisterIpv4RouteTableDefaultAgingHandler(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastUnregisterIpv4RouteTableAgingHandler
    #/********************************************************************************/
    def do_multicast_unregister_ipv4_route_table_aging_handler(self, arg):
        '''
         xpsMulticastUnregisterIpv4RouteTableAgingHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastUnregisterIpv4RouteTableAgingHandler(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastTriggerIpv6BridgeTableAging
    #/********************************************************************************/
    def do_multicast_trigger_ipv6_bridge_table_aging(self, arg):
        '''
         xpsMulticastTriggerIpv6BridgeTableAging: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastTriggerIpv6BridgeTableAging(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastConfigureIpv6BridgeTableAging
    #/********************************************************************************/
    def do_multicast_configure_ipv6_bridge_table_aging(self, arg):
        '''
         xpsMulticastConfigureIpv6BridgeTableAging: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsMulticastConfigureIpv6BridgeTableAging(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastConfigureIpv6BridgeEntryAging
    #/********************************************************************************/
    def do_multicast_configure_ipv6_bridge_entry_aging(self, arg):
        '''
         xpsMulticastConfigureIpv6BridgeEntryAging: Enter [ devId,enable,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, enable=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastConfigureIpv6BridgeEntryAging(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6BridgeTableAgingStatus
    #/********************************************************************************/
    def do_multicast_get_ipv6_bridge_table_aging_status(self, arg):
        '''
         xpsMulticastGetIpv6BridgeTableAgingStatus: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_boolp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv6BridgeTableAgingStatus(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (boolp_value(enable_Ptr_1)))
                pass
            delete_boolp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6BridgeEntryAgingStatus
    #/********************************************************************************/
    def do_multicast_get_ipv6_bridge_entry_aging_status(self, arg):
        '''
         xpsMulticastGetIpv6BridgeEntryAgingStatus: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_boolp()
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastGetIpv6BridgeEntryAgingStatus(args[0],enable_Ptr_1,args[1])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (boolp_value(enable_Ptr_1)))
                pass
            delete_boolp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastSetIpv6BridgeTableAgingCycle
    #/********************************************************************************/
    def do_multicast_set_ipv6_bridge_table_aging_cycle(self, arg):
        '''
         xpsMulticastSetIpv6BridgeTableAgingCycle: Enter [ devId,sec ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sec ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, sec=%d' % (args[0],args[1]))
            ret = xpsMulticastSetIpv6BridgeTableAgingCycle(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6BridgeTableAgingCycle
    #/********************************************************************************/
    def do_multicast_get_ipv6_bridge_table_aging_cycle(self, arg):
        '''
         xpsMulticastGetIpv6BridgeTableAgingCycle: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            sec_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv6BridgeTableAgingCycle(args[0],sec_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('sec = %d' % (uint32_tp_value(sec_Ptr_1)))
                pass
            delete_uint32_tp(sec_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastRegisterIpv6BridgeTableAgingHandler
    #/********************************************************************************/
    def do_multicast_register_ipv6_bridge_table_aging_handler(self, arg):
        '''
         xpsMulticastRegisterIpv6BridgeTableAgingHandler: Enter [ devId,func ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,func ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            userData_Ptr_2 = new_voidp()
            #print('Input Arguments are, devId=%d, func=%d' % (args[0],args[1]))
            ret = xpsMulticastRegisterIpv6BridgeTableAgingHandler(args[0],args[1],userData_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('userData = %d' % (voidp_value(userData_Ptr_2)))
                pass
            delete_voidp(userData_Ptr_2)
    #/********************************************************************************/
    # command for xpsMulticastRegisterIpv6BridgeTableDefaultAgingHandler
    #/********************************************************************************/
    def do_multicast_register_ipv6_bridge_table_default_aging_handler(self, arg):
        '''
         xpsMulticastRegisterIpv6BridgeTableDefaultAgingHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastRegisterIpv6BridgeTableDefaultAgingHandler(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastUnregisterIpv6BridgeTableAgingHandler
    #/********************************************************************************/
    def do_multicast_unregister_ipv6_bridge_table_aging_handler(self, arg):
        '''
         xpsMulticastUnregisterIpv6BridgeTableAgingHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastUnregisterIpv6BridgeTableAgingHandler(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastTriggerIpv6RouteTableAging
    #/********************************************************************************/
    def do_multicast_trigger_ipv6_route_table_aging(self, arg):
        '''
         xpsMulticastTriggerIpv6RouteTableAging: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastTriggerIpv6RouteTableAging(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastConfigureIpv6RouteTableAging
    #/********************************************************************************/
    def do_multicast_configure_ipv6_route_table_aging(self, arg):
        '''
         xpsMulticastConfigureIpv6RouteTableAging: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsMulticastConfigureIpv6RouteTableAging(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastConfigureIpv6RouteEntryAging
    #/********************************************************************************/
    def do_multicast_configure_ipv6_route_entry_aging(self, arg):
        '''
         xpsMulticastConfigureIpv6RouteEntryAging: Enter [ devId,enable,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, enable=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastConfigureIpv6RouteEntryAging(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6RouteTableAgingStatus
    #/********************************************************************************/
    def do_multicast_get_ipv6_route_table_aging_status(self, arg):
        '''
         xpsMulticastGetIpv6RouteTableAgingStatus: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_boolp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv6RouteTableAgingStatus(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (boolp_value(enable_Ptr_1)))
                pass
            delete_boolp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6RouteEntryAgingStatus
    #/********************************************************************************/
    def do_multicast_get_ipv6_route_entry_aging_status(self, arg):
        '''
         xpsMulticastGetIpv6RouteEntryAgingStatus: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_boolp()
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastGetIpv6RouteEntryAgingStatus(args[0],enable_Ptr_1,args[1])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (boolp_value(enable_Ptr_1)))
                pass
            delete_boolp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastSetIpv6RouteTableAgingCycle
    #/********************************************************************************/
    def do_multicast_set_ipv6_route_table_aging_cycle(self, arg):
        '''
         xpsMulticastSetIpv6RouteTableAgingCycle: Enter [ devId,sec ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sec ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, sec=%d' % (args[0],args[1]))
            ret = xpsMulticastSetIpv6RouteTableAgingCycle(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6RouteTableAgingCycle
    #/********************************************************************************/
    def do_multicast_get_ipv6_route_table_aging_cycle(self, arg):
        '''
         xpsMulticastGetIpv6RouteTableAgingCycle: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            sec_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv6RouteTableAgingCycle(args[0],sec_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('sec = %d' % (uint32_tp_value(sec_Ptr_1)))
                pass
            delete_uint32_tp(sec_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastRegisterIpv6RouteTableAgingHandler
    #/********************************************************************************/
    def do_multicast_register_ipv6_route_table_aging_handler(self, arg):
        '''
         xpsMulticastRegisterIpv6RouteTableAgingHandler: Enter [ devId,func ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,func ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            userData_Ptr_2 = new_voidp()
            #print('Input Arguments are, devId=%d, func=%d' % (args[0],args[1]))
            ret = xpsMulticastRegisterIpv6RouteTableAgingHandler(args[0],args[1],userData_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('userData = %d' % (voidp_value(userData_Ptr_2)))
                pass
            delete_voidp(userData_Ptr_2)
    #/********************************************************************************/
    # command for xpsMulticastRegisterIpv6RouteTableDefaultAgingHandler
    #/********************************************************************************/
    def do_multicast_register_ipv6_route_table_default_aging_handler(self, arg):
        '''
         xpsMulticastRegisterIpv6RouteTableDefaultAgingHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastRegisterIpv6RouteTableDefaultAgingHandler(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastUnregisterIpv6RouteTableAgingHandler
    #/********************************************************************************/
    def do_multicast_unregister_ipv6_route_table_aging_handler(self, arg):
        '''
         xpsMulticastUnregisterIpv6RouteTableAgingHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastUnregisterIpv6RouteTableAgingHandler(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for l2EncapType enumerations
    #/********************************************************************************/
    def complete_multicast_get_vif_from_l2_interface_list_scope(self, text, line, begidx, endidx):
        tempDict = { 3 : 'l2EncapType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsMulticastGetVifFromL2InterfaceListScope
    #/********************************************************************************/
    def do_multicast_get_vif_from_l2_interface_list_scope(self, arg):
        '''
         xpsMulticastGetVifFromL2InterfaceListScope: Enter [ scopeId,ifListId,encapType ]
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
            mcVif_Ptr_3 = new_xpVif_tp()
            #print('Input Arguments are, scopeId=%d, ifListId=%d, encapType=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastGetVifFromL2InterfaceListScope(args[0],args[1],args[2],mcVif_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcVif = %d' % (xpVif_tp_value(mcVif_Ptr_3)))
                pass
            delete_xpVif_tp(mcVif_Ptr_3)

    #/********************************************************************************/
    # Auto completion for l2EncapType enumerations
    #/********************************************************************************/
    def complete_multicast_get_vif_from_l2_interface_list(self, text, line, begidx, endidx):
        tempDict = { 2 : 'l2EncapType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsMulticastGetVifFromL2InterfaceList
    #/********************************************************************************/
    def do_multicast_get_vif_from_l2_interface_list(self, arg):
        '''
         xpsMulticastGetVifFromL2InterfaceList: Enter [ ifListId,encapType ]
         Valid values for encapType : <l2EncapType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ifListId,encapType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            mcVif_Ptr_2 = new_xpVif_tp()
            #print('Input Arguments are, ifListId=%d, encapType=%d' % (args[0],args[1]))
            ret = xpsMulticastGetVifFromL2InterfaceList(args[0],args[1],mcVif_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcVif = %d' % (xpVif_tp_value(mcVif_Ptr_2)))
                pass
            delete_xpVif_tp(mcVif_Ptr_2)

    #/********************************************************************************/
    # command for xpsMulticastGetVifFromL3InterfaceListScope
    #/********************************************************************************/
    def do_multicast_get_vif_from_l3_interface_list_scope(self, arg):
        '''
         xpsMulticastGetVifFromL3InterfaceListScope: Enter [ scopeId,ifListId,l3IntfId,encapType ]
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
            mcVif_Ptr_4 = new_xpVif_tp()
            #print('Input Arguments are, scopeId=%d, ifListId=%d, l3IntfId=%d, encapType=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsMulticastGetVifFromL3InterfaceListScope(args[0],args[1],args[2],args[3],mcVif_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcVif = %d' % (xpVif_tp_value(mcVif_Ptr_4)))
                pass
            delete_xpVif_tp(mcVif_Ptr_4)

    #/********************************************************************************/
    # command for xpsMulticastGetVifFromL3InterfaceList
    #/********************************************************************************/
    def do_multicast_get_vif_from_l3_interface_list(self, arg):
        '''
         xpsMulticastGetVifFromL3InterfaceList: Enter [ ifListId,l3IntfId,encapType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ifListId,l3IntfId,encapType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            mcVif_Ptr_3 = new_xpVif_tp()
            #print('Input Arguments are, ifListId=%d, l3IntfId=%d, encapType=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastGetVifFromL3InterfaceList(args[0],args[1],args[2],mcVif_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcVif = %d' % (xpVif_tp_value(mcVif_Ptr_3)))
                pass
            delete_xpVif_tp(mcVif_Ptr_3)
    #/********************************************************************************/
    # command for xpsMulticastTriggerIpv6BridgeTableAging
    #/********************************************************************************/
    def do_multicast_trigger_ipv6_bridge_table_aging(self, arg):
        '''
         xpsMulticastTriggerIpv6BridgeTableAging: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastTriggerIpv6BridgeTableAging(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastConfigureIpv6BridgeTableAging
    #/********************************************************************************/
    def do_multicast_configure_ipv6_bridge_table_aging(self, arg):
        '''
         xpsMulticastConfigureIpv6BridgeTableAging: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsMulticastConfigureIpv6BridgeTableAging(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastConfigureIpv6BridgeEntryAging
    #/********************************************************************************/
    def do_multicast_configure_ipv6_bridge_entry_aging(self, arg):
        '''
         xpsMulticastConfigureIpv6BridgeEntryAging: Enter [ devId,enable,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, enable=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastConfigureIpv6BridgeEntryAging(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6BridgeTableAgingStatus
    #/********************************************************************************/
    def do_multicast_get_ipv6_bridge_table_aging_status(self, arg):
        '''
         xpsMulticastGetIpv6BridgeTableAgingStatus: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_boolp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv6BridgeTableAgingStatus(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (boolp_value(enable_Ptr_1)))
                pass
            delete_boolp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6BridgeEntryAgingStatus
    #/********************************************************************************/
    def do_multicast_get_ipv6_bridge_entry_aging_status(self, arg):
        '''
         xpsMulticastGetIpv6BridgeEntryAgingStatus: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_boolp()
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastGetIpv6BridgeEntryAgingStatus(args[0],enable_Ptr_1,args[1])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (boolp_value(enable_Ptr_1)))
                pass
            delete_boolp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastSetIpv6BridgeTableAgingCycle
    #/********************************************************************************/
    def do_multicast_set_ipv6_bridge_table_aging_cycle(self, arg):
        '''
         xpsMulticastSetIpv6BridgeTableAgingCycle: Enter [ devId,sec ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sec ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, sec=%d' % (args[0],args[1]))
            ret = xpsMulticastSetIpv6BridgeTableAgingCycle(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6BridgeTableAgingCycle
    #/********************************************************************************/
    def do_multicast_get_ipv6_bridge_table_aging_cycle(self, arg):
        '''
         xpsMulticastGetIpv6BridgeTableAgingCycle: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            sec_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv6BridgeTableAgingCycle(args[0],sec_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('sec = %d' % (uint32_tp_value(sec_Ptr_1)))
                pass
            delete_uint32_tp(sec_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastRegisterIpv6BridgeTableAgingHandler
    #/********************************************************************************/
    def do_multicast_register_ipv6_bridge_table_aging_handler(self, arg):
        '''
         xpsMulticastRegisterIpv6BridgeTableAgingHandler: Enter [ devId,func ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,func ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            userData_Ptr_2 = new_voidp()
            #print('Input Arguments are, devId=%d, func=%d' % (args[0],args[1]))
            ret = xpsMulticastRegisterIpv6BridgeTableAgingHandler(args[0],args[1],userData_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('userData = %d' % (voidp_value(userData_Ptr_2)))
                pass
            delete_voidp(userData_Ptr_2)
    #/********************************************************************************/
    # command for xpsMulticastUnregisterIpv6BridgeTableAgingHandler
    #/********************************************************************************/
    def do_multicast_unregister_ipv6_bridge_table_aging_handler(self, arg):
        '''
         xpsMulticastUnregisterIpv6BridgeTableAgingHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastUnregisterIpv6BridgeTableAgingHandler(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastTriggerIpv6RouteTableAging
    #/********************************************************************************/
    def do_multicast_trigger_ipv6_route_table_aging(self, arg):
        '''
         xpsMulticastTriggerIpv6RouteTableAging: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastTriggerIpv6RouteTableAging(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastConfigureIpv6RouteTableAging
    #/********************************************************************************/
    def do_multicast_configure_ipv6_route_table_aging(self, arg):
        '''
         xpsMulticastConfigureIpv6RouteTableAging: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsMulticastConfigureIpv6RouteTableAging(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastConfigureIpv6RouteEntryAging
    #/********************************************************************************/
    def do_multicast_configure_ipv6_route_entry_aging(self, arg):
        '''
         xpsMulticastConfigureIpv6RouteEntryAging: Enter [ devId,enable,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, enable=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsMulticastConfigureIpv6RouteEntryAging(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6RouteTableAgingStatus
    #/********************************************************************************/
    def do_multicast_get_ipv6_route_table_aging_status(self, arg):
        '''
         xpsMulticastGetIpv6RouteTableAgingStatus: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_boolp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv6RouteTableAgingStatus(args[0],enable_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (boolp_value(enable_Ptr_1)))
                pass
            delete_boolp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6RouteEntryAgingStatus
    #/********************************************************************************/
    def do_multicast_get_ipv6_route_entry_aging_status(self, arg):
        '''
         xpsMulticastGetIpv6RouteEntryAgingStatus: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            enable_Ptr_1 = new_boolp()
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastGetIpv6RouteEntryAgingStatus(args[0],enable_Ptr_1,args[1])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (boolp_value(enable_Ptr_1)))
                pass
            delete_boolp(enable_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastSetIpv6RouteTableAgingCycle
    #/********************************************************************************/
    def do_multicast_set_ipv6_route_table_aging_cycle(self, arg):
        '''
         xpsMulticastSetIpv6RouteTableAgingCycle: Enter [ devId,sec ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,sec ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, sec=%d' % (args[0],args[1]))
            ret = xpsMulticastSetIpv6RouteTableAgingCycle(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6RouteTableAgingCycle
    #/********************************************************************************/
    def do_multicast_get_ipv6_route_table_aging_cycle(self, arg):
        '''
         xpsMulticastGetIpv6RouteTableAgingCycle: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            sec_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv6RouteTableAgingCycle(args[0],sec_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('sec = %d' % (uint32_tp_value(sec_Ptr_1)))
                pass
            delete_uint32_tp(sec_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastRegisterIpv6RouteTableAgingHandler
    #/********************************************************************************/
    def do_multicast_register_ipv6_route_table_aging_handler(self, arg):
        '''
         xpsMulticastRegisterIpv6RouteTableAgingHandler: Enter [ devId,func ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,func ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            userData_Ptr_2 = new_voidp()
            #print('Input Arguments are, devId=%d, func=%d' % (args[0],args[1]))
            ret = xpsMulticastRegisterIpv6RouteTableAgingHandler(args[0],args[1],userData_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('userData = %d' % (voidp_value(userData_Ptr_2)))
                pass
            delete_voidp(userData_Ptr_2)
    #/********************************************************************************/
    # command for xpsMulticastUnregisterIpv6RouteTableAgingHandler
    #/********************************************************************************/
    def do_multicast_unregister_ipv6_route_table_aging_handler(self, arg):
        '''
         xpsMulticastUnregisterIpv6RouteTableAgingHandler: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastUnregisterIpv6RouteTableAgingHandler(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMulticastAllocateIpv6L3DomainIdScope
    #/********************************************************************************/
    def do_multicast_allocate_ipv6_l3_domain_id_scope(self, arg):
        '''
         xpsMulticastAllocateIpv6L3DomainIdScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            mcL3DomainId_Ptr_1 = new_xpsMcastDomainId_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsMulticastAllocateIpv6L3DomainIdScope(args[0],mcL3DomainId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcL3DomainId = %d' % (xpsMcastDomainId_tp_value(mcL3DomainId_Ptr_1)))
                pass
            delete_xpsMcastDomainId_tp(mcL3DomainId_Ptr_1)

    #/********************************************************************************/
    # command for xpsMulticastAllocateIpv6L3DomainId
    #/********************************************************************************/
    def do_multicast_allocate_ipv6_l3_domain_id(self, arg):
        '''
         xpsMulticastAllocateIpv6L3DomainId: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            mcL3DomainId_Ptr_0 = new_xpsMcastDomainId_tp()
            #print('Input Arguments are' % ())
            ret = xpsMulticastAllocateIpv6L3DomainId(mcL3DomainId_Ptr_0)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcL3DomainId = %d' % (xpsMcastDomainId_tp_value(mcL3DomainId_Ptr_0)))
                pass
            delete_xpsMcastDomainId_tp(mcL3DomainId_Ptr_0)

    #/********************************************************************************/
    # command for xpsMulticastReleaseIpv6L3DomainIdScope
    #/********************************************************************************/
    def do_multicast_release_ipv6_l3_domain_id_scope(self, arg):
        '''
         xpsMulticastReleaseIpv6L3DomainIdScope: Enter [ scopeId,mcL3DomainId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,mcL3DomainId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, mcL3DomainId=%d' % (args[0],args[1]))
            ret = xpsMulticastReleaseIpv6L3DomainIdScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMulticastReleaseIpv6L3DomainId
    #/********************************************************************************/
    def do_multicast_release_ipv6_l3_domain_id(self, arg):
        '''
         xpsMulticastReleaseIpv6L3DomainId: Enter [ mcL3DomainId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ mcL3DomainId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, mcL3DomainId=%d' % (args[0]))
            ret = xpsMulticastReleaseIpv6L3DomainId(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMcGetCreateMcL3DomainId
    #/********************************************************************************/
    def do_mc_get_create_mc_l3_domain_id(self, arg):
        '''
         xpsMcGetCreateMcL3DomainId: Enter [ devId,vrfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            mcL3DomainId_Ptr_2 = new_xpMcastDomainId_tp()
            #print('Input Arguments are, devId=%d, vrfId=%d' % (args[0],args[1]))
            ret = xpsMcGetCreateMcL3DomainId(args[0],args[1],mcL3DomainId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcL3DomainId = %d' % (xpMcastDomainId_tp_value(mcL3DomainId_Ptr_2)))
                xpShellGlobals.cmdRetVal = ((xpMcastDomainId_tp_value(mcL3DomainId_Ptr_2)))
                pass
            delete_xpMcastDomainId_tp(mcL3DomainId_Ptr_2)
    #/********************************************************************************/
    # command for xpsMcReleaseDestroyL3DomainId
    #/********************************************************************************/
    def do_mc_release_destroy_l3_domain_id(self, arg):
        '''
         xpsMcReleaseDestroyL3DomainId: Enter [ devId,vrfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vrfId=%d' % (args[0],args[1]))
            ret = xpsMcReleaseDestroyL3DomainId(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMcDestroyL3DomainIdScope
    #/********************************************************************************/
    def do_mc_destroy_l3_domain_id_scope(self, arg):
        '''
         xpsMcDestroyL3DomainIdScope: Enter [ scopeId,vrfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vrfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, vrfId=%d' % (args[0],args[1]))
            ret = xpsMcDestroyL3DomainIdScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsMcDestroyL3DomainId
    #/********************************************************************************/
    def do_mc_destroy_l3_domain_id(self, arg):
        '''
         xpsMcDestroyL3DomainId: Enter [ vrfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ vrfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, vrfId=%d' % (args[0]))
            ret = xpsMcDestroyL3DomainId(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMcGetCreateMcL2DomainId
    #/********************************************************************************/
    def do_mc_get_create_mc_l2_domain_id(self, arg):
        '''
         xpsMcGetCreateMcL2DomainId: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            mcL2DomainId_Ptr_2 = new_xpMcastDomainId_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsMcGetCreateMcL2DomainId(args[0],args[1],mcL2DomainId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcL2DomainId = %d' % (xpMcastDomainId_tp_value(mcL2DomainId_Ptr_2)))
                pass
            delete_xpMcastDomainId_tp(mcL2DomainId_Ptr_2)
    #/********************************************************************************/
    # command for xpsMcReleaseDestroyL2DomainId
    #/********************************************************************************/
    def do_mc_release_destroy_l2_domain_id(self, arg):
        '''
         xpsMcReleaseDestroyL2DomainId: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsMcReleaseDestroyL2DomainId(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastAddIpv4RouteControlEntry
    #/********************************************************************************/
    def do_multicast_add_ipv4_route_control_entry(self, arg):
        '''
         xpsMulticastAddIpv4RouteControlEntry: Enter [ devId,vrfId,groupAddress,reasonCode,pktCmd ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,groupAddress,reasonCode,pktCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = args[2].replace(".", ":").replace(",", ":")
            groupAddressList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(groupAddressList)
            groupAddress = ipv4Addr_t()
            for ix in range(listLen-1, -1, -1):
                groupAddress[listLen - ix - 1] = int(groupAddressList[ix])
            args[3] = int(args[3])
            args[4] = eval(args[4])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, vrfId=%d, groupAddress=%s, reasonCode=%d, pktCmd=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsMulticastAddIpv4RouteControlEntry(args[0],args[1],groupAddress,args[3],args[4],xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastGetIpv4RouteControlEntryReasonCode
    #/********************************************************************************/
    def do_multicast_get_ipv4_route_control_entry_reason_code(self, arg):
        '''
         xpsMulticastGetIpv4RouteControlEntryReasonCode: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            reasonCode_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastGetIpv4RouteControlEntryReasonCode(args[0],args[1],reasonCode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('reasonCode = %d' % (uint32_tp_value(reasonCode_Ptr_2)))
                pass
            delete_uint32_tp(reasonCode_Ptr_2)
    #/********************************************************************************/
    # command for xpsMulticastAddIpv6RouteControlEntry
    #/********************************************************************************/
    def do_multicast_add_ipv6_route_control_entry(self, arg):
        '''
         xpsMulticastAddIpv6RouteControlEntry: Enter [ devId,ipv6McL3DomainId,groupAddress,reasonCode,pktCmd ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,ipv6McL3DomainId,groupAddress,reasonCode,pktCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = args[2].replace(".", ":").replace(",", ":")
            groupAddressList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(groupAddressList)
            groupAddress = ipv6Addr_t()
            for ix in range(listLen-1, -1, -1):
                groupAddress[listLen - ix - 1] = int(groupAddressList[ix], 16)
            args[3] = int(args[3])
            args[4] = eval(args[4])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, ipv6McL3DomainId=%d, groupAddress=%s, reasonCode=%d, pktCmd=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsMulticastAddIpv6RouteControlEntry(args[0],args[1],groupAddress,args[3],args[4],xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6RouteControlEntryReasonCode
    #/********************************************************************************/
    def do_multicast_get_ipv6_route_control_entry_reason_code(self, arg):
        '''
         xpsMulticastGetIpv6RouteControlEntryReasonCode: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            reasonCode_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsMulticastGetIpv6RouteControlEntryReasonCode(args[0],args[1],reasonCode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('reasonCode = %d' % (uint32_tp_value(reasonCode_Ptr_2)))
                pass
            delete_uint32_tp(reasonCode_Ptr_2)
    #/********************************************************************************/
    # command for xpsMulticastSetIpv4BridgeRehashLevel
    #/********************************************************************************/
    def do_multicast_set_ipv4_bridge_rehash_level(self, arg):
        '''
         xpsMulticastSetIpv4BridgeRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsMulticastSetIpv4BridgeRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv4BridgeRehashLevel
    #/********************************************************************************/
    def do_multicast_get_ipv4_bridge_rehash_level(self, arg):
        '''
         xpsMulticastGetIpv4BridgeRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv4BridgeRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastSetIpv6BridgeRehashLevel
    #/********************************************************************************/
    def do_multicast_set_ipv6_bridge_rehash_level(self, arg):
        '''
         xpsMulticastSetIpv6BridgeRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsMulticastSetIpv6BridgeRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6BridgeRehashLevel
    #/********************************************************************************/
    def do_multicast_get_ipv6_bridge_rehash_level(self, arg):
        '''
         xpsMulticastGetIpv6BridgeRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv6BridgeRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastSetIpv4RouteRehashLevel
    #/********************************************************************************/
    def do_multicast_set_ipv4_route_rehash_level(self, arg):
        '''
         xpsMulticastSetIpv4RouteRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsMulticastSetIpv4RouteRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv4RouteRehashLevel
    #/********************************************************************************/
    def do_multicast_get_ipv4_route_rehash_level(self, arg):
        '''
         xpsMulticastGetIpv4RouteRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv4RouteRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastSetIpv6RouteRehashLevel
    #/********************************************************************************/
    def do_multicast_set_ipv6_route_rehash_level(self, arg):
        '''
         xpsMulticastSetIpv6RouteRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsMulticastSetIpv6RouteRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6RouteRehashLevel
    #/********************************************************************************/
    def do_multicast_get_ipv6_route_rehash_level(self, arg):
        '''
         xpsMulticastGetIpv6RouteRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv6RouteRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastSetIpv4PIMBiDirRPFRehashLevel
    #/********************************************************************************/
    def do_multicast_set_ipv4_pim_bi_dir_rpf_rehash_level(self, arg):
        '''
         xpsMulticastSetIpv4PIMBiDirRPFRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsMulticastSetIpv4PIMBiDirRPFRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv4PIMBiDirRPFRehashLevel
    #/********************************************************************************/
    def do_multicast_get_ipv4_pim_bi_dir_rpf_rehash_level(self, arg):
        '''
         xpsMulticastGetIpv4PIMBiDirRPFRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv4PIMBiDirRPFRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)
    #/********************************************************************************/
    # command for xpsMulticastSetIpv6PIMBiDirRPFRehashLevel
    #/********************************************************************************/
    def do_multicast_set_ipv6_pim_bi_dir_rpf_rehash_level(self, arg):
        '''
         xpsMulticastSetIpv6PIMBiDirRPFRehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsMulticastSetIpv6PIMBiDirRPFRehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMulticastGetIpv6PIMBiDirRPFRehashLevel
    #/********************************************************************************/
    def do_multicast_get_ipv6_pim_bi_dir_rpf_rehash_level(self, arg):
        '''
         xpsMulticastGetIpv6PIMBiDirRPFRehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsMulticastGetIpv6PIMBiDirRPFRehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)
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
