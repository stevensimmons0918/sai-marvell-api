#!/usr/bin/env python
#  xpsLag.py
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
from LagHash import *
#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *
from enumDict import enumDictionaries


#/**********************************************************************************/
# The class object for xpsLag operations
#/**********************************************************************************/

class xpsLagObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsLagInit
    #/********************************************************************************/
    def do_lag_init(self, arg):
        '''
         xpsLagInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsLagInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagInitScope
    #/********************************************************************************/
    def do_lag_init_scope(self, arg):
        '''
         xpsLagInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsLagInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagAddDevice
    #/********************************************************************************/
    def do_lag_add_device(self, arg):
        '''
         xpsLagAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsLagAddDevice(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagRemoveDevice
    #/********************************************************************************/
    def do_lag_remove_device(self, arg):
        '''
         xpsLagRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsLagRemoveDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagDeInit
    #/********************************************************************************/
    def do_lag_de_init(self, arg):
        '''
         xpsLagDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsLagDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagDeInitScope
    #/********************************************************************************/
    def do_lag_de_init_scope(self, arg):
        '''
         xpsLagDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsLagDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagCreate
    #/********************************************************************************/
    def do_lag_create(self, arg):
        '''
         xpsLagCreate: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            lagIntf_Ptr_0 = new_xpsInterfaceId_tp()
            #print('Input Arguments are' % ())
            ret = xpsLagCreate(lagIntf_Ptr_0)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('lagIntf = %d' % (xpsInterfaceId_tp_value(lagIntf_Ptr_0)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(lagIntf_Ptr_0)))
                pass
            delete_xpsInterfaceId_tp(lagIntf_Ptr_0)

    #/********************************************************************************/
    # command for xpsLagCreateScope
    #/********************************************************************************/
    def do_lag_create_scope(self, arg):
        '''
         xpsLagCreateScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            lagIntf_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsLagCreateScope(args[0],lagIntf_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lagIntf = %d' % (xpsInterfaceId_tp_value(lagIntf_Ptr_1)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(lagIntf_Ptr_1))
                pass
            delete_xpsInterfaceId_tp(lagIntf_Ptr_1)

    #/********************************************************************************/
    # command for xpsLagDestroy
    #/********************************************************************************/
    def do_lag_destroy(self, arg):
        '''
         xpsLagDestroy: Enter [ lagIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ lagIntf ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, lagIntf=%d' % (args[0]))
            ret = xpsLagDestroy(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagDestroyScope
    #/********************************************************************************/
    def do_lag_destroy_scope(self, arg):
        '''
         xpsLagDestroyScope: Enter [ scopeId,lagIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,lagIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, lagIntf=%d' % (args[0],args[1]))
            ret = xpsLagDestroyScope(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagAddPort
    #/********************************************************************************/
    def do_lag_add_port(self, arg):
        '''
         xpsLagAddPort: Enter [ lagIntf,portIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ lagIntf,portIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, lagIntf=%d, portIntf=%d' % (args[0],args[1]))
            ret = xpsLagAddPort(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagAddPortScope
    #/********************************************************************************/
    def do_lag_add_port_scope(self, arg):
        '''
         xpsLagAddPortScope: Enter [ scopeId,lagIntf,portIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,lagIntf,portIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, lagIntf=%d, portIntf=%d' % (args[0],args[1],args[2]))
            ret = xpsLagAddPortScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagRemovePort
    #/********************************************************************************/
    def do_lag_remove_port(self, arg):
        '''
         xpsLagRemovePort: Enter [ lagIntf,portIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ lagIntf,portIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, lagIntf=%d, portIntf=%d' % (args[0],args[1]))
            ret = xpsLagRemovePort(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagRemovePortScope
    #/********************************************************************************/
    def do_lag_remove_port_scope(self, arg):
        '''
         xpsLagRemovePortScope: Enter [ scopeId,lagIntf,portIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,lagIntf,portIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, lagIntf=%d, portIntf=%d' % (args[0],args[1],args[2]))
            ret = xpsLagRemovePortScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagGetIngressPortIntfList
    #/********************************************************************************/
    def do_lag_get_ingress_port_intf_list(self, arg):
        '''
         xpsLagGetIngressPortIntfList: Enter [ scopeId,lagIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,lagIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsLagPortIntfList_t_Ptr = new_xpsLagPortIntfList_tp()

            #print('Input Arguments are, scopeId=%d, lagIntf=%d' % (args[0],args[1]))
            ret = xpsLagGetIngressPortIntfList(args[0],args[1],xpsLagPortIntfList_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('size = %d' % (xpsLagPortIntfList_t_Ptr.size))
                print('portIntfList = '),
                for i in range(0, xpsLagPortIntfList_t_Ptr.size):
                    print xpsLagPortIntfList_t_Ptr.portIntf[i],
                print('')
                pass
            delete_xpsLagPortIntfList_tp(xpsLagPortIntfList_t_Ptr)

    #/********************************************************************************/
    # command for xpsLagGetEgressPortIntfList
    #/********************************************************************************/
    def do_lag_get_egress_port_intf_list(self, arg):
        '''
         xpsLagGetEgressPortIntfList: Enter [ scopeId,lagIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,lagIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsLagPortIntfList_t_Ptr = new_xpsLagPortIntfList_tp()

            #print('Input Arguments are, scopeId=%d, lagIntf=%d' % (args[0],args[1]))
            ret = xpsLagGetEgressPortIntfList(args[0],args[1],xpsLagPortIntfList_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('size = %d' % (xpsLagPortIntfList_t_Ptr.size))
                print('portIntfList = '),
                for i in range(0, xpsLagPortIntfList_t_Ptr.size):
                    print xpsLagPortIntfList_t_Ptr.portIntf[i],
                print('')
                pass
            delete_xpsLagPortIntfList_tp(xpsLagPortIntfList_t_Ptr)

    #/********************************************************************************/
    # command for xpsLagGetPortIntfList
    #/********************************************************************************/
    def do_lag_get_port_intf_list(self, arg):
        '''
         xpsLagGetPortIntfList: Enter [ lagIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ lagIntf ]')
        else:
            args[0] = int(args[0])
            xpsLagPortIntfList_t_Ptr = new_xpsLagPortIntfList_tp()
            #print('Input Arguments are, lagIntf=%d' % (args[0]))
            ret = xpsLagGetPortIntfList(args[0],xpsLagPortIntfList_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('size = %d' % (xpsLagPortIntfList_t_Ptr.size))
                print('portIntfList = '),
                for i in range(0, xpsLagPortIntfList_t_Ptr.size):
                    print xpsLagPortIntfList_t_Ptr.portIntf[i],
                
                print ('')
                pass
            delete_xpsLagPortIntfList_tp(xpsLagPortIntfList_t_Ptr)

    #/********************************************************************************/
    # command for xpsLagGetPortIntfListScope
    #/********************************************************************************/
    def do_lag_get_port_intf_list_scope(self, arg):
        '''
         xpsLagGetPortIntfListScope: Enter [ scopeId,lagIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,lagIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsLagPortIntfList_t_Ptr = new_xpsLagPortIntfList_tp()
            #print('Input Arguments are, scopeId=%d, lagIntf=%d, ' % (args[0],args[1]))
            ret = xpsLagGetPortIntfListScope(args[0],args[1],xpsLagPortIntfList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('size = %d' % (xpsLagPortIntfList_t_Ptr.size))
                print('portIntfList = '),
                for i in range(0, xpsLagPortIntfList_t_Ptr.size):
                    print xpsLagPortIntfList_t_Ptr.portIntf[i],

                print('')
                pass
            delete_xpsLagPortIntfList_tp(xpsLagPortIntfList_t_Ptr)
    #/********************************************************************************/
    # command for xpsLagGetIngressOperationalState
    #/********************************************************************************/
    def do_lag_get_ingress_operational_state(self, arg):
        '''
         xpsLagGetIngressOperationalState: Enter [ devId,lagIntf,portIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntf,portIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            state_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, lagIntf=%d, portIntf=%d' % (args[0],args[1],args[2]))
            ret = xpsLagGetIngressOperationalState(args[0],args[1],args[2],state_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('state = %d' % (uint32_tp_value(state_Ptr_3)))
                pass
            delete_uint32_tp(state_Ptr_3)
    #/********************************************************************************/
    # command for xpsLagIngressEnablePort
    #/********************************************************************************/
    def do_lag_ingress_enable_port(self, arg):
        '''
         xpsLagIngressEnablePort: Enter [ devId,lagIntf,portIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntf,portIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, lagIntf=%d, portIntf=%d' % (args[0],args[1],args[2]))
            ret = xpsLagIngressEnablePort(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsLagIngressDisablePort
    #/********************************************************************************/
    def do_lag_ingress_disable_port(self, arg):
        '''
         xpsLagIngressDisablePort: Enter [ devId,lagIntf,portIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntf,portIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, lagIntf=%d, portIntf=%d' % (args[0],args[1],args[2]))
            ret = xpsLagIngressDisablePort(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsLagGetEgressOperationalState
    #/********************************************************************************/
    def do_lag_get_egress_operational_state(self, arg):
        '''
         xpsLagGetEgressOperationalState: Enter [ devId,lagIntf,portIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntf,portIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            state_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, lagIntf=%d, portIntf=%d' % (args[0],args[1],args[2]))
            ret = xpsLagGetEgressOperationalState(args[0],args[1],args[2],state_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('state = %d' % (uint32_tp_value(state_Ptr_3)))
                pass
            delete_uint32_tp(state_Ptr_3)
    #/********************************************************************************/
    # command for xpsLagEgressEnablePort
    #/********************************************************************************/
    def do_lag_egress_enable_port(self, arg):
        '''
         xpsLagEgressEnablePort: Enter [ devId,lagIntf,portIntf,autoEnable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntf,portIntf,autoEnable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, lagIntf=%d, portIntf=%d, autoEnable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsLagEgressEnablePort(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsLagEgressDisablePort
    #/********************************************************************************/
    def do_lag_egress_disable_port(self, arg):
        '''
         xpsLagEgressDisablePort: Enter [ devId,lagIntf,portIntf,autoEnable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntf,portIntf,autoEnable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, lagIntf=%d, portIntf=%d, autoEnable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsLagEgressDisablePort(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsLagDeployIngress
    #/********************************************************************************/
    def do_lag_deploy_ingress(self, arg):
        '''
         xpsLagDeployIngress: Enter [ devId,lagIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, lagIntf=%d' % (args[0],args[1]))
            ret = xpsLagDeployIngress(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagDeployEgress
    #/********************************************************************************/
    def do_lag_deploy_egress(self, arg):
        '''
         xpsLagDeployEgress: Enter [ devId,lagIntf,autoEnable,isRedirected ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntf,autoEnable,isRedirected ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, lagIntf=%d, autoEnable=%d' % (args[0],args[1],args[2]))
            ret = xpsLagDeployEgress(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagDeploy
    #/********************************************************************************/
    def do_lag_deploy(self, arg):
        '''
         xpsLagDeploy: Enter [ devId,lagIntf,autoEnable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntf,autoEnable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, lagIntf=%d, autoEnable=%d' % (args[0],args[1],args[2]))
            ret = xpsLagDeploy(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagReadDistributionTable
    #/********************************************************************************/
    def do_lag_read_distribution_table(self, arg):
        '''
         xpsLagReadDistributionTable: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            tableBuffer_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsLagReadDistributionTable(args[0],tableBuffer_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('tableBuffer = %d' % (uint8_tp_value(tableBuffer_Ptr_1)))
                pass
            delete_uint8_tp(tableBuffer_Ptr_1)

    #/********************************************************************************/
    # command for xpsLagWriteDistributionTable
    #/********************************************************************************/
    def do_lag_write_distribution_table(self, arg):
        '''
         xpsLagWriteDistributionTable: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            tableBuffer_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsLagWriteDistributionTable(args[0],tableBuffer_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('tableBuffer = %d' % (uint8_tp_value(tableBuffer_Ptr_1)))
                pass
            delete_uint8_tp(tableBuffer_Ptr_1)

    #/********************************************************************************/
    # command for xpsLagAddPortToDistributionEntry
    #/********************************************************************************/
    def do_lag_add_port_to_distribution_entry(self, arg):
        '''
         xpsLagAddPortToDistributionEntry: Enter [ devId,idx,portListSize,portList ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,idx,portListSize,portList ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsPortList_t_Ptr = new_xpsPortList_tp()
            xpsPortList_t_Ptr.size = int(args[2])

            pList = str(args[3])
            portList = []
            for port in pList.split(','):
                port = port.replace('[','').replace(']','').strip()
                if '-' in port:
                    startIdx = int(port.split('-')[0].strip())
                    endIdx = int(port.split('-')[1].strip())
                    if startIdx > endIdx:
                        print('Improper Range')
                        return
                    for port1 in range (startIdx, endIdx+1):
                        portList.append(int(port1))
                else:
                    portList.append(int(port))

            if len(portList) != xpsPortList_t_Ptr.size:
               err = 1
               print('Invalid Arguments')
            else:
               for i in range (0, xpsPortList_t_Ptr.size):
                   xpsPortList_t_Ptr.portList[i] = portList[i]
               #print('Input Arguments are, devId=%d, idx=%d portListSize=%d' % args[0],args[1], args[2])
               ret = xpsLagAddPortToDistributionEntry(args[0],args[1],xpsPortList_t_Ptr)
               err = 0
               if ret != 0:
                   print('Return Value = %d' % (ret))
                   err = 1
               if err == 0:
                   print('size = %d' % (xpsPortList_t_Ptr.size))
                   print"portList = ",
                   for i in range (0, xpsPortList_t_Ptr.size):
                       print xpsPortList_t_Ptr.portList[i],
                   print('')
            delete_xpsPortList_tp(xpsPortList_t_Ptr)

    #/********************************************************************************/
    # command for xpsLagRemovePortFromDistributionEntry
    #/********************************************************************************/
    def do_lag_remove_port_from_distribution_entry(self, arg):
        '''
         xpsLagRemovePortFromDistributionEntry: Enter [ devId,idx,portListSize,portList ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,idx,portListSize,portList ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsPortList_t_Ptr = new_xpsPortList_tp()
            xpsPortList_t_Ptr.size = int(args[2])

            pList = str(args[3])
            portList = []
            for port in pList.split(','):
                port = port.replace('[','').replace(']','').strip()
                if '-' in port:
                    startIdx = int(port.split('-')[0].strip())
                    endIdx = int(port.split('-')[1].strip())
                    if startIdx > endIdx:
                        print('Improper Range')
                        return
                    for port1 in range (startIdx, endIdx+1):
                        portList.append(int(port1))
                else:
                    portList.append(int(port))
            if len(portList) != xpsPortList_t_Ptr.size:
                err = 1
                print('Invalid Arguments')
            else:
                for i in range (0, xpsPortList_t_Ptr.size):
                    xpsPortList_t_Ptr.portList[i] = portList[i]
                #print('Input Arguments are, devId=%d, idx=%d portListSize=%d' % args[0],args[1], args[2])
                ret = xpsLagRemovePortFromDistributionEntry(args[0],args[1],xpsPortList_t_Ptr)
                err = 0
                if ret != 0:
                    print('Return Value = %d' % (ret))
                    err = 1
                if err == 0:
                    print('size = %d' % (xpsPortList_t_Ptr.size))
                    print"portList = ",
                    for i in range (0, xpsPortList_t_Ptr.size):
                        print xpsPortList_t_Ptr.portList[i],
                    print('')
            delete_xpsPortList_tp(xpsPortList_t_Ptr)

    #/********************************************************************************/
    # command for xpsLagReadDistributionTableEntry
    #/********************************************************************************/
    def do_lag_read_distribution_table_entry(self, arg):
        '''
         xpsLagReadDistributionTableEntry: Enter [ devId,idx ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,idx ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsPortList_t_Ptr = new_xpLagDistributionEntry_tp()
            #print('Input Arguments are, devId=%d, idx=%d' % (args[0],args[1]))
            ret = xpsLagReadDistributionTableEntry(args[0],args[1],xpsPortList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('portMaskSize = %d' % (xpsPortList_t_Ptr.portMaskSize))
                portMask = ""
                for i in range (0, ((xpsPortList_t_Ptr.portMaskSize)/8)):
                    portMask = portMask + str('0x%0.2x,' % (uint8Arr_tp_getitem(xpsPortList_t_Ptr.portMask,i)))
                print 'portMask = [',portMask.rstrip(','),']'
                print('')
            delete_xpLagDistributionEntry_tp(xpsPortList_t_Ptr)

    #/********************************************************************************/
    # command for xpsLagWriteDistributionTableEntry
    #/********************************************************************************/
    def do_lag_write_distribution_table_entry(self, arg):
        '''
         xpsLagWriteDistributionTableEntry: Enter [ devId,idx,portMaskSize,portMask ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,idx,portMaskSize,portMask ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsPortList_t_Ptr = new_xpLagDistributionEntry_tp()
            xpsPortList_t_Ptr.portMaskSize = int(args[2])

            pList = eval(args[3])
            if len(pList) != (xpsPortList_t_Ptr.portMaskSize/8):
                err = 1
                print('Invalid Arguments')
            else:
                for i, value in enumerate(pList):
                    uint8Arr_tp_setitem(xpsPortList_t_Ptr.portMask, i, int(value))
                #print('Input Arguments are, devId=%d, idx=%d' % (args[0],args[1]))
                ret = xpsLagWriteDistributionTableEntry(args[0],args[1],xpsPortList_t_Ptr)
                err = 0
                if ret != 0:
                    print('Return Value = %d' % (ret))
                    err = 1
                if err == 0:
                    print('portMaskSize = %d' % (xpsPortList_t_Ptr.portMaskSize))
                    portMask = ""
                    for i in range (0, ((xpsPortList_t_Ptr.portMaskSize)/8)):
                        portMask = portMask + str('0x%0.2x,' % (uint8Arr_tp_getitem(xpsPortList_t_Ptr.portMask,i)))
                    print 'portMask = [',portMask.rstrip(','),']'
                    print('')
            delete_xpLagDistributionEntry_tp(xpsPortList_t_Ptr)

    #/********************************************************************************/
    # command for xpsLagGetDistributionTableEntryWidth
    #/********************************************************************************/
    def do_lag_get_distribution_table_entry_width(self, arg):
        '''
         xpsLagGetDistributionTableEntryWidth: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            entryWidthBits_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsLagGetDistributionTableEntryWidth(args[0],entryWidthBits_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('entryWidthBits = %d' % (uint32_tp_value(entryWidthBits_Ptr_1)))
                pass
            delete_uint32_tp(entryWidthBits_Ptr_1)

    #/********************************************************************************/
    # command for xpsLagGetDistributionTableDepth
    #/********************************************************************************/
    def do_lag_get_distribution_table_depth(self, arg):
        '''
         xpsLagGetDistributionTableDepth: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            tableDepth_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsLagGetDistributionTableDepth(args[0],tableDepth_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tableDepth = %d' % (uint32_tp_value(tableDepth_Ptr_1)))
                pass
            delete_uint32_tp(tableDepth_Ptr_1)

    #/********************************************************************************/
    # command for xpsLagIsPortMember
    #/********************************************************************************/
    def do_lag_is_port_member(self, arg):
        '''
         xpsLagIsPortMember: Enter [ devId,port,lagIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,lagIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            isMember_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d, lagIntf=%d' % (args[0],args[1],args[2]))
            ret = xpsLagIsPortMember(args[0],args[1],args[2],isMember_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('isMember = %d' % (uint32_tp_value(isMember_Ptr_3)))
                pass
            delete_uint32_tp(isMember_Ptr_3)

    #/********************************************************************************/
    # command for xpsLagIsPortIntfMember
    #/********************************************************************************/
    def do_lag_is_port_intf_member(self, arg):
        '''
         xpsLagIsPortIntfMember: Enter [ portIntf,lagIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIntf,lagIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            isMember_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, portIntf=%d, lagIntf=%d' % (args[0],args[1]))
            ret = xpsLagIsPortIntfMember(args[0],args[1],isMember_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('isMember = %d' % (uint32_tp_value(isMember_Ptr_2)))
                pass
            delete_uint32_tp(isMember_Ptr_2)

    #/********************************************************************************/
    # command for xpsLagIsPortIntfMemberScope
    #/********************************************************************************/
    def do_lag_is_port_intf_member_scope(self, arg):
        '''
         xpsLagIsPortIntfMemberScope: Enter [ scopeId,portIntf,lagIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIntf,lagIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            isMember_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, portIntf=%d, lagIntf=%d' % (args[0],args[1],args[2]))
            ret = xpsLagIsPortIntfMemberScope(args[0],args[1],args[2],isMember_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('isMember = %d' % (uint32_tp_value(isMember_Ptr_3)))
                pass
            delete_uint32_tp(isMember_Ptr_3)

    #/********************************************************************************/
    # Auto completion for hasField Enums
    #/********************************************************************************/
    def complete_lag_set_hash_fields(self, text, line, begidx, endidx):
        return hashFieldCompletion(text)
    #/*****************************************************************************************************/
    # command for setting Hash field
    #/*****************************************************************************************************/
    def do_lag_set_hash_fields(self, arg):
        '''
         xpsLagSetHashFields: Enter [ deviceId, hashList ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,hashList ]')
        else:
            args[0] = int(args[0])
            postList = args[1].strip("]").strip("[").split(",")
            listLen = len(postList)
            for hashFieldEnum in postList:
                hashFieldEnum = hashFieldEnum.strip()
                if hashFieldEnum not in enumDictionaries['hashField']:
                    print "invalid data %s"%(hashFieldEnum)
                    return
            #print('Input Arguments are, deviceId=%d hashList %s' % (args[0], args[1]))
            fields_Ptr_1 = new_xpHashField_arr(listLen)
            for i in range(0, listLen):
                xpHashField_arr_setitem(fields_Ptr_1, i, eval(postList[i]))
            ret = xpsLagSetHashFields(args[0],fields_Ptr_1,listLen)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
            delete_xpHashField_arr(fields_Ptr_1)
    #/********************************************************************************/
    # command for xpsLagGetHashFields
    #/********************************************************************************/
    def do_lag_get_hash_fields(self, arg):
        '''
         xpsLagGetHashFields: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            xpHashField_t_Ptr = new_xpHashField_arr(264)
            size_t_Ptr = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsLagGetHashFields(args[0],xpHashField_t_Ptr,size_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('size = %d' % uint32_tp_value(size_t_Ptr))
                print('lagHashFields = \n'),
                for i in range(0,uint32_tp_value(size_t_Ptr)):
                    print xpHashField_arr_getitem(xpHashField_t_Ptr, i)
                print ('')
                pass
            delete_xpHashField_arr(xpHashField_t_Ptr)
            delete_uint32_tp(size_t_Ptr)
    #/********************************************************************************/
    # command for xpsLagSetLagHashPolynomial
    #/********************************************************************************/
    def do_lag_set_lag_hash_polynomial(self, arg):
        '''
         xpsLagSetLagHashPolynomial: Enter [ deviceId,instance,polynomialId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,polynomialId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, deviceId=%d, instance=%d, polynomialId=%d' % (args[0],args[1],args[2]))
            ret = xpsLagSetLagHashPolynomial(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagGetLagHashPolynomial
    #/********************************************************************************/
    def do_lag_get_lag_hash_polynomial(self, arg):
        '''
         xpsLagGetLagHashPolynomial: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            polynomialId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsLagGetLagHashPolynomial(args[0],args[1],polynomialId_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('polynomialId = %d' % (uint32_tp_value(polynomialId_Ptr_2)))
                pass
            delete_uint32_tp(polynomialId_Ptr_2)

    #/********************************************************************************/
    # command for xpsLagSetLagHashPolynomialSeed
    #/********************************************************************************/
    def do_lag_set_lag_hash_polynomial_seed(self, arg):
        '''
         xpsLagSetLagHashPolynomialSeed: Enter [ deviceId,instance,seed ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,seed ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, deviceId=%d, instance=%d, seed=%d' % (args[0],args[1],args[2]))
            ret = xpsLagSetLagHashPolynomialSeed(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagGetLagHashPolynomialSeed
    #/********************************************************************************/
    def do_lag_get_lag_hash_polynomial_seed(self, arg):
        '''
         xpsLagGetLagHashPolynomialSeed: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            seed_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsLagGetLagHashPolynomialSeed(args[0],args[1],seed_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('seed = %d' % (uint32_tp_value(seed_Ptr_2)))
                pass
            delete_uint32_tp(seed_Ptr_2)

    #/********************************************************************************/
    # command for xpsLagEnableMirroring
    #/********************************************************************************/
    def do_lag_enable_mirroring(self, arg):
        '''
         xpsLagEnableMirroring: Enter [ devId,lagIntf,analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntf,analyzerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, lagIntf=%d, analyzerId=%d' % (args[0],args[1],args[2]))
            ret = xpsLagEnableMirroring(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagDisableMirroring
    #/********************************************************************************/
    def do_lag_disable_mirroring(self, arg):
        '''
         xpsLagDisableMirroring: Enter [ devId,lagIntf,analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntf,analyzerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, lagIntf=%d, analyzerId=%d' % (args[0],args[1],args[2]))
            ret = xpsLagDisableMirroring(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagGetFirst
    #/********************************************************************************/
    def do_lag_get_first(self, arg):
        '''
         xpsLagGetFirst: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            lagIntf_Ptr_0 = new_xpsInterfaceId_tp()
            #print('Input Arguments are' % ())
            ret = xpsLagGetFirst(lagIntf_Ptr_0)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('lagIntf = %d' % (xpsInterfaceId_tp_value(lagIntf_Ptr_0)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(lagIntf_Ptr_0)))
                pass
            delete_xpsInterfaceId_tp(lagIntf_Ptr_0)

    #/********************************************************************************/
    # command for xpsLagGetFirstScope
    #/********************************************************************************/
    def do_lag_get_first_scope(self, arg):
        '''
         xpsLagGetFirstScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            lagIntf_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsLagGetFirstScope(args[0],lagIntf_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lagIntf = %d' % (xpsInterfaceId_tp_value(lagIntf_Ptr_1)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(lagIntf_Ptr_1))
                pass
            delete_xpsInterfaceId_tp(lagIntf_Ptr_1)

    #/********************************************************************************/
    # command for xpsLagGetNext
    #/********************************************************************************/
    def do_lag_get_next(self, arg):
        '''
         xpsLagGetNext: Enter [ lagIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ lagIntf ]')
        else:
            args[0] = int(args[0])
            lagIntfNext_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, lagIntf=%d' % (args[0]))
            ret = xpsLagGetNext(args[0],lagIntfNext_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('lagIntfNext = %d' % (xpsInterfaceId_tp_value(lagIntfNext_Ptr_1)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(lagIntfNext_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(lagIntfNext_Ptr_1)

    #/********************************************************************************/
    # command for xpsLagGetNextScope
    #/********************************************************************************/
    def do_lag_get_next_scope(self, arg):
        '''
         xpsLagGetNextScope: Enter [ scopeId,lagIntf ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,lagIntf ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            lagIntfNext_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, lagIntf=%d' % (args[0],args[1]))
            ret = xpsLagGetNextScope(args[0],args[1],lagIntfNext_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lagIntfNext = %d' % (xpsInterfaceId_tp_value(lagIntfNext_Ptr_2)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(lagIntfNext_Ptr_2))
                pass
            delete_xpsInterfaceId_tp(lagIntfNext_Ptr_2)

    #/********************************************************************************/
    # command for xpsLagSetHashAlgo
    #/********************************************************************************/
    def do_lag_set_hash_algo(self, arg):
        '''
         xpsLagSetHashAlgo: Enter [ devId,xpsHashAlgo ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,xpsHashAlgo ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, xpsHashAlgo=%d' % (args[0],args[1]))
            ret = xpsLagSetHashAlgo(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagGetHashAlgo
    #/********************************************************************************/
    def do_lag_get_hash_algo(self, arg):
        '''
         xpsLagGetHashAlgo: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            lagHashAlgo_Ptr = new_xpsLagHashAlgoType_ep()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsLagGetHashAlgo(args[0],lagHashAlgo_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lagHashAlgo = %d' % (xpsLagHashAlgoType_ep_value(lagHashAlgo_Ptr)))
                xpShellGlobals.cmdRetVal = (xpsLagHashAlgoType_ep_value(lagHashAlgo_Ptr))
                pass
            delete_xpsLagHashAlgoType_ep(lagHashAlgo_Ptr)

    #/********************************************************************************/
    # command for xpsLagSetHashSeed
    #/********************************************************************************/
    def do_lag_set_hash_seed(self, arg):
        '''
         xpsLagSetHashSeed: Enter [ devId,xpsHashAlgo ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,xpsHashSeed ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, xpsHashSeed=%d' % (args[0],args[1]))
            ret = xpsLagSetHashSeed(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsLagGetHashSeed
    #/********************************************************************************/
    def do_lag_get_hash_seed(self, arg):
        '''
         xpsLagGetHashSeed: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            lagHashSeed_Ptr = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsLagGetHashSeed(args[0],lagHashSeed_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lagHashSeed = %d' % (uint32_tp_value(lagHashSeed_Ptr)))
                xpShellGlobals.cmdRetVal = (uint32_tp_value(lagHashSeed_Ptr))
                pass
            delete_uint32_tp(lagHashSeed_Ptr)

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

