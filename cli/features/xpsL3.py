#!/usr/bin/env python
#  xpsL3.py
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
from xpsUtils import *

import xpShellGlobals
import enumDict

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *


#/**********************************************************************************/
# The class object for xpsL3 operations
#/**********************************************************************************/

class xpsL3Obj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsL3CreateSubIntf
    #/********************************************************************************/
    def do_l3_create_sub_intf(self, arg):
        '''
         xpsL3CreateSubIntf: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            l3IntfId_Ptr_0 = new_xpsInterfaceId_tp()
            #print('Input Arguments are' % ())
            ret = xpsL3CreateSubIntf(l3IntfId_Ptr_0)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3IntfId = %d' % (xpsInterfaceId_tp_value(l3IntfId_Ptr_0)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(l3IntfId_Ptr_0))
                pass
            delete_xpsInterfaceId_tp(l3IntfId_Ptr_0)
    #/********************************************************************************/
    # command for xpsL3CreateSubIntfScope
    #/********************************************************************************/
    def do_l3_create_sub_intf_scope(self, arg):
        '''
         xpsL3CreateSubIntfScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            l3IntfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3CreateSubIntfScope(args[0],l3IntfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3IntfId = %d' % (xpsInterfaceId_tp_value(l3IntfId_Ptr_1)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(l3IntfId_Ptr_1))
                pass
            delete_xpsInterfaceId_tp(l3IntfId_Ptr_1)
    #/********************************************************************************/
    # command for xpsL3DestroySubIntf
    #/********************************************************************************/
    def do_l3_destroy_sub_intf(self, arg):
        '''
         xpsL3DestroySubIntf: Enter [ l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ l3IntfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, l3IntfId=%d' % (args[0]))
            ret = xpsL3DestroySubIntf(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DestroySubIntfScope
    #/********************************************************************************/
    def do_l3_destroy_sub_intf_scope(self, arg):
        '''
         xpsL3DestroySubIntfScope: Enter [ scopeId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3DestroySubIntfScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3BindSubIntf
    #/********************************************************************************/
    def do_l3_bind_sub_intf(self, arg):
        '''
         xpsL3BindSubIntf: Enter [ portIntfId,l3IntfId,encapId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIntfId,l3IntfId,encapId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            pVlanIvifIdx_Ptr_3 = new_uint32_tp()
            reHashIndex_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, portIntfId=%d, l3IntfId=%d, encapId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3BindSubIntf(args[0],args[1],args[2],pVlanIvifIdx_Ptr_3,reHashIndex_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pVlanIvifIdx = %d' % (uint32_tp_value(pVlanIvifIdx_Ptr_3)))
                print('reHashIndex = %d' % (uint32_tp_value(reHashIndex_Ptr_4)))
                pass
            delete_uint32_tp(reHashIndex_Ptr_4)
            delete_uint32_tp(pVlanIvifIdx_Ptr_3)
    #/********************************************************************************/
    # command for xpsL3BindSubIntfScope
    #/********************************************************************************/
    def do_l3_bind_sub_intf_scope(self, arg):
        '''
         xpsL3BindSubIntfScope: Enter [ scopeId,portIntfId,l3IntfId,encapId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIntfId,l3IntfId,encapId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            pVlanIvifIdx_Ptr_4 = new_uint32_tp()
            reHashIndex_Ptr_5 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, portIntfId=%d, l3IntfId=%d, encapId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsL3BindSubIntfScope(args[0],args[1],args[2],args[3],pVlanIvifIdx_Ptr_4,reHashIndex_Ptr_5)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pVlanIvifIdx = %d' % (uint32_tp_value(pVlanIvifIdx_Ptr_4)))
                print('reHashIndex = %d' % (uint32_tp_value(reHashIndex_Ptr_5)))
                pass
            delete_uint32_tp(reHashIndex_Ptr_5)
            delete_uint32_tp(pVlanIvifIdx_Ptr_4)
    #/********************************************************************************/
    # command for xpsL3UnBindSubIntf
    #/********************************************************************************/
    def do_l3_un_bind_sub_intf(self, arg):
        '''
         xpsL3UnBindSubIntf: Enter [ portIntfId,encapId,pVlanIvifIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIntfId,encapId,pVlanIvifIdx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, portIntfId=%d, encapId=%d, pVlanIvifIdx=%d' % (args[0],args[1],args[2]))
            ret = xpsL3UnBindSubIntf(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3UnBindSubIntfScope
    #/********************************************************************************/
    def do_l3_un_bind_sub_intf_scope(self, arg):
        '''
         xpsL3UnBindSubIntfScope: Enter [ scopeId,portIntfId,encapId,pVlanIvifIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIntfId,encapId,pVlanIvifIdx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, scopeId=%d, portIntfId=%d, encapId=%d, pVlanIvifIdx=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsL3UnBindSubIntfScope(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3InitSubIntf
    #/********************************************************************************/
    def do_l3_init_sub_intf(self, arg):
        '''
         xpsL3InitSubIntf: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3InitSubIntf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DeInitSubIntf
    #/********************************************************************************/
    def do_l3_de_init_sub_intf(self, arg):
        '''
         xpsL3DeInitSubIntf: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3DeInitSubIntf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3CreatePortIntf
    #/********************************************************************************/
    def do_l3_create_port_intf(self, arg):
        '''
         xpsL3CreatePortIntf: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            l3IntfId_Ptr_0 = new_xpsInterfaceId_tp()
            #print('Input Arguments are' % ())
            ret = xpsL3CreatePortIntf(l3IntfId_Ptr_0)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3IntfId = %d' % (xpsInterfaceId_tp_value(l3IntfId_Ptr_0)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(l3IntfId_Ptr_0)))
                pass
            delete_xpsInterfaceId_tp(l3IntfId_Ptr_0)

    #/********************************************************************************/
    # command for xpsL3CreatePortIntfScope
    #/********************************************************************************/
    def do_l3_create_port_intf_scope(self, arg):
        '''
         xpsL3CreatePortIntfScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            l3IntfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3CreatePortIntfScope(args[0],l3IntfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3IntfId = %d' % (xpsInterfaceId_tp_value(l3IntfId_Ptr_1)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(l3IntfId_Ptr_1))
                pass
            delete_xpsInterfaceId_tp(l3IntfId_Ptr_1)

    #/********************************************************************************/
    # command for xpsL3DestroyPortIntf
    #/********************************************************************************/
    def do_l3_destroy_port_intf(self, arg):
        '''
         xpsL3DestroyPortIntf: Enter [ l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ l3IntfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, l3IntfId=%d' % (args[0]))
            ret = xpsL3DestroyPortIntf(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsL3DestroyPortIntfScope
    #/********************************************************************************/
    def do_l3_destroy_port_intf_scope(self, arg):
        '''
         xpsL3DestroyPortIntfScope: Enter [ scopeId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3DestroyPortIntfScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsL3InitPortIntf
    #/********************************************************************************/
    def do_l3_init_port_intf(self, arg):
        '''
         xpsL3InitPortIntf: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3InitPortIntf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DeInitPortIntf
    #/********************************************************************************/
    def do_l3_de_init_port_intf(self, arg):
        '''
         xpsL3DeInitPortIntf: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3DeInitPortIntf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3BindPortIntf
    #/********************************************************************************/
    def do_l3_bind_port_intf(self, arg):
        '''
         xpsL3BindPortIntf: Enter [ portIntfId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIntfId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, portIntfId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3BindPortIntf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsL3BindPortIntfScope
    #/********************************************************************************/
    def do_l3_bind_port_intf_scope(self, arg):
        '''
         xpsL3BindPortIntfScope: Enter [ scopeId,portIntfId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIntfId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, portIntfId=%d, l3IntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3BindPortIntfScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetBindPortIntfScope
    #/********************************************************************************/
    def do_l3_get_bind_port_intf_scope(self, arg):
        '''
         xpsL3GetBindPortIntfScope: Enter [ scopeId,portIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            l3IntfId_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, portIntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetBindPortIntfScope(args[0],args[1],l3IntfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3IntfId = %d' % (xpsInterfaceId_tp_value(l3IntfId_Ptr_2)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(l3IntfId_Ptr_2))
                pass
            delete_xpsInterfaceId_tp(l3IntfId_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3GetBindPortIntf
    #/********************************************************************************/
    def do_l3_get_bind_port_intf(self, arg):
        '''
         xpsL3GetBindPortIntf: Enter [ portIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIntfId ]')
        else:
            args[0] = int(args[0])
            l3IntfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, portIntfId=%d' % (args[0]))
            ret = xpsL3GetBindPortIntf(args[0],l3IntfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3IntfId = %d' % (xpsInterfaceId_tp_value(l3IntfId_Ptr_1)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(l3IntfId_Ptr_1))
                pass
            delete_xpsInterfaceId_tp(l3IntfId_Ptr_1)
    #/********************************************************************************/
    # command for xpsL3UnBindPortIntf
    #/********************************************************************************/
    def do_l3_un_bind_port_intf(self, arg):
        '''
         xpsL3UnBindPortIntf: Enter [ portIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIntfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, portIntfId=%d' % (args[0]))
            ret = xpsL3UnBindPortIntf(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsL3UnBindPortIntfScope
    #/********************************************************************************/
    def do_l3_un_bind_port_intf_scope(self, arg):
        '''
         xpsL3UnBindPortIntfScope: Enter [ scopeId,portIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, portIntfId=%d' % (args[0],args[1]))
            ret = xpsL3UnBindPortIntfScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsL3CreateVlanIntf
    #/********************************************************************************/
    def do_l3_create_vlan_intf(self, arg):
        '''
         xpsL3CreateVlanIntf: Enter [ vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ vlanId ]')
        else:
            args[0] = int(args[0])
            l3IntfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, vlanId=%d' % (args[0]))
            ret = xpsL3CreateVlanIntf(args[0],l3IntfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3IntfId = %d' % (xpsInterfaceId_tp_value(l3IntfId_Ptr_1)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(l3IntfId_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(l3IntfId_Ptr_1)

    #/********************************************************************************/
    # command for xpsL3CreateVlanIntfScope
    #/********************************************************************************/
    def do_l3_create_vlan_intf_scope(self, arg):
        '''
         xpsL3CreateVlanIntfScope: Enter [ scopeId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            l3IntfId_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsL3CreateVlanIntfScope(args[0],args[1],l3IntfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3IntfId = %d' % (xpsInterfaceId_tp_value(l3IntfId_Ptr_2)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(l3IntfId_Ptr_2))
                pass
            delete_xpsInterfaceId_tp(l3IntfId_Ptr_2)

    #/********************************************************************************/
    # command for xpsL3DestroyVlanIntf
    #/********************************************************************************/
    def do_l3_destroy_vlan_intf(self, arg):
        '''
         xpsL3DestroyVlanIntf: Enter [ vlanId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ vlanId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, vlanId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3DestroyVlanIntf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DestroyVlanIntfScope
    #/********************************************************************************/
    def do_l3_destroy_vlan_intf_scope(self, arg):
        '''
         xpsL3DestroyVlanIntfScope: Enter [ scopeId,vlanId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vlanId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, vlanId=%d, l3IntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3DestroyVlanIntfScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3CreateTunnelIntf
    #/********************************************************************************/
    def do_l3_create_tunnel_intf(self, arg):
        '''
         xpsL3CreateTunnelIntf: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            l3IntfId_Ptr_0 = new_xpsInterfaceId_tp()
            #print('Input Arguments are' % ())
            ret = xpsL3CreateTunnelIntf(l3IntfId_Ptr_0)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3IntfId = %d' % (xpsInterfaceId_tp_value(l3IntfId_Ptr_0)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(l3IntfId_Ptr_0)))
                pass
            delete_xpsInterfaceId_tp(l3IntfId_Ptr_0)
    #/********************************************************************************/
    # command for xpsL3CreateTunnelIntfScope
    #/********************************************************************************/
    def do_l3_create_tunnel_intf_scope(self, arg):
        '''
         xpsL3CreateTunnelIntfScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            l3IntfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3CreateTunnelIntfScope(args[0],l3IntfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3IntfId = %d' % (xpsInterfaceId_tp_value(l3IntfId_Ptr_1)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(l3IntfId_Ptr_1))
                pass
            delete_xpsInterfaceId_tp(l3IntfId_Ptr_1)
    #/********************************************************************************/
    # command for xpsL3DestroyTunnelIntf
    #/********************************************************************************/
    def do_l3_destroy_tunnel_intf(self, arg):
        '''
         xpsL3DestroyTunnelIntf: Enter [ l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ l3IntfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, l3IntfId=%d' % (args[0]))
            ret = xpsL3DestroyTunnelIntf(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DestroyTunnelIntfScope
    #/********************************************************************************/
    def do_l3_destroy_tunnel_intf_scope(self, arg):
        '''
         xpsL3DestroyTunnelIntfScope: Enter [ scopeId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3DestroyTunnelIntfScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3InitTunnelIntf
    #/********************************************************************************/
    def do_l3_init_tunnel_intf(self, arg):
        '''
         xpsL3InitTunnelIntf: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3InitTunnelIntf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DeInitTunnelIntf
    #/********************************************************************************/
    def do_l3_de_init_tunnel_intf(self, arg):
        '''
         xpsL3DeInitTunnelIntf: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3DeInitTunnelIntf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3BindTunnelIntf
    #/********************************************************************************/
    def do_l3_bind_tunnel_intf(self, arg):
        '''
         xpsL3BindTunnelIntf: Enter [ devId,tnnlIntfId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnnlIntfId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, tnnlIntfId=%d, l3IntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3BindTunnelIntf(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3UnBindTunnelIntf
    #/********************************************************************************/
    def do_l3_un_bind_tunnel_intf(self, arg):
        '''
         xpsL3UnBindTunnelIntf: Enter [ devId,tnnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnnlIntfId=%d' % (args[0],args[1]))
            ret = xpsL3UnBindTunnelIntf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3CreateVpnIntf
    #/********************************************************************************/
    def do_l3_create_vpn_intf(self, arg):
        '''
         xpsL3CreateVpnIntf: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            l3IntfId_Ptr_0 = new_xpsInterfaceId_tp()
            #print('Input Arguments are' % ())
            ret = xpsL3CreateVpnIntf(l3IntfId_Ptr_0)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3IntfId = %d' % (xpsInterfaceId_tp_value(l3IntfId_Ptr_0)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(l3IntfId_Ptr_0)))
                pass
            delete_xpsInterfaceId_tp(l3IntfId_Ptr_0)
    #/********************************************************************************/
    # command for xpsL3CreateVpnIntfScope
    #/********************************************************************************/
    def do_l3_create_vpn_intf_scope(self, arg):
        '''
         xpsL3CreateVpnIntfScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            l3IntfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3CreateVpnIntfScope(args[0],l3IntfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3IntfId = %d' % (xpsInterfaceId_tp_value(l3IntfId_Ptr_1)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(l3IntfId_Ptr_1))
                pass
            delete_xpsInterfaceId_tp(l3IntfId_Ptr_1)
    #/********************************************************************************/
    # command for xpsL3DestroyVpnIntf
    #/********************************************************************************/
    def do_l3_destroy_vpn_intf(self, arg):
        '''
         xpsL3DestroyVpnIntf: Enter [ l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ l3IntfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, l3IntfId=%d' % (args[0]))
            ret = xpsL3DestroyVpnIntf(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DestroyVpnIntfScope
    #/********************************************************************************/
    def do_l3_destroy_vpn_intf_scope(self, arg):
        '''
         xpsL3DestroyVpnIntfScope: Enter [ scopeId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3DestroyVpnIntfScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3InitVpnIntf
    #/********************************************************************************/
    def do_l3_init_vpn_intf(self, arg):
        '''
         xpsL3InitVpnIntf: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3InitVpnIntf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DeInitVpnIntf
    #/********************************************************************************/
    def do_l3_de_init_vpn_intf(self, arg):
        '''
         xpsL3DeInitVpnIntf: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3DeInitVpnIntf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3BindVpnIntfToLabel
    #/********************************************************************************/
    def do_l3_bind_vpn_intf_to_label(self, arg):
        '''
         xpsL3BindVpnIntfToLabel: Enter [ devId,vpnLabel,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vpnLabel,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vpnLabel=%d, l3IntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3BindVpnIntfToLabel(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3UnBindVpnIntfFromLabel
    #/********************************************************************************/
    def do_l3_un_bind_vpn_intf_from_label(self, arg):
        '''
         xpsL3UnBindVpnIntfFromLabel: Enter [ devId,vpnLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vpnLabel ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vpnLabel=%d' % (args[0],args[1]))
            ret = xpsL3UnBindVpnIntfFromLabel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3SetVpnIntfVrf
    #/********************************************************************************/
    def do_l3_set_vpn_intf_vrf(self, arg):
        '''
         xpsL3SetVpnIntfVrf: Enter [ devId,vpnIntfId,vrfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vpnIntfId,vrfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, vpnIntfId=%d, vrfId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetVpnIntfVrf(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetVpnIntfVrf
    #/********************************************************************************/
    def do_l3_get_vpn_intf_vrf(self, arg):
        '''
         xpsL3GetVpnIntfVrf: Enter [ devId,vpnIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vpnIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            vrfId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vpnIntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetVpnIntfVrf(args[0],args[1],vrfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (uint32_tp_value(vrfId_Ptr_2)))
                pass
            delete_uint32_tp(vrfId_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3AddIngressRouterMac
    #/********************************************************************************/
    def do_l3_add_ingress_router_mac(self, arg):
        '''
         xpsL3AddIngressRouterMac: Enter [ devId,mac ]
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
            for ix in range(0, listLen, 1):
                mac[ix] = int(macList[ix], 16)
            #print('Input Arguments are, devId=%d, mac=%s' % (args[0],args[1]))
            ret = xpsL3AddIngressRouterMac(args[0],mac)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3RemoveIngressRouterMac
    #/********************************************************************************/
    def do_l3_remove_ingress_router_mac(self, arg):
        '''
         xpsL3RemoveIngressRouterMac: Enter [ devId,mac ]
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
            for ix in range(0, listLen, 1):
                mac[ix] = int(macList[ix], 16)
            #print('Input Arguments are, devId=%d, mac=%s' % (args[0],args[1]))
            ret = xpsL3RemoveIngressRouterMac(args[0],mac)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3AddIntfIngressRouterMac
    #/********************************************************************************/
    def do_l3_add_intf_ingress_router_mac(self, arg):
        '''
         xpsL3AddIntfIngressRouterMac: Enter [ devId,l3IntfId,mac ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,mac ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = args[2].replace(".", ":").replace(",", ":")
            macList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(macList)
            mac = macAddr_t()
            for ix in range(0, listLen, 1):
                mac[ix] = int(macList[ix], 16)
            #print('Input Arguments are, devId=%d, l3IntfId=%d, mac=%s' % (args[0],args[1],args[2]))
            ret = xpsL3AddIntfIngressRouterMac(args[0],args[1],mac)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3RemoveIntfIngressRouterMac
    #/********************************************************************************/
    def do_l3_remove_intf_ingress_router_mac(self, arg):
        '''
         xpsL3RemoveIntfIngressRouterMac: Enter [ devId,l3IntfId,mac ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,mac ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = args[2].replace(".", ":").replace(",", ":")
            macList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(macList)
            mac = macAddr_t()
            for ix in range(0, listLen, 1):
                mac[ix] = int(macList[ix], 16)
            #print('Input Arguments are, devId=%d, l3IntfId=%d, mac=%s' % (args[0],args[1],args[2]))
            ret = xpsL3RemoveIntfIngressRouterMac(args[0],args[1],mac)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3SetEgressRouterMac
    #/********************************************************************************/
    def do_l3_set_egress_router_mac(self, arg):
        '''
         xpsL3SetEgressRouterMac: Enter [ devId,l3IntfId,mac ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,mac ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = args[2].replace(".", ":").replace(",", ":")
            macList = args[2].strip("'").strip("]").strip("[").split(":")
            listLen = len(macList)
            mac = macAddr_t()
            for ix in range(0, listLen, 1):
                mac[ix] = int(macList[ix], 16)
            #print('Input Arguments are, devId=%d, l3IntfId=%d, mac=%s' % (args[0],args[1],args[2]))
            ret = xpsL3SetEgressRouterMac(args[0],args[1],mac)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3RemoveEgressRouterMac
    #/********************************************************************************/
    def do_l3_remove_egress_router_mac(self, arg):
        '''
         xpsL3RemoveEgressRouterMac: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])

            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3RemoveEgressRouterMac(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3SetEgressRouterMacMSbs
    #/********************************************************************************/
    def do_l3_set_egress_router_mac_m_sbs(self, arg):
        '''
         xpsL3SetEgressRouterMacMSbs: Enter [ devId,macHi ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,macHi ]')
        else:
            args[0] = int(args[0])
            args[1] = args[1].replace(".", ":").replace(",", ":")
            macHiList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(macHiList)
            macHi = macAddrHigh_t()
            for ix in range(0, listLen, 1):
                macHi[ix] = int(macHiList[ix], 16)
            #print('Input Arguments are, devId=%d, macHi=%s' % (args[0],args[1]))
            ret = xpsL3SetEgressRouterMacMSbs(args[0],macHi)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetEgressRouterMacMSbs
    #/********************************************************************************/
    def do_l3_get_egress_router_mac_m_sbs(self, arg):
        '''
         xpsL3GetEgressRouterMacMSbs: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            macHi = macAddrHigh_t()
            macHiPtr =  getUint8macAddrHighPtr(macHi)
            #print('Input Arguments are, devId=%d, macHi=%s' % (args[0],args[1]))
            ret = xpsL3GetEgressRouterMacMSbs(args[0],macHiPtr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            macHiAddr = ''
            if err == 0:
                for i in range(5):
                    macHiAddr += str(format(macAddrHigh_tp_getitem(macHiPtr, 4 - i), 'x')) + ':'
                print 'macHiAddr = ' + macHiAddr.strip(':')
            del(macHiPtr)

    #/********************************************************************************/
    # command for xpsL3SetIntfEgressRouterMacLSB
    #/********************************************************************************/
    def do_l3_set_intf_egress_router_mac_lsb(self, arg):
        '''
         xpsL3SetIntfEgressRouterMacLSB: Enter [ devId,l3IntfId,macSa ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,macSa ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, macSa=0x%x' % (args[0],args[1],args[2]))
            ret = xpsL3SetIntfEgressRouterMacLSB(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIntfEgressRouterMacLSB
    #/********************************************************************************/
    def do_l3_get_intf_egress_router_mac_lsb(self, arg):
        '''
         xpsL3GetIntfEgressRouterMacLSB: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            macSa_Ptr_2 = new_macAddrLow_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIntfEgressRouterMacLSB(args[0],args[1],macSa_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('macSa = %d' % (macAddrLow_tp_value(macSa_Ptr_2)))
                pass
            delete_macAddrLow_tp(macSa_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIntfVrf
    #/********************************************************************************/
    def do_l3_set_intf_vrf(self, arg):
        '''
         xpsL3SetIntfVrf: Enter [ devId,l3IntfId,vrfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,vrfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, vrfId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIntfVrf(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIntfVrf
    #/********************************************************************************/
    def do_l3_get_intf_vrf(self, arg):
        '''
         xpsL3GetIntfVrf: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            vrfId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIntfVrf(args[0],args[1],vrfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (uint32_tp_value(vrfId_Ptr_2)))
                pass
            delete_uint32_tp(vrfId_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIntfIpv4UcRoutingEn
    #/********************************************************************************/
    def do_l3_set_intf_ipv4_uc_routing_en(self, arg):
        '''
         xpsL3SetIntfIpv4UcRoutingEn: Enter [ devId,l3IntfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIntfIpv4UcRoutingEn(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIntfIpv4UcRoutingEn
    #/********************************************************************************/
    def do_l3_get_intf_ipv4_uc_routing_en(self, arg):
        '''
         xpsL3GetIntfIpv4UcRoutingEn: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIntfIpv4UcRoutingEn(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIntfIpv6UcRoutingEn
    #/********************************************************************************/
    def do_l3_set_intf_ipv6_uc_routing_en(self, arg):
        '''
         xpsL3SetIntfIpv6UcRoutingEn: Enter [ devId,l3IntfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIntfIpv6UcRoutingEn(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIntfIpv6UcRoutingEn
    #/********************************************************************************/
    def do_l3_get_intf_ipv6_uc_routing_en(self, arg):
        '''
         xpsL3GetIntfIpv6UcRoutingEn: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIntfIpv6UcRoutingEn(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIntfMplsRoutingEn
    #/********************************************************************************/
    def do_l3_set_intf_mpls_routing_en(self, arg):
        '''
         xpsL3SetIntfMplsRoutingEn: Enter [ devId,l3IntfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIntfMplsRoutingEn(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIntfMplsRoutingEn
    #/********************************************************************************/
    def do_l3_get_intf_mpls_routing_en(self, arg):
        '''
         xpsL3GetIntfMplsRoutingEn: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIntfMplsRoutingEn(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIntfIpv4McRoutingEn
    #/********************************************************************************/
    def do_l3_set_intf_ipv4_mc_routing_en(self, arg):
        '''
         xpsL3SetIntfIpv4McRoutingEn: Enter [ devId,l3IntfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIntfIpv4McRoutingEn(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIntfIpv4McRoutingEn
    #/********************************************************************************/
    def do_l3_get_intf_ipv4_mc_routing_en(self, arg):
        '''
         xpsL3GetIntfIpv4McRoutingEn: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIntfIpv4McRoutingEn(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIntfIpv6McRoutingEn
    #/********************************************************************************/
    def do_l3_set_intf_ipv6_mc_routing_en(self, arg):
        '''
         xpsL3SetIntfIpv6McRoutingEn: Enter [ devId,l3IntfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIntfIpv6McRoutingEn(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIntfIpv6McRoutingEn
    #/********************************************************************************/
    def do_l3_get_intf_ipv6_mc_routing_en(self, arg):
        '''
         xpsL3GetIntfIpv6McRoutingEn: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIntfIpv6McRoutingEn(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIntfIpv4McRouteMode
    #/********************************************************************************/
    def do_l3_set_intf_ipv4_mc_route_mode(self, arg):
        '''
         xpsL3SetIntfIpv4McRouteMode: Enter [ devId,l3IntfId,mode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,mode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, mode=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIntfIpv4McRouteMode(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIntfIpv4McRouteMode
    #/********************************************************************************/
    def do_l3_get_intf_ipv4_mc_route_mode(self, arg):
        '''
         xpsL3GetIntfIpv4McRouteMode: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            mode_Ptr_2 = new_xpVlanRouteMcMode_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIntfIpv4McRouteMode(args[0],args[1],mode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mode = %d' % (xpVlanRouteMcMode_tp_value(mode_Ptr_2)))
                pass
            delete_xpVlanRouteMcMode_tp(mode_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIntfIpv6McRouteMode
    #/********************************************************************************/
    def do_l3_set_intf_ipv6_mc_route_mode(self, arg):
        '''
         xpsL3SetIntfIpv6McRouteMode: Enter [ devId,l3IntfId,mode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,mode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, mode=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIntfIpv6McRouteMode(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIntfIpv6McRouteMode
    #/********************************************************************************/
    def do_l3_get_intf_ipv6_mc_route_mode(self, arg):
        '''
         xpsL3GetIntfIpv6McRouteMode: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            mode_Ptr_2 = new_xpVlanRouteMcMode_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIntfIpv6McRouteMode(args[0],args[1],mode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mode = %d' % (xpVlanRouteMcMode_tp_value(mode_Ptr_2)))
                pass
            delete_xpVlanRouteMcMode_tp(mode_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIpv4UrpfEnable
    #/********************************************************************************/
    def do_l3_set_ipv4_urpf_enable(self, arg):
        '''
         xpsL3SetIpv4UrpfEnable: Enter [ devId,l3IntfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIpv4UrpfEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIpv4UrpfEnable
    #/********************************************************************************/
    def do_l3_get_ipv4_urpf_enable(self, arg):
        '''
         xpsL3GetIpv4UrpfEnable: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIpv4UrpfEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIpv4UrpfMode
    #/********************************************************************************/
    def do_l3_set_ipv4_urpf_mode(self, arg):
        '''
         xpsL3SetIpv4UrpfMode: Enter [ devId,l3IntfId,urpfMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,urpfMode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, urpfMode=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIpv4UrpfMode(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIpv4UrpfMode
    #/********************************************************************************/
    def do_l3_get_ipv4_urpf_mode(self, arg):
        '''
         xpsL3GetIpv4UrpfMode: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            urpfMode_Ptr_2 = new_xpsUrpfMode_ep()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIpv4UrpfMode(args[0],args[1],urpfMode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('urpfMode = %d' % (xpsUrpfMode_ep_value(urpfMode_Ptr_2)))
                pass
            delete_xpsUrpfMode_ep(urpfMode_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIpv6UrpfEnable
    #/********************************************************************************/
    def do_l3_set_ipv6_urpf_enable(self, arg):
        '''
         xpsL3SetIpv6UrpfEnable: Enter [ devId,l3IntfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIpv6UrpfEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIpv6UrpfEnable
    #/********************************************************************************/
    def do_l3_get_ipv6_urpf_enable(self, arg):
        '''
         xpsL3GetIpv6UrpfEnable: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIpv6UrpfEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIpv6UrpfMode
    #/********************************************************************************/
    def do_l3_set_ipv6_urpf_mode(self, arg):
        '''
         xpsL3SetIpv6UrpfMode: Enter [ devId,l3IntfId,urpfMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,urpfMode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, urpfMode=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIpv6UrpfMode(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIpv6UrpfMode
    #/********************************************************************************/
    def do_l3_get_ipv6_urpf_mode(self, arg):
        '''
         xpsL3GetIpv6UrpfMode: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            urpfMode_Ptr_2 = new_xpsUrpfMode_ep()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIpv6UrpfMode(args[0],args[1],urpfMode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('urpfMode = %d' % (xpsUrpfMode_ep_value(urpfMode_Ptr_2)))
                pass
            delete_xpsUrpfMode_ep(urpfMode_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3GetIntfEgressVif
    #/********************************************************************************/
    def do_l3_get_intf_egress_vif(self, arg):
        '''
         xpsL3GetIntfEgressVif: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            egressVif_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIntfEgressVif(args[0],args[1],egressVif_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('egressVif = %d' % (xpsInterfaceId_tp_value(egressVif_Ptr_2)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(egressVif_Ptr_2)))
                pass
            delete_xpsInterfaceId_tp(egressVif_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIntfNatMode
    #/********************************************************************************/
    def do_l3_set_intf_nat_mode(self, arg):
        '''
         xpsL3SetIntfNatMode: Enter [ devId,l3IntfId,mode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,mode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, mode=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIntfNatMode(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIntfNatMode
    #/********************************************************************************/
    def do_l3_get_intf_nat_mode(self, arg):
        '''
         xpsL3GetIntfNatMode: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            mode_Ptr_2 = new_xpsVlanNatMode_ep()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIntfNatMode(args[0],args[1],mode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mode = %d' % (xpsVlanNatMode_ep_value(mode_Ptr_2)))
                pass
            delete_xpsVlanNatMode_ep(mode_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIntfNatScope
    #/********************************************************************************/
    def do_l3_set_intf_nat_scope(self, arg):
        '''
         xpsL3SetIntfNatScope: Enter [ devId,l3IntfId,scope ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,scope ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, scope=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIntfNatScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIntfNatScope
    #/********************************************************************************/
    def do_l3_get_intf_nat_scope(self, arg):
        '''
         xpsL3GetIntfNatScope: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            scope_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIntfNatScope(args[0],args[1],scope_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('scope = %d' % (uint32_tp_value(scope_Ptr_2)))
                pass
            delete_uint32_tp(scope_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIntfCounterEn
    #/********************************************************************************/
    def do_l3_set_intf_counter_en(self, arg):
        '''
         xpsL3SetIntfCounterEn: Enter [ devId,l3IntfId,dir,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,dir,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, dir=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsL3SetIntfCounterEn(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIntfCounterEn
    #/********************************************************************************/
    def do_l3_get_intf_counter_en(self, arg):
        '''
         xpsL3GetIntfCounterEn: Enter [ devId,l3IntfId,dir ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,dir ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, l3IntfId=%d, dir=%d' % (args[0],args[1],args[2]))
            ret = xpsL3GetIntfCounterEn(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
    #/********************************************************************************/
    # command for xpsL3SetIntfEgressCounterId
    #/********************************************************************************/
    def do_l3_set_intf_egress_counter_id(self, arg):
        '''
         xpsL3SetIntfEgressCounterId: Enter [ devId,l3IntfId,counterId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,counterId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, counterId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIntfEgressCounterId(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsL3SetRouterAclEnable
    #/********************************************************************************/
    def do_l3_set_router_acl_enable(self, arg):
        '''
         xpsL3SetRouterAclEnable: Enter [ devId,l3IfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IfId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetRouterAclEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3SetRouterAclId
    #/********************************************************************************/
    def do_l3_set_router_acl_id(self, arg):
        '''
         xpsL3SetRouterAclId: Enter [ devId,l3IfId,aclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IfId,aclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, l3IfId=%d, aclId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetRouterAclId(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetRouterAclId
    #/********************************************************************************/
    def do_l3_get_router_acl_id(self, arg):
        '''
         xpsL3GetRouterAclId: Enter [ devId,l3IfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            aclId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, l3IfId=%d' % (args[0],args[1]))
            ret = xpsL3GetRouterAclId(args[0],args[1],aclId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('aclId = %d' % (uint32_tp_value(aclId_Ptr_2)))
                pass
            delete_uint32_tp(aclId_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3GetRouterAclEnable
    #/********************************************************************************/
    def do_l3_get_router_acl_enable(self, arg):
        '''
         xpsL3GetRouterAclEnable: Enter [ devId,l3IfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, l3IfId=%d' % (args[0],args[1]))
            ret = xpsL3GetRouterAclEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3InitIpHost
    #/********************************************************************************/
    def do_l3_init_ip_host(self, arg):
        '''
         xpsL3InitIpHost: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsL3InitIpHost()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3InitIpHostScope
    #/********************************************************************************/
    def do_l3_init_ip_host_scope(self, arg):
        '''
         xpsL3InitIpHostScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3InitIpHostScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DeInitIpHost
    #/********************************************************************************/
    def do_l3_de_init_ip_host(self, arg):
        '''
         xpsL3DeInitIpHost: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsL3DeInitIpHost()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DeInitIpHostScope
    #/********************************************************************************/
    def do_l3_de_init_ip_host_scope(self, arg):
        '''
         xpsL3DeInitIpHostScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3DeInitIpHostScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3AddIpHostDevice
    #/********************************************************************************/
    def do_l3_add_ip_host_device(self, arg):
        '''
         xpsL3AddIpHostDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsL3AddIpHostDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3RemoveIpHostDevice
    #/********************************************************************************/
    def do_l3_remove_ip_host_device(self, arg):
        '''
         xpsL3RemoveIpHostDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsL3RemoveIpHostDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass


    #/********************************************************************************/
    # command for xpsL3AddIpHostEntry
    #/********************************************************************************/
    def do_l3_add_ip_host_entry(self, arg):
        '''
         xpsL3AddIpHostEntry: Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,pktCmd, serviceInstId, vpnLabel, propTTL, l3InterfaceId, macDa, egressIntfId, reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 13
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,pktCmd, serviceInstId, vpnLabel, propTTL, l3InterfaceId, macDa, egressIntfId, reasonCode ]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)

            xpsL3HostEntry_t_Ptr.nhEntry.pktCmd = int(args[5])
            xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId = int(args[6])
            xpsL3HostEntry_t_Ptr.nhEntry.vpnLabel = int(args[7])
            xpsL3HostEntry_t_Ptr.nhEntry.propTTL = int(args[8])
            xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId = int(args[9])
            macList = args[10].split(":")
            listLen = len(macList)
            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix] = int(macList[ix], 16)
            xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId = int(str(args[11]).strip(' '),16)
            xpsL3HostEntry_t_Ptr.nhEntry.reasonCode = int(args[12])

            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%s, vrfId=%s, type=%s, ipv4Addr=%s, ipv6Addr=%s, pktCmd=%s, serviceInstId=%s, propTTL=%s, l3InterfaceId=%s, macDa=%s, egressIntfId=%s, reasonCode=%s' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11]))
            ret = xpsL3AddIpHostEntry(args[0],xpsL3HostEntry_t_Ptr,xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3HostEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3HostEntry_t_Ptr.type))
                print('ipv4Addr = '),
                args[3] = args[3].replace(".",":").replace(",",":")
                postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)

                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                args[4] = args[4].replace(".",":").replace(",",":")
                postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)

                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.pktCmd))
                print('serviceInstId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId))
                print('vpnLabel = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.vpnLabel))
                print('propTTL = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.propTTL))
                print('l3InterfaceId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId))
                print('macDa = ' )
                for ix in range(0, 6):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix])),
                    if ix < (5):
                        sys.stdout.write(':'),
                print('')
                print('egressIntfId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId))
                print('reasonCode = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.reasonCode))

                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # command for route_entry_scaling
    #/********************************************************************************/
    def do_l3_route_entry_scaling(self, arg):
        '''
           route_entry_scaling: Enter [ devId, subTrieMode, configFile]
        '''
        import cProfile, pstats, StringIO
        pr = cProfile.Profile()
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, subTrieMode, configFile(path from xdk) ]')
        else:
            OutOfId = 0
            first = 0
            fisrtFailed = 0
            totalNoEntry = 0
            totalFailed = 0
            totalAdded = 0
            rehashFail = 0
            cnt = 0
            subTrieMode = int(args[1])
            configName = str(args[2])
            fd = open(configName, "r")
            for line in fd:
                if "#" in line or len(line.strip('\n')) == 0:
                    continue
                line = ' '.join(line.split())
                routeData = line.split(" ")
                args[0] = int(args[0])
                xpsL3RouteEntry_t_Ptr = new_xpsL3RouteEntry_tp()
                xpsL3RouteEntry_t_Ptr.vrfId = int(routeData[1].strip(''))
                xpsL3RouteEntry_t_Ptr.type = 0

                routeData[0] = routeData[0].replace(".",":").replace(",",":")
                postList = routeData[0].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)

                if listLen != 4 or invalid_ipv4_address(postList):
                    print("Invalid IPv4 address, please use x.x.x.x format.")
                    return

                for ix in range(listLen-1, -1, -1):
                    xpsL3RouteEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

                listLen = 16
                for ix in range(listLen-1, -1, -1):
                    xpsL3RouteEntry_t_Ptr.ipv6Addr[ix] = 0
                xpsL3RouteEntry_t_Ptr.ipMaskLen = int(routeData[2].strip(''))
                xpsL3RouteEntry_t_Ptr.nhEcmpSize = int(routeData[3].strip(''))
                xpsL3RouteEntry_t_Ptr.nhId = int(routeData[4].strip(''))
                prfxBucketIdx_Ptr_8 = new_uint32_tp()
                pr.enable()
                ret = xpsL3AddIpRouteEntryBySubTrieMode(args[0],xpsL3RouteEntry_t_Ptr,prfxBucketIdx_Ptr_8, subTrieMode)
                pr.disable()
                cnt = cnt + 1
                #print cnt
                err = 0
                if ret != 0:
                    if int(ret) == 49:  # 49 = XP_ERR_OUT_OF_IDS
                        OutOfId += 1
                    if int(ret) == 18:  # 49 = XP_ERR_REHASH_FAIL
                        rehashFail += 1
                    if first == 0:
                        fisrtFailed = totalNoEntry
                        first = 1
                    totalFailed += 1
                    err = 1
                if err == 0:
                    totalAdded += 1
                totalNoEntry += 1
                delete_uint32_tp(prfxBucketIdx_Ptr_8)
                delete_xpsL3RouteEntry_tp(xpsL3RouteEntry_t_Ptr)
            print cnt
            print "\n" + "="*50
            print "Summary Detils for route scaling test"
            print "-"*50
            print "subTrieMode      : ",subTrieMode
            print "totalNumEntry     : ",totalNoEntry
            print "totalAdded       : ",totalAdded
            print "totalFailed      : ",totalFailed
            print "fisrtFailed      : ",fisrtFailed
            print "totalrehashFail  : ",rehashFail
            print "OutOfId          : ",OutOfId
            print "="*50 + "\n"
            print "\n profilers details for route scaling\n"
            s = StringIO.StringIO()
            sortby = 'cumulative'
            ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
            ps.print_stats()
            print s.getvalue()

        

    #/********************************************************************************/
    # command for xpsL3AddIpv4HostEntry
    #/********************************************************************************/
    def do_l3_add_ipv4_host_entry(self, arg):
        '''
         xpsL3AddIpV4HostEntry: Enter [ devId,vrfId,ipv4Addr,pktCmd, serviceInstId, vpnLabel, propTTL, l3InterfaceId, macDa, egressIntfId, reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 11
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId, ipv4Addr,pktCmd, serviceInstId, vpnLabel, propTTL, l3InterfaceId, macDa, egressIntfId, reasonCode ]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = 0
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])
            listLen = 16

            for ix in range(0, listLen, 1): 
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = 0

            xpsL3HostEntry_t_Ptr.nhEntry.pktCmd = int(args[3])
            xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId = int(args[4])
            xpsL3HostEntry_t_Ptr.nhEntry.vpnLabel = int(args[5])
            xpsL3HostEntry_t_Ptr.nhEntry.propTTL = int(args[6])
            xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId = int(args[7])
            macList = args[8].split(":")
            listLen = len(macList)
            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix] = int(macList[ix], 16)
            xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId = int(args[9])
            xpsL3HostEntry_t_Ptr.nhEntry.reasonCode = int(args[10])

            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%s, vrfId=%s, ipv4Addr=%s, pktCmd=%s, serviceInstId=%s, propTTL=%s, l3InterfaceId=%s, macDa=%s, egressIntfId=%s, reasonCode=%s' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10]))
            ret = xpsL3AddIpHostEntry(args[0],xpsL3HostEntry_t_Ptr,xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3HostEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3HostEntry_t_Ptr.type))
                print('ipv4Addr = '),
                args[2] = args[2].replace(".",":").replace(",",":")
                postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)

                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.pktCmd))
                print('serviceInstId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId))
                print('vpnLabel = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.vpnLabel))
                print('propTTL = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.propTTL))
                print('l3InterfaceId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId))
                print('macDa = ' )
                for ix in range(0, 6):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix])),
                    if ix < (5):
                        sys.stdout.write(':'),
                print('')
                print('egressIntfId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId))
                print('reasonCode = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.reasonCode))

                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3AddIpv6HostEntry
    #/********************************************************************************/
    def do_l3_add_ipv6_host_entry(self, arg):
        '''
         xpsL3AddIpv6HostEntry: Enter [ devId,vrfId,ipv6Addr,pktCmd, serviceInstId, vpnLabel, propTTL, l3InterfaceId, macDa, egressIntfId, reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 11
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,ipv6Addr,pktCmd, serviceInstId, vpnLabel, propTTL, l3InterfaceId, macDa, egressIntfId, reasonCode ]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = 1
            listLen = 4

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = 0
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)

            xpsL3HostEntry_t_Ptr.nhEntry.pktCmd = int(args[3])
            xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId = int(args[4])
            xpsL3HostEntry_t_Ptr.nhEntry.vpnLabel = int(args[5])
            xpsL3HostEntry_t_Ptr.nhEntry.propTTL = int(args[6])
            xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId = int(args[7])
            macList = args[8].split(":")
            listLen = len(macList)
            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix] = int(macList[ix], 16)
            xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId = int(args[9])
            xpsL3HostEntry_t_Ptr.nhEntry.reasonCode = int(args[10])

            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%s, vrfId=%s, ipv6Addr=%s, pktCmd=%s, serviceInstId=%s, propTTL=%s, l3InterfaceId=%s, macDa=%s, egressIntfId=%s, reasonCode=%s' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10]))
            ret = xpsL3AddIpHostEntry(args[0],xpsL3HostEntry_t_Ptr,xpsHashIndexList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3HostEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3HostEntry_t_Ptr.type))
                print('ipv6Addr = '),
                args[2] = args[2].replace(".",":").replace(",",":")
                postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)

                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.pktCmd))
                print('serviceInstId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId))
                print('vpnLabel = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.vpnLabel))
                print('propTTL = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.propTTL))
                print('l3InterfaceId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId))
                print('macDa = ' )
                for ix in range(0, 6):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix])),
                    if ix < (5):
                        sys.stdout.write(':'),
                print('')
                print('egressIntfId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId))
                print('reasonCode = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.reasonCode))

                print('Size of Index List = %d' % (xpsHashIndexList_t_Ptr.size))
                print('HashIndexList = '),
                for i in range(0, xpsHashIndexList_t_Ptr.size):
                    print xpsHashIndexList_t_Ptr.index[i],
                print('')
                pass
            delete_xpsHashIndexList_tp(xpsHashIndexList_t_Ptr)
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3RemoveIpHostEntry
    #/********************************************************************************/
    def do_l3_remove_ip_host_entry(self, arg):
        '''
         xpsL3RemoveIpHostEntry: Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr ]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(0, listLen, 1): 
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(0, listLen, 1): 
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)
            print('Input Arguments are, devId=%s, vrfId=%s, type=%s, ipv4Addr=%s, ipv6Addr=%s' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsL3RemoveIpHostEntry(args[0],xpsL3HostEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3HostEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3HostEntry_t_Ptr.type))
                print('ipv4Addr = '),
                args[3] = args[3].replace(".",":").replace(",",":")
                postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                args[4] = args[4].replace(".",":").replace(",",":")
                postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.pktCmd))
                print('serviceInstId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId))
                print('propTTL = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.propTTL))
                print('l3InterfaceId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId))
                print('macDa = ' )
                for ix in range(0, 6):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix])),
                    if ix < (5):
                        sys.stdout.write(':'),
                print('')
                print('egressIntfId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId))
                pass
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # Auto completion for ipPrefixType enumerations
    #/********************************************************************************/
    def complete_l3_remove_ip_host_entry_by_index(self, text, line, begidx, endidx):
        tempDict = { 3 : 'ipPrefixType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsL3RemoveIpHostEntryByIndex
    #/********************************************************************************/
    def do_l3_remove_ip_host_entry_by_index(self, arg):
        '''
          xpsL3RemoveIpHostEntryByIndex: Enter [ devId, index, type(ipv4-0/ipv6-1) ]
          Valid values for type : <ipPrefixType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, index, type(ipv4-0/ipv6-1) ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
	    args[2]= eval(args[2])

            #print('Input Arguments are deviceId =%s, index=%s' % (args[0],args[1]))
            ret = xpsL3RemoveIpHostEntryByIndex(args[0],args[1],args[2])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
    #/********************************************************************************/
    # command for xpsL3UpdateIpHostEntry
    #/********************************************************************************/
    def do_l3_update_ip_host_entry(self, arg):
        '''
         xpsL3UpdateIpHostEntry: Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,pktCmd, serviceInstId, vpnLabel, propTTL, l3InterfaceId, macDa, egressIntfId, reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 13
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,pktCmd, serviceInstId, vpnLabel, propTTL, l3InterfaceId, macDa, egressIntfId, reasonCode ]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(0, listLen, 1): 
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)


            xpsL3HostEntry_t_Ptr.nhEntry.pktCmd = int(args[5])
            xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId = int(args[6])
            xpsL3HostEntry_t_Ptr.nhEntry.vpnLabel = int(args[7])
            xpsL3HostEntry_t_Ptr.nhEntry.propTTL = int(args[8])
            xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId = int(args[9])
            macList = args[10].split(":")
            listLen = len(macList)
            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix] = int(macList[ix], 16)
            xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId = int(args[11])
            xpsL3HostEntry_t_Ptr.nhEntry.reasonCode = int(args[12])

            #print('Input Arguments are, devId=%s, vrfId=%s, type=%s, ipv4Addr=%s, ipv6Addr=%s, pktCmd=%s, serviceInstId=%s, propTTL=%s, l3InterfaceId=%s, macDa=%s, egressIntfId=%s, reasonCode=%s' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11]))
            ret = xpsL3UpdateIpHostEntry(args[0],xpsL3HostEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3HostEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3HostEntry_t_Ptr.type))
                print('ipv4Addr = '),
                args[3] = args[3].replace(".",":").replace(",",":")
                postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)

                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                args[4] = args[4].replace(".",":").replace(",",":")
                postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)

                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.pktCmd))
                print('serviceInstId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId))
                print('vpnLabel = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.vpnLabel))
                print('propTTL = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.propTTL))
                print('l3InterfaceId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId))
                print('macDa = ' )
                for ix in range(0, 6):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix])),
                    if ix < (5):
                        sys.stdout.write(':'),
                print('')
                print('egressIntfId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId))
                print('reasonCode = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.reasonCode))
                pass
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsL3UpdateIpHostEntryByIndex
    #/********************************************************************************/
    def do_l3_update_ip_host_entry_by_index(self, arg):
        '''
         xpsL3UpdateIpHostEntryByIndex: Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,pktCmd, serviceInstId, vpnLabel, propTTL, l3InterfaceId, macDa, egressIntfId, reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 13
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,pktCmd, serviceInstId, vpnLabel, propTTL, l3InterfaceId, macDa, egressIntfId, reasonCode ]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)

            xpsL3HostEntry_t_Ptr.nhEntry.pktCmd = int(args[5])
            xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId = int(args[6])
            xpsL3HostEntry_t_Ptr.nhEntry.vpnLabel = int(args[7])
            xpsL3HostEntry_t_Ptr.nhEntry.propTTL = int(args[8])
            xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId = int(args[9])
            macList = args[10].split(":")
            listLen = len(macList)
            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix] = int(macList[ix], 16)
            xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId = int(args[11])
            xpsL3HostEntry_t_Ptr.nhEntry.reasonCode = int(args[12])

            #print('Input Arguments are, devId=%s, vrfId=%s, type=%s, ipv4Addr=%s, ipv6Addr=%s, pktCmd=%s, serviceInstId=%s, propTTL=%s, l3InterfaceId=%s, macDa=%s, egressIntfId=%s, reasonCode=%s' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10],args[11]))
            ret = xpsL3UpdateIpHostEntryByIndex(args[0],xpsL3HostEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3HostEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3HostEntry_t_Ptr.type))
                print('ipv4Addr = '),
                args[3] = args[3].replace(".",":").replace(",",":")
                postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)

                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                args[4] = args[4].replace(".",":").replace(",",":")
                postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)

                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.pktCmd))
                print('serviceInstId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId))
                print('vpnLabel = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.vpnLabel))
                print('propTTL = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.propTTL))
                print('l3InterfaceId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId))
                print('macDa = ' )
                for ix in range(0, 6):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix])),
                    if ix < (5):
                        sys.stdout.write(':'),
                print('')
                print('egressIntfId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId))
                print('reasonCode = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.reasonCode))
                pass
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3GetNumOfValidIpHostEntries
    #/********************************************************************************/
    def do_l3_get_num_of_valid_ip_host_entries(self, arg):
        '''
         xpsL3GetNumOfValidIpHostEntries: Enter [ devId,type ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,type ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            numOfValidEntries_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, type=%d' % (args[0],args[1]))
            ret = xpsL3GetNumOfValidIpHostEntries(args[0],args[1],numOfValidEntries_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfValidEntries = %d' % (uint32_tp_value(numOfValidEntries_Ptr_2)))
                pass
            delete_uint32_tp(numOfValidEntries_Ptr_2)

    #/********************************************************************************/
    # command for xpsL3GetNumOfValidNhEntries
    #/********************************************************************************/
    def do_l3_get_num_of_valid_nh_entries(self, arg):
        '''
         xpsL3GetNumOfValidNhEntries: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfValidEntries_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, type=%d' % (args[0],args[1]))
            ret = xpsL3GetNumOfValidNhEntries(args[0],numOfValidEntries_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfValidEntries = %d' % (uint32_tp_value(numOfValidEntries_Ptr_2)))
                pass
            delete_uint32_tp(numOfValidEntries_Ptr_2)

    #/********************************************************************************/
    # command for xpsL3NhGetTableSize
    #/********************************************************************************/
    def do_l3_nh_get_table_size(self, arg):
        '''
         xpsL3NhGetTableSize: Enter [ devId]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId]')
        else:
            args[0] = int(args[0])
            tableSize = new_uint32_tp()
            ret = xpsL3NhGetTableSize(args[0],tableSize)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('NH table size = %d' % (uint32_tp_value(tableSize)))
                pass
            delete_uint32_tp(tableSize)

    #/********************************************************************************/
    # command for xpsL3GetIpHostEntry
    #/********************************************************************************/
    def do_l3_get_ip_host_entry(self, arg):
        '''
         xpsL3GetIpHostEntry: Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(0, listLen, 1): 
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(0, listLen, 1): 
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)

            print('Input Arguments are, devId=%s, vrfId=%s, type=%s, ipv4Addr=%s, ipv6Addr=%s' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsL3GetIpHostEntry(args[0],xpsL3HostEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3HostEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3HostEntry_t_Ptr.type))
                print('ipv4Addr = '),
                args[3] = args[3].replace(".",":").replace(",",":")
                postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                args[4] = args[4].replace(".",":").replace(",",":")
                postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.pktCmd))
                print('serviceInstId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId))
                print('vpnLabel = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.vpnLabel))
                print('propTTL = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.propTTL))
                print('l3InterfaceId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId))
                print('macDa = ' )
                for ix in range(0, 6):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix])),
                    if ix < (5):
                        sys.stdout.write(':'),
                print('')
                print('egressIntfId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId))
                pass
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3GetIpHostEntryByIndex
    #/********************************************************************************/
    def do_l3_get_ip_host_entry_by_index(self, arg):
        '''
         xpsL3GetIpHostEntryByIndex: Enter [ devId,index,type ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,type ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.type = int(args[2])

            print('Input Arguments are deviceId=%s, index=%s, type =%s' % (args[0],args[1],args[2]))
            ret = xpsL3GetIpHostEntryByIndex(args[0],args[1],xpsL3HostEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('l3InterfaceId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId))
                print('macDa = ' )
                for ix in range(0, 6):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix])),
                    if ix < (5):
                        sys.stdout.write(':'),
                print('')
                print('egressIntfId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId))
                pass
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3FindIpHostEntry
    #/********************************************************************************/
    def do_l3_find_ip_host_entry(self, arg):
        '''
         xpsL3FindIpHostEntry: Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) != numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr ]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)

            xpsL3HostEntry_t_Ptr.nhEntry.pktCmd = 0
            xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId = 0
            xpsL3HostEntry_t_Ptr.nhEntry.propTTL = 0
            xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId = 0
            #macList = args[9].split(":")
            #for i in range(6):
              #  xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[i] = int(macList[i], 16)
            xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId =0

            index_Ptr_6 = new_intp()

            #print('Input Arguments are, devId=%s, vrfId=%s, type=%s, ipv4Addr=%s, ipv6Addr=%s, pktCmd=%s, serviceInstId=%s, propTTL=%s, l3InterfaceId=%s, macDa=%s,egressIntfId=%s' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9],args[10]))


            ret = xpsL3FindIpHostEntry(args[0],xpsL3HostEntry_t_Ptr,index_Ptr_6)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3HostEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3HostEntry_t_Ptr.type))
                print('ipv4Addr = '),
                args[3] = args[3].replace(".",":").replace(",",":")
                postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                args[4] = args[4].replace(".",":").replace(",",":")
                postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('pktCmd = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.pktCmd))
                print('serviceInstId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.serviceInstId))
                print('propTTL = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.propTTL))
                print('l3InterfaceId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.l3InterfaceId))
                print('macDa = ' )
                for ix in range(0, 6):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.macDa[ix])),
                    if ix < (5):
                        sys.stdout.write(':'),
                print('')
                print('egressIntfId = %d' % (xpsL3HostEntry_t_Ptr.nhEntry.nextHop.egressIntfId))
                print('index = %d' % (intp_value(index_Ptr_6)))
                pass
            delete_intp(index_Ptr_6)
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3FlushIpHostEntries
    #/********************************************************************************/
    def do_l3_flush_ip_host_entries(self, arg):
        '''
         xpsL3FlushIpHostEntries: Enter devId,vrfId,type,ipv4Addr,ipv6Addr,nhEntry [ updateShadow ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter devId,vrfId,type,ipv4Addr,ipv6Addr,nhEntry [ updateShadow ]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)
            xpsL3HostEntry_t_Ptr.nhEntry = int(args[5])

            if(len(args) > numArgsReq):
                args[6] = int(args[6])
                #print('Input Arguments are, devId=%d, vrfId=%d, type=%d, ipv4Addr=%s, ipv6Addr=%s, nhEntry=%d, updateShadow=%d' % (args[0],xpsL3HostEntry_t_Ptr.vrfId,xpsL3HostEntry_t_Ptr.type,args[3],args[4],xpsL3HostEntry_t_Ptr.nhEntry, args[6]))
                ret = xpsL3FlushIpHostEntriesWithShadowUpdate(args[0],xpsL3HostEntry_t_Ptr,args[6])
            else:
                #print('Input Arguments are, devId=%d, vrfId=%d, type=%d, ipv4Addr=%s, ipv6Addr=%s, nhEntry=%d' % (args[0],xpsL3HostEntry_t_Ptr.vrfId,xpsL3HostEntry_t_Ptr.type,args[3],args[4],xpsL3HostEntry_t_Ptr.nhEntry))
                ret = xpsL3FlushIpHostEntriesWithShadowUpdate(args[0],xpsL3HostEntry_t_Ptr, 1)

            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3HostEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3HostEntry_t_Ptr.type))
                print('ipv4Addr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3HostEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('nhEntry = %d' % (xpsL3HostEntry_t_Ptr.nhEntry))
                pass
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3AddIpv4HostControlEntry
    #/********************************************************************************/
    def do_l3_add_ipv4_host_control_entry(self, arg):
        '''
         xpsL3AddIpv4HostControlEntry: Enter [ devId,vrfId,ipv4Addr,reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,ip,reasonCode ]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = 0
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])
            listLen = 16

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = 0

            xpsL3HostEntry_t_Ptr.nhEntry.reasonCode = int(args[3])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, vrfId=%d, ip=%s, reasonCode=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsL3AddIpHostControlEntry(args[0],xpsL3HostEntry_t_Ptr,xpsHashIndexList_t_Ptr)
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
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3AddIpv6HostControlEntry
    #/********************************************************************************/
    def do_l3_add_ipv6_host_control_entry(self, arg):
        '''
         xpsL3AddIpv6HostControlEntry: Enter [ devId,vrfId,ipv6Addr,reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,ip,reasonCode ]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = 1
            listLen = 4

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = 0
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)

            xpsL3HostEntry_t_Ptr.nhEntry.reasonCode = int(args[3])
            xpsHashIndexList_t_Ptr = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, vrfId=%d, ip=%s, reasonCode=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsL3AddIpHostControlEntry(args[0],xpsL3HostEntry_t_Ptr,xpsHashIndexList_t_Ptr)
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
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3GetIpv4HostControlEntryReasonCode
    #/********************************************************************************/
    def do_l3_get_ipv4_host_control_entry_reason_code(self, arg):
        '''
         xpsL3GetIpv4HostControlEntryReasonCode: Enter [ devId,vrfId,ipv4Addr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,ip ]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = 0
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])
            listLen = 16

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = 0

            reasonCode_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vrfId=%d, ip=%s' % (args[0],args[1],args[2]))
            ret = xpsL3GetIpHostControlEntryReasonCode(args[0],xpsL3HostEntry_t_Ptr,reasonCode_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('reasonCode = %d' % (uint32_tp_value(reasonCode_Ptr_3)))
                pass
            delete_uint32_tp(reasonCode_Ptr_3)
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3GetIpv6HostControlEntryReasonCode
    #/********************************************************************************/
    def do_l3_get_ipv6_host_control_entry_reason_code(self, arg):
        '''
         xpsL3GetIpv6HostControlEntryReasonCode: Enter [ devId,vrfId,ipv6Addr ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,ip ]')
        else:
            args[0] = int(args[0])
            xpsL3HostEntry_t_Ptr = new_xpsL3HostEntry_tp()
            xpsL3HostEntry_t_Ptr.vrfId = int(args[1])
            xpsL3HostEntry_t_Ptr.type = 1
            listLen = 4

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv4Addr[ix] = 0
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3HostEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)

            reasonCode_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vrfId=%d, ip=%s' % (args[0],args[1],args[2]))
            ret = xpsL3GetIpHostControlEntryReasonCode(args[0],xpsL3HostEntry_t_Ptr,reasonCode_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('reasonCode = %d' % (uint32_tp_value(reasonCode_Ptr_3)))
                pass
            delete_uint32_tp(reasonCode_Ptr_3)
            delete_xpsL3HostEntry_tp(xpsL3HostEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3InitIpRoute
    #/********************************************************************************/
    def do_l3_init_ip_route(self, arg):
        '''
         xpsL3InitIpRoute: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsL3InitIpRoute()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3InitIpRouteScope
    #/********************************************************************************/
    def do_l3_init_ip_route_scope(self, arg):
        '''
         xpsL3InitIpRouteScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3InitIpRouteScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DeInitIpRoute
    #/********************************************************************************/
    def do_l3_de_init_ip_route(self, arg):
        '''
         xpsL3DeInitIpRoute: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsL3DeInitIpRoute()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DeInitIpRouteScope
    #/********************************************************************************/
    def do_l3_de_init_ip_route_scope(self, arg):
        '''
         xpsL3DeInitIpRouteScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3DeInitIpRouteScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3AddIpRouteDevice
    #/********************************************************************************/
    def do_l3_add_ip_route_device(self, arg):
        '''
         xpsL3AddIpRouteDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsL3AddIpRouteDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3RemoveIpRouteDevice
    #/********************************************************************************/
    def do_l3_remove_ip_route_device(self, arg):
        '''
         xpsL3RemoveIpRouteDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsL3RemoveIpRouteDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3AddIpRouteEntry
    #/********************************************************************************/
    def do_l3_add_ip_route_entry(self, arg):
        '''
         xpsL3AddIpRouteEntry: Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]')
        else:
            args[0] = int(args[0])
            xpsL3RouteEntry_t_Ptr = new_xpsL3RouteEntry_tp()
            xpsL3RouteEntry_t_Ptr.vrfId = int(args[1])
            xpsL3RouteEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3RouteEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(0, listLen, 1):
                xpsL3RouteEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)
            xpsL3RouteEntry_t_Ptr.ipMaskLen = int(args[5])
            xpsL3RouteEntry_t_Ptr.nhEcmpSize = int(args[6])
            xpsL3RouteEntry_t_Ptr.nhId = int(args[7])
            prfxBucketIdx_Ptr_8 = new_uint32_tp()

            #print('Input Arguments are, devId=%d, vrfId=%d, type=%d, ipv4Addr=%s, ipv6Addr=%s, ipMaskLen=%d, nhEcmpSize=%d, nhId=%d' % (args[0],xpsL3RouteEntry_t_Ptr.vrfId,xpsL3RouteEntry_t_Ptr.type,args[3],args[4],xpsL3RouteEntry_t_Ptr.ipMaskLen,xpsL3RouteEntry_t_Ptr.nhEcmpSize,xpsL3RouteEntry_t_Ptr.nhId))
            ret = xpsL3AddIpRouteEntry(args[0],xpsL3RouteEntry_t_Ptr,prfxBucketIdx_Ptr_8)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3RouteEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3RouteEntry_t_Ptr.type))
                args[3] = args[3].replace(".",":").replace(",",":")
                postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                print('ipv4Addr = '),
                for ix in range(0, listLen):
                    print('%d' % (xpsL3RouteEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                args[4] = args[4].replace(".",":").replace(",",":")
                postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                print('ipv6Addr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipMaskLen = %d' % (xpsL3RouteEntry_t_Ptr.ipMaskLen))
                print('nhEcmpSize = %d' % (xpsL3RouteEntry_t_Ptr.nhEcmpSize))
                print('nhId = %d' % (xpsL3RouteEntry_t_Ptr.nhId))
                print('prfxBucketIdx = %d' % (uint32_tp_value(prfxBucketIdx_Ptr_8)))
                pass
            delete_uint32_tp(prfxBucketIdx_Ptr_8)
            delete_xpsL3RouteEntry_tp(xpsL3RouteEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3AddIpv4RouteEntry
    #/********************************************************************************/
    def do_l3_add_ipv4_route_entry(self, arg):
        '''
         xpsL3AddIpRouteEntry: Enter [ devId,vrfId,ipv4Addr,ipMaskLen,nhEcmpSize,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,ipv4Addr,ipMaskLen,nhEcmpSize,nhId ]')
        else:
            args[0] = int(args[0])
            xpsL3RouteEntry_t_Ptr = new_xpsL3RouteEntry_tp()
            xpsL3RouteEntry_t_Ptr.vrfId = int(args[1])
            xpsL3RouteEntry_t_Ptr.type = 0

            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            listLen = 16
            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv6Addr[listLen - ix - 1] = 0
            xpsL3RouteEntry_t_Ptr.ipMaskLen = int(args[3])
            xpsL3RouteEntry_t_Ptr.nhEcmpSize = int(args[4])
            xpsL3RouteEntry_t_Ptr.nhId = int(args[5])
            prfxBucketIdx_Ptr_8 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vrfId=%d, ipv4Addr=%s, ipMaskLen=%d, nhEcmpSize=%d, nhId=%d' % (args[0],xpsL3RouteEntry_t_Ptr.vrfId,args[2],xpsL3RouteEntry_t_Ptr.ipMaskLen,xpsL3RouteEntry_t_Ptr.nhEcmpSize,xpsL3RouteEntry_t_Ptr.nhId))
            ret = xpsL3AddIpRouteEntry(args[0],xpsL3RouteEntry_t_Ptr,prfxBucketIdx_Ptr_8)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3RouteEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3RouteEntry_t_Ptr.type))
                args[2] = args[2].replace(".",":").replace(",",":")
                postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                print('ipv4Addr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipMaskLen = %d' % (xpsL3RouteEntry_t_Ptr.ipMaskLen))
                print('nhEcmpSize = %d' % (xpsL3RouteEntry_t_Ptr.nhEcmpSize))
                print('nhId = %d' % (xpsL3RouteEntry_t_Ptr.nhId))
                print('prfxBucketIdx = %d' % (uint32_tp_value(prfxBucketIdx_Ptr_8)))
                pass
            delete_uint32_tp(prfxBucketIdx_Ptr_8)
            delete_xpsL3RouteEntry_tp(xpsL3RouteEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3AddIpv6RouteEntry
    #/********************************************************************************/
    def do_l3_add_ipv6_route_entry(self, arg):
        '''
         xpsL3AddIpRouteEntry: Enter [ devId,vrfId,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]')
        else:
            args[0] = int(args[0])
            xpsL3RouteEntry_t_Ptr = new_xpsL3RouteEntry_tp()
            xpsL3RouteEntry_t_Ptr.vrfId = int(args[1])
            xpsL3RouteEntry_t_Ptr.type = 1
            listLen = 4
            for ix in range(0, listLen, 1):
                xpsL3RouteEntry_t_Ptr.ipv4Addr[ix] = 0
            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)
            xpsL3RouteEntry_t_Ptr.ipMaskLen = int(args[3])
            xpsL3RouteEntry_t_Ptr.nhEcmpSize = int(args[4])
            xpsL3RouteEntry_t_Ptr.nhId = int(args[5])
            prfxBucketIdx_Ptr_8 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vrfId=%d, ipv6Addr=%s, ipMaskLen=%d, nhEcmpSize=%d, nhId=%d' % (args[0],xpsL3RouteEntry_t_Ptr.vrfId,args[2],args[3],xpsL3RouteEntry_t_Ptr.ipMaskLen,xpsL3RouteEntry_t_Ptr.nhEcmpSize,xpsL3RouteEntry_t_Ptr.nhId))
            ret = xpsL3AddIpRouteEntry(args[0],xpsL3RouteEntry_t_Ptr,prfxBucketIdx_Ptr_8)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3RouteEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3RouteEntry_t_Ptr.type))
                args[2] = args[2].replace(".",":").replace(",",":")
                postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                print('ipv6Addr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipMaskLen = %d' % (xpsL3RouteEntry_t_Ptr.ipMaskLen))
                print('nhEcmpSize = %d' % (xpsL3RouteEntry_t_Ptr.nhEcmpSize))
                print('nhId = %d' % (xpsL3RouteEntry_t_Ptr.nhId))
                print('prfxBucketIdx = %d' % (uint32_tp_value(prfxBucketIdx_Ptr_8)))
                pass
            delete_uint32_tp(prfxBucketIdx_Ptr_8)
            delete_xpsL3RouteEntry_tp(xpsL3RouteEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3RemoveIpRouteEntry
    #/********************************************************************************/
    def do_l3_remove_ip_route_entry(self, arg):
        '''
         xpsL3RemoveIpRouteEntry: Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]')
        else:
            args[0] = int(args[0])
            xpsL3RouteEntry_t_Ptr = new_xpsL3RouteEntry_tp()
            xpsL3RouteEntry_t_Ptr.vrfId = int(args[1])
            xpsL3RouteEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)
            xpsL3RouteEntry_t_Ptr.ipMaskLen = int(args[5])
            xpsL3RouteEntry_t_Ptr.nhEcmpSize = int(args[6])
            xpsL3RouteEntry_t_Ptr.nhId = int(args[7])

            #print('Input Arguments are, devId=%d, vrfId=%d, type=%d, ipv4Addr=%s, ipv6Addr=%s, ipMaskLen=%d, nhEcmpSize=%d, nhId=%d' % (args[0],xpsL3RouteEntry_t_Ptr.vrfId,xpsL3RouteEntry_t_Ptr.type,args[3],args[4],xpsL3RouteEntry_t_Ptr.ipMaskLen,xpsL3RouteEntry_t_Ptr.nhEcmpSize,xpsL3RouteEntry_t_Ptr.nhId))
            ret = xpsL3RemoveIpRouteEntry(args[0],xpsL3RouteEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3RouteEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3RouteEntry_t_Ptr.type))
                print('ipv4Addr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipMaskLen = %d' % (xpsL3RouteEntry_t_Ptr.ipMaskLen))
                print('nhEcmpSize = %d' % (xpsL3RouteEntry_t_Ptr.nhEcmpSize))
                print('nhId = %d' % (xpsL3RouteEntry_t_Ptr.nhId))
                pass
            delete_xpsL3RouteEntry_tp(xpsL3RouteEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsL3RemoveIpRouteEntryByIndex
    #/********************************************************************************/
    def do_l3_remove_ip_route_entry_by_index(self, arg):
        '''
         xpsL3RemoveIpRouteEntryByIndex: Enter [ devId,prfxBucketIdx,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 9
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,prfxBucketIdx,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsL3RouteEntry_t_Ptr = new_xpsL3RouteEntry_tp()
            xpsL3RouteEntry_t_Ptr.vrfId = int(args[2])
            xpsL3RouteEntry_t_Ptr.type = eval(args[3])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[5] = args[5].replace(".",":").replace(",",":")
            postList = args[5].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)
            xpsL3RouteEntry_t_Ptr.ipMaskLen = int(args[6])
            xpsL3RouteEntry_t_Ptr.nhEcmpSize = int(args[7])
            xpsL3RouteEntry_t_Ptr.nhId = int(args[8])

            #print('Input Arguments are, devId=%d, prfxBucketIdx=%d, vrfId=%d, type=%d, ipv4Addr=%s, ipv6Addr=%s, ipMaskLen=%d, nhEcmpSize=%d, nhId=%d' % (args[0],args[1],xpsL3RouteEntry_t_Ptr.vrfId,xpsL3RouteEntry_t_Ptr.type,args[4],args[5],xpsL3RouteEntry_t_Ptr.ipMaskLen,xpsL3RouteEntry_t_Ptr.nhEcmpSize,xpsL3RouteEntry_t_Ptr.nhId))
            ret = xpsL3RemoveIpRouteEntryByIndex(args[0],args[1],xpsL3RouteEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3RouteEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3RouteEntry_t_Ptr.type))
                print('ipv4Addr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipMaskLen = %d' % (xpsL3RouteEntry_t_Ptr.ipMaskLen))
                print('nhEcmpSize = %d' % (xpsL3RouteEntry_t_Ptr.nhEcmpSize))
                print('nhId = %d' % (xpsL3RouteEntry_t_Ptr.nhId))
                pass
            delete_xpsL3RouteEntry_tp(xpsL3RouteEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsL3UpdateIpRouteEntry
    #/********************************************************************************/
    def do_l3_update_ip_route_entry(self, arg):
        '''
         xpsL3UpdateIpRouteEntry: Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]')
        else:
            args[0] = int(args[0])
            xpsL3RouteEntry_t_Ptr = new_xpsL3RouteEntry_tp()
            xpsL3RouteEntry_t_Ptr.vrfId = int(args[1])
            xpsL3RouteEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)
            xpsL3RouteEntry_t_Ptr.ipMaskLen = int(args[5])
            xpsL3RouteEntry_t_Ptr.nhEcmpSize = int(args[6])
            xpsL3RouteEntry_t_Ptr.nhId = int(args[7])

            #print('Input Arguments are, devId=%d, vrfId=%d, type=%d, ipv4Addr=%s, ipv6Addr=%s, ipMaskLen=%d, nhEcmpSize=%d, nhId=%d' % (args[0],xpsL3RouteEntry_t_Ptr.vrfId,xpsL3RouteEntry_t_Ptr.type,args[3],args[4],xpsL3RouteEntry_t_Ptr.ipMaskLen,xpsL3RouteEntry_t_Ptr.nhEcmpSize,xpsL3RouteEntry_t_Ptr.nhId))
            ret = xpsL3UpdateIpRouteEntry(args[0],xpsL3RouteEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3RouteEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3RouteEntry_t_Ptr.type))
                print('ipv4Addr = '),
                args[3] = args[3].replace(".",":").replace(",",":")
                postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                args[4] = args[4].replace(".",":").replace(",",":")
                postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipMaskLen = %d' % (xpsL3RouteEntry_t_Ptr.ipMaskLen))
                print('nhEcmpSize = %d' % (xpsL3RouteEntry_t_Ptr.nhEcmpSize))
                print('nhId = %d' % (xpsL3RouteEntry_t_Ptr.nhId))
                pass
            delete_xpsL3RouteEntry_tp(xpsL3RouteEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsL3UpdateIpRouteEntryByIndex
    #/********************************************************************************/
    def do_l3_update_ip_route_entry_by_index(self, arg):
        '''
         xpsL3UpdateIpRouteEntryByIndex: Enter [ devId,prfxBucketIdx,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 9
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,prfxBucketIdx,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsL3RouteEntry_t_Ptr = new_xpsL3RouteEntry_tp()
            xpsL3RouteEntry_t_Ptr.vrfId = int(args[2])
            xpsL3RouteEntry_t_Ptr.type = eval(args[3])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[5] = args[5].replace(".",":").replace(",",":")
            postList = args[5].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)
            xpsL3RouteEntry_t_Ptr.ipMaskLen = int(args[6])
            xpsL3RouteEntry_t_Ptr.nhEcmpSize = int(args[7])
            xpsL3RouteEntry_t_Ptr.nhId = int(args[8])

            #print('Input Arguments are, devId=%d, prfxBucketIdx=%d, vrfId=%d, type=%d, ipv4Addr=%s, ipv6Addr=%s, ipMaskLen=%d, nhEcmpSize=%d, nhId=%d' % (args[0],args[1],xpsL3RouteEntry_t_Ptr.vrfId,xpsL3RouteEntry_t_Ptr.type,args[4],args[5],xpsL3RouteEntry_t_Ptr.ipMaskLen,xpsL3RouteEntry_t_Ptr.nhEcmpSize,xpsL3RouteEntry_t_Ptr.nhId))
            ret = xpsL3UpdateIpRouteEntryByIndex(args[0],args[1],xpsL3RouteEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3RouteEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3RouteEntry_t_Ptr.type))
                print('ipv4Addr = '),
                args[4] = args[4].replace(".",":").replace(",",":")
                postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                args[5] = args[5].replace(".",":").replace(",",":")
                postList = args[5].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipMaskLen = %d' % (xpsL3RouteEntry_t_Ptr.ipMaskLen))
                print('nhEcmpSize = %d' % (xpsL3RouteEntry_t_Ptr.nhEcmpSize))
                print('nhId = %d' % (xpsL3RouteEntry_t_Ptr.nhId))
                pass
            delete_xpsL3RouteEntry_tp(xpsL3RouteEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsL3FindIpRouteEntry
    #/********************************************************************************/
    def do_l3_find_ip_route_entry(self, arg):
        '''
         xpsL3FindIpRouteEntry: Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen ]')
        else:
            args[0] = int(args[0])
            xpsL3RouteEntry_t_Ptr = new_xpsL3RouteEntry_tp()
            xpsL3RouteEntry_t_Ptr.vrfId = int(args[1])
            xpsL3RouteEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)
            xpsL3RouteEntry_t_Ptr.ipMaskLen = int(args[5])
            xpsL3RouteEntry_t_Ptr.nhEcmpSize = 0
            xpsL3RouteEntry_t_Ptr.nhId = 0
            prfxBucketIdx_Ptr_8 = new_uint32_tp()

            #print('Input Arguments are, devId=%d, vrfId=%d, type=%d, ipv4Addr=%s, ipv6Addr=%s, ipMaskLen=%d, nhEcmpSize=%d, nhId=%d' % (args[0],xpsL3RouteEntry_t_Ptr.vrfId,xpsL3RouteEntry_t_Ptr.type,args[3],args[4],xpsL3RouteEntry_t_Ptr.ipMaskLen,xpsL3RouteEntry_t_Ptr.nhEcmpSize,xpsL3RouteEntry_t_Ptr.nhId))
            ret = xpsL3FindIpRouteEntry(args[0],xpsL3RouteEntry_t_Ptr,prfxBucketIdx_Ptr_8)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3RouteEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3RouteEntry_t_Ptr.type))
                print('ipv4Addr = '),
                args[3] = args[3].replace(".",":").replace(",",":")
                postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                args[4] = args[4].replace(".",":").replace(",",":")
                postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipMaskLen = %d' % (xpsL3RouteEntry_t_Ptr.ipMaskLen))
                print('nhEcmpSize = %d' % (xpsL3RouteEntry_t_Ptr.nhEcmpSize))
                print('nhId = %d' % (xpsL3RouteEntry_t_Ptr.nhId))
                print('prfxBucketIdx = %d' % (uint32_tp_value(prfxBucketIdx_Ptr_8)))
                pass
            delete_uint32_tp(prfxBucketIdx_Ptr_8)
            delete_xpsL3RouteEntry_tp(xpsL3RouteEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsL3FindIpRouteLpmEntry
    #/********************************************************************************/
    def do_l3_find_ip_route_lpm_entry(self, arg):
        '''
         xpsL3FindIpRouteLpmEntry: Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId ]')
        else:
            args[0] = int(args[0])
            xpsL3RouteEntry_t_Ptr = new_xpsL3RouteEntry_tp()
            xpsL3RouteEntry_t_Ptr.vrfId = int(args[1])
            xpsL3RouteEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 4 or invalid_ipv4_address(postList):
                print("Invalid IPv4 address, please use x.x.x.x format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)

            if listLen != 16 or invalid_ipv6_address(postList):
                print("Invalid IPv6 address length or format, please use xx:xx:xx... format.")
                return

            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)
            xpsL3RouteEntry_t_Ptr.ipMaskLen = int(args[5])
            xpsL3RouteEntry_t_Ptr.nhEcmpSize = int(args[6])
            xpsL3RouteEntry_t_Ptr.nhId = int(args[7])
            prfxBucketIdx_Ptr_8 = new_uint32_tp()

            #print('Input Arguments are, devId=%d, vrfId=%d, type=%d, ipv4Addr=%s, ipv6Addr=%s, ipMaskLen=%d, nhEcmpSize=%d, nhId=%d' % (args[0],xpsL3RouteEntry_t_Ptr.vrfId,xpsL3RouteEntry_t_Ptr.type,args[3],args[4],xpsL3RouteEntry_t_Ptr.ipMaskLen,xpsL3RouteEntry_t_Ptr.nhEcmpSize,xpsL3RouteEntry_t_Ptr.nhId))
            ret = xpsL3FindIpRouteLpmEntry(args[0],xpsL3RouteEntry_t_Ptr,prfxBucketIdx_Ptr_8)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3RouteEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3RouteEntry_t_Ptr.type))
                print('ipv4Addr = '),
                args[3] = args[3].replace(".",":").replace(",",":")
                postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                args[4] = args[4].replace(".",":").replace(",",":")
                postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
                listLen = len(postList)
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipMaskLen = %d' % (xpsL3RouteEntry_t_Ptr.ipMaskLen))
                print('nhEcmpSize = %d' % (xpsL3RouteEntry_t_Ptr.nhEcmpSize))
                print('nhId = %d' % (xpsL3RouteEntry_t_Ptr.nhId))
                print('prfxBucketIdx = %d' % (uint32_tp_value(prfxBucketIdx_Ptr_8)))
                pass
            delete_uint32_tp(prfxBucketIdx_Ptr_8)
            delete_xpsL3RouteEntry_tp(xpsL3RouteEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsL3FlushIpRouteEntries
    #/********************************************************************************/
    def do_l3_flush_ip_route_entries(self, arg):
        '''
         xpsL3FlushIpRouteEntries: Enter devId,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter devId,vrfId,type,ipv4Addr,ipv6Addr,ipMaskLen,nhEcmpSize,nhId')
        else:
            args[0] = int(args[0])
            xpsL3RouteEntry_t_Ptr = new_xpsL3RouteEntry_tp()
            xpsL3RouteEntry_t_Ptr.vrfId = int(args[1])
            xpsL3RouteEntry_t_Ptr.type = eval(args[2])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv4Addr[ix] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsL3RouteEntry_t_Ptr.ipv6Addr[ix] = int(postList[ix], 16)
            xpsL3RouteEntry_t_Ptr.ipMaskLen = int(args[5])
            xpsL3RouteEntry_t_Ptr.nhEcmpSize = int(args[6])
            xpsL3RouteEntry_t_Ptr.nhId = int(args[7])

            #print('Input Arguments are, devId=%d, vrfId=%d, type=%d, ipv4Addr=%s, ipv6Addr=%s, ipMaskLen=%d, nhEcmpSize=%d, nhId=%d' % (args[0],xpsL3RouteEntry_t_Ptr.vrfId,xpsL3RouteEntry_t_Ptr.type,args[3],args[4],xpsL3RouteEntry_t_Ptr.ipMaskLen,xpsL3RouteEntry_t_Ptr.nhEcmpSize,xpsL3RouteEntry_t_Ptr.nhId))
            ret = xpsL3FlushIpRouteEntries(args[0],xpsL3RouteEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vrfId = %d' % (xpsL3RouteEntry_t_Ptr.vrfId))
                print('type = %d' % (xpsL3RouteEntry_t_Ptr.type))
                print('ipv4Addr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv4Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipv6Addr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsL3RouteEntry_t_Ptr.ipv6Addr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('ipMaskLen = %d' % (xpsL3RouteEntry_t_Ptr.ipMaskLen))
                print('nhEcmpSize = %d' % (xpsL3RouteEntry_t_Ptr.nhEcmpSize))
                print('nhId = %d' % (xpsL3RouteEntry_t_Ptr.nhId))
                pass
            delete_xpsL3RouteEntry_tp(xpsL3RouteEntry_t_Ptr)

    #/********************************************************************************/
    # command for xpsL3GetNumOfValidIpRouteEntries
    #/********************************************************************************/
    def do_l3_get_num_of_valid_ip_route_entries(self, arg):
        '''
         xpsL3GetNumOfValidIpRouteEntries: Enter [ devId,type ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,type ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            numOfValidEntries_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, type=%d' % (args[0],args[1]))
            ret = xpsL3GetNumOfValidIpRouteEntries(args[0],args[1],numOfValidEntries_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfValidEntries = %d' % (uint32_tp_value(numOfValidEntries_Ptr_2)))
                pass
            delete_uint32_tp(numOfValidEntries_Ptr_2)


    #/********************************************************************************/
    # Auto completion for ipPrefixType enumerations
    #/********************************************************************************/
    def complete_display_route_table(self, text, line, begidx, endidx):
        tempDict = { 2 : 'ipPrefixType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsDisplayRouteTable
    #/********************************************************************************/
    def do_display_route_table(self, arg):
        '''
         xpsDisplayRouteTable: Enter [ devId,type,endIndex ]
         Valid values for type : <ipPrefixType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,type,endIndex ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, type=%d, endIndex=%d' % (args[0],args[1],args[2]))
            ret = xpsDisplayRouteTable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3InitRouteNextHop
    #/********************************************************************************/
    def do_l3_init_route_next_hop(self, arg):
        '''
         xpsL3InitRouteNextHop: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsL3InitRouteNextHop()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3InitRouteNextHopScope
    #/********************************************************************************/
    def do_l3_init_route_next_hop_scope(self, arg):
        '''
         xpsL3InitRouteNextHopScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3InitRouteNextHopScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DeInitRouteNextHop
    #/********************************************************************************/
    def do_l3_de_init_route_next_hop(self, arg):
        '''
         xpsL3DeInitRouteNextHop: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsL3DeInitRouteNextHop()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DeInitRouteNextHopScope
    #/********************************************************************************/
    def do_l3_de_init_route_next_hop_scope(self, arg):
        '''
         xpsL3DeInitRouteNextHopScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsL3DeInitRouteNextHopScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3AddRouteNextHopDevice
    #/********************************************************************************/
    def do_l3_add_route_next_hop_device(self, arg):
        '''
         xpsL3AddRouteNextHopDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsL3AddRouteNextHopDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3RemoveRouteNextHopDevice
    #/********************************************************************************/
    def do_l3_remove_route_next_hop_device(self, arg):
        '''
         xpsL3RemoveRouteNextHopDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsL3RemoveRouteNextHopDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3CreateRouteNextHop
    #/********************************************************************************/
    def do_l3_create_route_next_hop(self, arg):
        '''
         xpsL3CreateRouteNextHop: Enter [ nhEcmpSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ nhEcmpSize ]')
        else:
            args[0] = int(args[0])
            nhId_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, nhEcmpSize=%d' % (args[0]))
            ret = xpsL3CreateRouteNextHop(args[0],nhId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nhId = %d' % (uint32_tp_value(nhId_Ptr_1)))
                pass
            delete_uint32_tp(nhId_Ptr_1)

    #/********************************************************************************/
    # command for xpsL3CreateNextHopGroup
    #/********************************************************************************/
    def do_l3_create_next_hop_group(self, arg):
        '''
         xpsL3CreateNextHopGroup: Enter [ nhEcmpSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ nhEcmpSize ]')
        else:
            args[0] = int(args[0])
            nhId_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, nhEcmpSize=%d' % (args[0]))
            ret = xpsL3CreateNextHopGroup(args[0],nhId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nhGrpId = %d' % (uint32_tp_value(nhId_Ptr_1)))
                pass
            delete_uint32_tp(nhId_Ptr_1)

    #/********************************************************************************/
    # command for xpsL3CreateRouteNextHopScope
    #/********************************************************************************/
    def do_l3_create_route_next_hop_scope(self, arg):
        '''
         xpsL3CreateRouteNextHopScope: Enter [ scopeId,nhEcmpSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,nhEcmpSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            nhId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, nhEcmpSize=%d' % (args[0],args[1]))
            ret = xpsL3CreateRouteNextHopScope(args[0],args[1],nhId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nhId = %d' % (uint32_tp_value(nhId_Ptr_2)))
                pass
            delete_uint32_tp(nhId_Ptr_2)

    #/********************************************************************************/
    # command for xpsL3CreateNextHopGroupScope
    #/********************************************************************************/
    def do_l3_create_next_hop_group_scope(self, arg):
        '''
         xpsL3CreateRouteNextHopScope: Enter [ scopeId,nhEcmpSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,nhEcmpSize ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            nhId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, nhEcmpSize=%d' % (args[0],args[1]))
            ret = xpsL3CreateNextHopGroupScope(args[0],args[1],nhId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nhGrpId = %d' % (uint32_tp_value(nhId_Ptr_2)))
                pass
            delete_uint32_tp(nhId_Ptr_2)

    #/********************************************************************************/
    # command for xpsL3SetRouteNextHop
    #/********************************************************************************/
    def do_l3_set_route_next_hop(self, arg):
        '''
         xpsL3SetRouteNextHop: Enter [ devId,nhId,pktCmd,serviceInstId,vpnLabel,propTTL,l3InterfaceId, macDa, egressIntfId, reasonCode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 10
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,nhId,pktCmd,serviceInstId,vpnLabel,propTTL,l3InterfaceId, macDa, egressIntfId, reasonCode ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsL3NextHopEntry_t_Ptr = new_xpsL3NextHopEntry_tp()
            xpsL3NextHopEntry_t_Ptr.pktCmd = eval(args[2])
            xpsL3NextHopEntry_t_Ptr.serviceInstId = int(args[3])
            xpsL3NextHopEntry_t_Ptr.vpnLabel = int(args[4])
            xpsL3NextHopEntry_t_Ptr.propTTL = int(args[5])
            xpsL3NextHopEntry_t_Ptr.nextHop.l3InterfaceId = int(args[6])
            macList = args[7].split(":")
            for i in range(0,6,1):
                xpsL3NextHopEntry_t_Ptr.nextHop.macDa[i] = int(macList[i], 16)
            xpsL3NextHopEntry_t_Ptr.nextHop.egressIntfId = int(args[8])
            xpsL3NextHopEntry_t_Ptr.reasonCode = int(args[9])

            #print('Input Arguments are, devId=%d, nhId=%d, pktCmd=%d, serviceInstId=%d, propTTL=%d, l3InterfaceId=%d, macDa=%s, egressIntfId=%d' % (args[0],args[1],xpsL3NextHopEntry_t_Ptr.pktCmd,xpsL3NextHopEntry_t_Ptr.serviceInstId,xpsL3NextHopEntry_t_Ptr.propTTL,xpsL3NextHopEntry_t_Ptr.nextHop.l3InterfaceId,args[6],xpsL3NextHopEntry_t_Ptr.nextHop.egressIntfId))
            ret = xpsL3SetRouteNextHop(args[0],args[1],xpsL3NextHopEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pktCmd = %d' % (xpsL3NextHopEntry_t_Ptr.pktCmd))
                print('serviceInstId = %d' % (xpsL3NextHopEntry_t_Ptr.serviceInstId))
                print('vpnLabel = %d' % (xpsL3NextHopEntry_t_Ptr.vpnLabel))
                print('propTTL = %d' % (xpsL3NextHopEntry_t_Ptr.propTTL))
                print('l3InterfaceId = %d' % (xpsL3NextHopEntry_t_Ptr.nextHop.l3InterfaceId))
                print('macDa = %02x:%02x:%02x:%02x:%02x:%02x' % (xpsL3NextHopEntry_t_Ptr.nextHop.macDa[0],xpsL3NextHopEntry_t_Ptr.nextHop.macDa[1],xpsL3NextHopEntry_t_Ptr.nextHop.macDa[2],xpsL3NextHopEntry_t_Ptr.nextHop.macDa[3],xpsL3NextHopEntry_t_Ptr.nextHop.macDa[4],xpsL3NextHopEntry_t_Ptr.nextHop.macDa[5]))
                print('egressIntfId = %d' % (xpsL3NextHopEntry_t_Ptr.nextHop.egressIntfId))
                print('reasonCode = %d' % (xpsL3NextHopEntry_t_Ptr.reasonCode))
                pass
            delete_xpsL3NextHopEntry_tp(xpsL3NextHopEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsL3SetRouteVpnNextHop
    #/********************************************************************************/
    def do_l3_set_route_vpn_next_hop(self, arg):
       '''
        xpsL3SetRouteVpnNextHop: Enter [ devId,nhId,pktCmd,serviceInstId,vpnLabel,propTTL,l3InterfaceId, macDa, egressIntfId, reasonCode ]
       '''
       args = re.split(';| ',arg)
       numArgsReq = 10
       if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
           print('Invalid input, Enter [ devId,nhId,pktCmd,serviceInstId,vpnLabel,propTTL,l3InterfaceId, macDa, egressIntfId, reasonCode ]')
       else:
           args[0] = int(args[0])
           args[1] = int(args[1])
           xpsL3NextHopEntry_t_Ptr = new_xpsL3NextHopEntry_tp()
           xpsL3NextHopEntry_t_Ptr.pktCmd = eval(args[2])
           xpsL3NextHopEntry_t_Ptr.serviceInstId = int(args[3])
           xpsL3NextHopEntry_t_Ptr.vpnLabel = int(args[4])
           xpsL3NextHopEntry_t_Ptr.propTTL = int(args[5])
           xpsL3NextHopEntry_t_Ptr.nextHop.l3InterfaceId = int(args[6])
           macList = args[7].split(":")
           for i in range(0,6,1):
               xpsL3NextHopEntry_t_Ptr.nextHop.macDa[i] = int(macList[i], 16)
           xpsL3NextHopEntry_t_Ptr.nextHop.egressIntfId = int(args[8])
           xpsL3NextHopEntry_t_Ptr.reasonCode = int(args[9])

           #print('Input Arguments are, devId=%d, nhId=%d, pktCmd=%d, serviceInstId=%d, propTTL=%d, l3InterfaceId=%d, macDa=%s, egressIntfId=%d' % (args[0],args[1],xpsL3NextHopEntry_t_Ptr.pktCmd,xpsL3NextHopEntry_t_Ptr.serviceInstId,xpsL3NextHopEntry_t_Ptr.propTTL,xpsL3NextHopEntry_t_Ptr.nextHop.l3InterfaceId,args[6],xpsL3NextHopEntry_t_Ptr.nextHop.egressIntfId))
           ret = xpsL3SetRouteVpnNextHop(args[0],args[1],xpsL3NextHopEntry_t_Ptr)
           err = 0
           if ret != 0:
               print('Return Value = %d' % (ret))
               err = 1
           if err == 0:
               print('pktCmd = %d' % (xpsL3NextHopEntry_t_Ptr.pktCmd))
               print('serviceInstId = %d' % (xpsL3NextHopEntry_t_Ptr.serviceInstId))
               print('vpnLabel = %d' % (xpsL3NextHopEntry_t_Ptr.vpnLabel))
               print('propTTL = %d' % (xpsL3NextHopEntry_t_Ptr.propTTL))
               print('l3InterfaceId = %d' % (xpsL3NextHopEntry_t_Ptr.nextHop.l3InterfaceId))
               print('macDa = %s:%s:%s:%s:%s:%s' % (str(xpsL3NextHopEntry_t_Ptr.nextHop.macDa[0]),str(xpsL3NextHopEntry_t_Ptr.nextHop.macDa[1]),str(xpsL3NextHopEntry_t_Ptr.nextHop.macDa[2]),str(xpsL3NextHopEntry_t_Ptr.nextHop.macDa[3]),str(xpsL3NextHopEntry_t_Ptr.nextHop.macDa[4]),str(xpsL3NextHopEntry_t_Ptr.nextHop.macDa[5])))
               print('egressIntfId = %d' % (xpsL3NextHopEntry_t_Ptr.nextHop.egressIntfId))
               print('reasonCode = %d' % (xpsL3NextHopEntry_t_Ptr.reasonCode))
               pass
           delete_xpsL3NextHopEntry_tp(xpsL3NextHopEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsL3GetRouteNextHop
    #/********************************************************************************/
    def do_l3_get_route_next_hop(self, arg):
        '''
         xpsL3GetRouteNextHop: Enter [ devId,nhId]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,nhId]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsL3NextHopEntry_t_Ptr = new_xpsL3NextHopEntry_tp()

            #print('Input Arguments are, devId=%d, nhId=%d' % (args[0],args[1]))
            ret = xpsL3GetRouteNextHop(args[0],args[1],xpsL3NextHopEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pktCmd = %d' % (xpsL3NextHopEntry_t_Ptr.pktCmd))
                print('serviceInstId = %d' % (xpsL3NextHopEntry_t_Ptr.serviceInstId))
                print('vpnLabel = %d' % (xpsL3NextHopEntry_t_Ptr.vpnLabel))
                print('propTTL = %d' % (xpsL3NextHopEntry_t_Ptr.propTTL))
                print('l3InterfaceId = %d' % (xpsL3NextHopEntry_t_Ptr.nextHop.l3InterfaceId))
                for ix in range(0, 6):
                    print('%02x' % (xpsL3NextHopEntry_t_Ptr.nextHop.macDa[ix])),
                    if ix < (5):
                        sys.stdout.write(':'),
                print('')
                print('egressIntfId = %d' % (xpsL3NextHopEntry_t_Ptr.nextHop.egressIntfId))
                print('reasonCode = %d' % (xpsL3NextHopEntry_t_Ptr.reasonCode))
                pass
            delete_xpsL3NextHopEntry_tp(xpsL3NextHopEntry_t_Ptr)
    #/********************************************************************************/
    # command for xpsL3DestroyRouteNextHop
    #/********************************************************************************/
    def do_l3_destroy_route_next_hop(self, arg):
        '''
         xpsL3DestroyRouteNextHop: Enter [ nhEcmpSize,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ nhEcmpSize,nhId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, nhEcmpSize=%d, nhId=%d' % (args[0],args[1]))
            ret = xpsL3DestroyRouteNextHop(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3DestroyRouteNextHopScope
    #/********************************************************************************/
    def do_l3_destroy_route_next_hop_scope(self, arg):
        '''
         xpsL3DestroyRouteNextHopScope: Enter [ scopeId,nhEcmpSize,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,nhEcmpSize,nhId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, nhEcmpSize=%d, nhId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3DestroyRouteNextHopScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3ClearRouteNextHop
    #/********************************************************************************/
    def do_l3_clear_route_next_hop(self, arg):
        '''
         xpsL3ClearRouteNextHop: Enter [ devId,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,nhId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, nhId=%d' % (args[0],args[1]))
            ret = xpsL3ClearRouteNextHop(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
   #/********************************************************************************/
    # command for xpsL3GetV6McL3DomainIdForInterface
    #/********************************************************************************/
    def do_l3_get_v6_mc_l3_domain_id_for_interface(self, arg):
        '''
         xpsL3GetV6McL3DomainIdForInterface: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            mcL3DomainId_Ptr_2 = new_xpMcastDomainId_tp()
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsL3GetV6McL3DomainIdForInterface(args[0],args[1],mcL3DomainId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcL3DomainId = %d' % (xpMcastDomainId_tp_value(mcL3DomainId_Ptr_2)))
                pass
            delete_xpMcastDomainId_tp(mcL3DomainId_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetV6McL3DomainIdForInterface
    #/********************************************************************************/
    def do_l3_set_v6_mc_l3_domain_id_for_interface(self, arg):
        '''
         xpsL3SetV6McL3DomainIdForInterface: Enter [ devId,intfId,mcL3DomainId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,mcL3DomainId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, intfId=%d, mcL3DomainId=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetV6McL3DomainIdForInterface(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3SetIpv4MtuLenForInterface
    #/********************************************************************************/
    def do_l3_set_v4_mtu_len_for_interface(self, arg):
        '''
         xpsL3SetIpv4MtuLenForInterface: Enter [ devId,intfId,ipv4MtuLen ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,ipv4MtuLen ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, intfId=%d, ipv4MtuLen=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIpv4MtuLenForInterface(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIpv4MtuLenForInterface
    #/********************************************************************************/
    def do_l3_get_v4_mtu_len_for_interface(self, arg):
        '''
         xpsL3GetIpv4MtuLenForInterface: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            ipv4MtuLen_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIpv4MtuLenForInterface(args[0],args[1],ipv4MtuLen_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('ipv4MtuLen = %d' % (uint32_tp_value(ipv4MtuLen_Ptr_2)))
                pass
            delete_uint32_tp(ipv4MtuLen_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIpv6MtuLenForInterface
    #/********************************************************************************/
    def do_l3_set_v6_mtu_len_for_interface(self, arg):
        '''
         xpsL3SetIpv6MtuLenForInterface: Enter [ devId,intfId,ipv6MtuLen ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,ipv6MtuLen ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, intfId=%d, ipv6MtuLen=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIpv6MtuLenForInterface(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIpv6MtuLenForInterface
    #/********************************************************************************/
    def do_l3_get_v6_mtu_len_for_interface(self, arg):
        '''
         xpsL3GetIpv6MtuLenForInterface: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            ipv6MtuLen_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIpv6MtuLenForInterface(args[0],args[1],ipv6MtuLen_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('ipv6MtuLen = %d' % (uint32_tp_value(ipv6MtuLen_Ptr_2)))
                pass
            delete_uint32_tp(ipv6MtuLen_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetMtuPktCmdForInterface
    #/********************************************************************************/
    def do_l3_set_mtu_pktcmd_for_interface(self, arg):
        '''
         xpsL3SetMtuPktCmdForInterface: Enter [ devId,intfId,mtuPktCmd ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,mtuPktCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, intfId=%d, mtuPktCmd=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetMtuPktCmdForInterface(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetMtuPktCmdForInterface
    #/********************************************************************************/
    def do_l3_get_mtu_pktcmd_for_interface(self, arg):
        '''
         xpsL3GetMtuPktCmdForInterface: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            mtuPktCmd_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsL3GetMtuPktCmdForInterface(args[0],args[1],mtuPktCmd_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mtuPktCmd = %d' % (uint32_tp_value(mtuPktCmd_Ptr_2)))
                pass
            delete_uint32_tp(mtuPktCmd_Ptr_2)

    #/********************************************************************************/
    # Auto completion for pktCmd enumerations
    #/********************************************************************************/
    def complete_l3_set_igmp_cmd(self, text, line, begidx, endidx):
        tempDict = { 3 : 'pktCmd'}
        return enumDict.autoCompletion(line, text, tempDict)
    #/********************************************************************************/
    # command for xpsL3GetVlanIdForL3Interface
    #/********************************************************************************/
    def do_l3_get_vlan_id_for_l3_interface(self, arg):
        '''
         xpsL3GetVlanIdForL3Interface: Enter [ intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ intfId ]')
        else:
            args[0] = int(args[0])
            vlanId_Ptr_1 = new_xpsVlan_tp()
            #print('Input Arguments are, intfId=%d' % (args[0]))
            ret = xpsL3GetVlanIdForL3Interface(args[0],vlanId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (xpsVlan_tp_value(vlanId_Ptr_1)))
                pass
            delete_xpsVlan_tp(vlanId_Ptr_1)
    #/********************************************************************************/
    # command for xpsL3GetVlanIdForL3InterfaceScope
    #/********************************************************************************/
    def do_l3_get_vlan_id_for_l3_interface_scope(self, arg):
        '''
         xpsL3GetVlanIdForL3InterfaceScope: Enter [ scopeId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            vlanId_Ptr_2 = new_xpsVlan_tp()
            #print('Input Arguments are, scopeId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsL3GetVlanIdForL3InterfaceScope(args[0],args[1],vlanId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanId = %d' % (xpsVlan_tp_value(vlanId_Ptr_2)))
                pass
            delete_xpsVlan_tp(vlanId_Ptr_2)

    #/********************************************************************************/
    # command for xpsL3SetIgmpCmd
    #/********************************************************************************/
    def do_l3_set_igmp_cmd(self, arg):
        '''
         xpsL3SetIgmpCmd: Enter [ devId,l3IntfId,pktCmd ]
         Valid values for pktCmd : <pktCmd>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,pktCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, pktCmd=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIgmpCmd(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIgmpCmd
    #/********************************************************************************/
    def do_l3_get_igmp_cmd(self, arg):
        '''
         xpsL3GetIgmpCmd: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pktCmd_Ptr_2 = new_xpPktCmd_ep()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIgmpCmd(args[0],args[1],pktCmd_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pktCmd = %d' % (xpPktCmd_ep_value(pktCmd_Ptr_2)))
                pass
            delete_xpPktCmd_ep(pktCmd_Ptr_2)

    #/********************************************************************************/
    # Auto completion for pktCmd enumerations
    #/********************************************************************************/
    def complete_l3_set_icmpv6_cmd(self, text, line, begidx, endidx):
        tempDict = { 3 : 'pktCmd'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsL3SetIcmpv6Cmd
    #/********************************************************************************/
    def do_l3_set_icmpv6_cmd(self, arg):
        '''
         xpsL3SetIcmpv6Cmd: Enter [ devId,l3IntfId,pktCmd ]
         Valid values for pktCmd : <pktCmd>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId,pktCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, l3IntfId=%d, pktCmd=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetIcmpv6Cmd(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
     #/********************************************************************************/
    # command for xpsL3CreateRouteNextHopWithId
    #/********************************************************************************/
    def do_l3_create_route_next_hop_with_id(self, arg):
        '''
         xpsL3CreateRouteNextHopWithId: Enter [ nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ nhId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, nhId=%d' % (args[0]))
            ret = xpsL3CreateRouteNextHopWithId(args[0])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nhId = %d' % (args[0]))
                pass
    #/********************************************************************************/
    # command for xpsL3CreateRouteNextHopWithIdScope
    #/********************************************************************************/
    def do_l3_create_route_next_hop_with_id_scope(self, arg):
        '''
         xpsL3CreateRouteNextHopWithIdScope: Enter [ scopeId,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,nhId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, nhId=%d' % (args[0],args[1]))
            ret = xpsL3CreateRouteNextHopWithIdScope(args[0],args[1])
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nhId = %d' % (args[1]))
                pass
    #/********************************************************************************/
    # command for xpsL3GetIcmpv6Cmd
    #/********************************************************************************/
    def do_l3_get_icmpv6_cmd(self, arg):
        '''
         xpsL3GetIcmpv6Cmd: Enter [ devId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            pktCmd_Ptr_2 = new_xpPktCmd_ep()
            #print('Input Arguments are, devId=%d, l3IntfId=%d' % (args[0],args[1]))
            ret = xpsL3GetIcmpv6Cmd(args[0],args[1],pktCmd_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pktCmd = %d' % (xpPktCmd_ep_value(pktCmd_Ptr_2)))
                pass
            delete_xpPktCmd_ep(pktCmd_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3SetIpv4RehashLevel
    #/********************************************************************************/
    def do_l3_set_ipv4_rehash_level(self, arg):
        '''
         xpsL3SetIpv4RehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsL3SetIpv4RehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIpv4RehashLevel
    #/********************************************************************************/
    def do_l3_get_ipv4_rehash_level(self, arg):
        '''
         xpsL3GetIpv4RehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsL3GetIpv4RehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)
    #/********************************************************************************/
    # command for xpsL3SetIpv6RehashLevel
    #/********************************************************************************/
    def do_l3_set_ipv6_rehash_level(self, arg):
        '''
         xpsL3SetIpv6RehashLevel: Enter [ devId,numOfRehashLevels ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numOfRehashLevels ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, numOfRehashLevels=%d' % (args[0],args[1]))
            ret = xpsL3SetIpv6RehashLevel(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsL3GetIpv6RehashLevel
    #/********************************************************************************/
    def do_l3_get_ipv6_rehash_level(self, arg):
        '''
         xpsL3GetIpv6RehashLevel: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            numOfRehashLevels_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsL3GetIpv6RehashLevel(args[0],numOfRehashLevels_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfRehashLevels = %d' % (uint8_tp_value(numOfRehashLevels_Ptr_1)))
                pass
            delete_uint8_tp(numOfRehashLevels_Ptr_1)

    #/********************************************************************************/
    # Auto completion for ipPrefixType enumerations
    #/********************************************************************************/
    def complete_l3_set_num_rehash_level(self, text, line, begidx, endidx):
        tempDict = { 2 : 'ipPrefixType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsL3SetNumRehashLevel
    #/********************************************************************************/
    def do_l3_set_num_rehash_level(self, arg):
        '''
         xpsL3SetNumRehashLevel: Enter [ devId,prefixType,numRehashLevel ]
         Valid values for prefixType : <ipPrefixType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,prefixType,numRehashLevel ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, prefixType=%d, numRehashLevel=%d' % (args[0],args[1],args[2]))
            ret = xpsL3SetNumRehashLevel(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for ipPrefixType enumerations
    #/********************************************************************************/
    def complete_l3_get_num_rehash_level(self, text, line, begidx, endidx):
        tempDict = { 2 : 'ipPrefixType'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsL3GetNumRehashLevel
    #/********************************************************************************/
    def do_l3_get_num_rehash_level(self, arg):
        '''
         xpsL3GetNumRehashLevel: Enter [ devId,prefixType ]
         Valid values for prefixType : <ipPrefixType>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,prefixType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            numRehashLevel_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, prefixType=%d' % (args[0],args[1]))
            ret = xpsL3GetNumRehashLevel(args[0],args[1],numRehashLevel_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numRehashLevel = %d' % (uint32_tp_value(numRehashLevel_Ptr_2)))
                pass
            delete_uint32_tp(numRehashLevel_Ptr_2)

    #/********************************************************************************/
    # command for xpsL3Ipv4HostGetTableSize
    #/********************************************************************************/
    def do_l3_ipv4_host_get_table_size(self, arg):
        '''
         xpsL3Ipv4HostGetTableSize: Enter [ devId]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId]')
        else:
            args[0] = int(args[0])
            tableSize = new_uint32_tp()
            ret = xpsL3Ipv4HostGetTableSize(args[0],tableSize)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('ipv4 host table size = %d' % (uint32_tp_value(tableSize)))
                pass
            delete_uint32_tp(tableSize)

    #/********************************************************************************/
    # command for xpsL3Ipv6HostGetTableSize
    #/********************************************************************************/
    def do_l3_ipv6_host_get_table_size(self, arg):
        '''
         xpsL3Ipv6HostGetTableSize: Enter [ devId]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId]')
        else:
            args[0] = int(args[0])
            tableSize = new_uint32_tp()
            ret = xpsL3Ipv6HostGetTableSize(args[0],tableSize)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('ipv6 host table size = %d' % (uint32_tp_value(tableSize)))
                pass
            delete_uint32_tp(tableSize)

    #/********************************************************************************/
    # command for xpsL3Ipv4HostClearBucketState
    #/********************************************************************************/
    def do_l3_ipv4_Host_clear_bucket_state(self, arg):
        '''
         xpsL3Ipv4HostClearBucketState: Enter [ devId,tblCopyIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tblCopyIdx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            ret = xpsL3Ipv4HostClearBucketState(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsL3Ipv6HostClearBucketState
    #/********************************************************************************/
    def do_l3_ipv6_host_clear_bucket_state(self, arg):
        '''
         xpsL3Ipv6HostClearBucketState: Enter [ devId,tblCopyIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tblCopyIdx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            ret = xpsL3Ipv6HostClearBucketState(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsL3Ipv4RouteClearBucketState
    #/********************************************************************************/
    def do_l3_ipv4_route_clear_bucket_State(self, arg):
        '''
         xpsL3Ipv4RouteClearBucketState: Enter [ devId,tblCopyIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tblCopyIdx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            ret = xpsL3Ipv4RouteClearBucketState(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsL3Ipv6RouteClearBucketState
    #/********************************************************************************/
    def do_l3_ipv6_route_clear_bucket_state(self, arg):
        '''
         xpsL3Ipv6RouteClearBucketState: Enter [ devId,tblCopyIdx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tblCopyIdx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            ret = xpsL3Ipv6RouteClearBucketState(args[0],args[1])
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

