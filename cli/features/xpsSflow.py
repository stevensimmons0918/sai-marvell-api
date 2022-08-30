#!/usr/bin/env python
#  xpsSflow.py
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
# The class object for xpsSflow operations
#/**********************************************************************************/

class xpsSflowObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsSflowInitScope
    #/********************************************************************************/
    def do_sflow_init_scope(self, arg):
        '''
         xpsSflowInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsSflowInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSflowInit
    #/********************************************************************************/
    def do_sflow_init(self, arg):
        '''
         xpsSflowInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsSflowInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSflowDeInitScope
    #/********************************************************************************/
    def do_sflow_de_init_scope(self, arg):
        '''
         xpsSflowDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsSflowDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSflowDeInit
    #/********************************************************************************/
    def do_sflow_de_init(self, arg):
        '''
         xpsSflowDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsSflowDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSflowAddDevice
    #/********************************************************************************/
    def do_sflow_add_device(self, arg):
        '''
         xpsSflowAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsSflowAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSflowRemoveDevice
    #/********************************************************************************/
    def do_sflow_remove_device(self, arg):
        '''
         xpsSflowRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsSflowRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_sflow_set_port_sampling_config_scope(self, text, line, begidx, endidx):
        tempDict = { 3 : 'acmClient'}
        return  enumDict.autoCompletion(line, text , tempDict) 

    #/********************************************************************************/
    # command for xpsSflowSetPortSamplingConfigScope
    #/********************************************************************************/
    def do_sflow_set_port_sampling_config_scope(self, arg):
        '''
         xpsSflowSetPortSamplingConfigScope: Enter [ scopeId,portIntfId,client,nSample,mBase,mExpo ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIntfId,client,nSample,mBase,mExpo ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            #print('Input Arguments are, scopeId=%d, portIntfId=%d, client=%d, nSample=%d, mBase=%d, mExpo=%d' % (args[0],args[1],args[2],args[3],args[4],args[5]))
            ret = xpsSflowSetPortSamplingConfigScope(args[0],args[1],args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_sflow_set_port_sampling_config(self, text, line, begidx, endidx):
        tempDict = { 2 : 'acmClient'}
        return  enumDict.autoCompletion(line, text , tempDict) 

    #/********************************************************************************/
    # command for xpsSflowSetPortSamplingConfig
    #/********************************************************************************/
    def do_sflow_set_port_sampling_config(self, arg):
        '''
         xpsSflowSetPortSamplingConfig: Enter [ portIntfId,client,nSample,mBase,mExpo ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIntfId,client,nSample,mBase,mExpo ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, portIntfId=%d, nSample=%d, mBase=%d, mExpo=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsSflowSetPortSamplingConfig(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for acmClient enumerations
    #/********************************************************************************/
    def complete_sflow_get_port_sampling_config_scope(self, text, line, begidx, endidx):
        tempDict = { 3 : 'acmClient'}
        return  enumDict.autoCompletion(line, text , tempDict) 

     #/********************************************************************************/
    # command for xpsSflowGetPortSamplingConfigScope
    #/********************************************************************************/
    def do_sflow_get_port_sampling_config_scope(self, arg):
        '''
         xpsSflowGetPortSamplingConfigScope: Enter [ scopeId,portIntfId,client ]
         Valid values for client : <acmClient>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIntfId,client ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            nSample_Ptr_3 = new_uint32_tp()
            mBase_Ptr_4 = new_uint32_tp()
            mExpo_Ptr_5 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, portIntfId=%d, client=%d' % (args[0],args[1],args[2]))
            ret = xpsSflowGetPortSamplingConfigScope(args[0],args[1],args[2],nSample_Ptr_3,mBase_Ptr_4,mExpo_Ptr_5)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nSample = %d' % (uint32_tp_value(nSample_Ptr_3)))
                print('mBase = %d' % (uint32_tp_value(mBase_Ptr_4)))
                print('mExpo = %d' % (uint32_tp_value(mExpo_Ptr_5)))
                pass
            delete_uint32_tp(mExpo_Ptr_5)
            delete_uint32_tp(mBase_Ptr_4)
            delete_uint32_tp(nSample_Ptr_3)
    #/********************************************************************************/
    # command for xpsSflowGetPortSamplingConfig
    #/********************************************************************************/
    def do_sflow_get_port_sampling_config(self, arg):
        '''
         xpsSflowGetPortSamplingConfig: Enter [ portIntfId,client ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIntfId,client ]')
        else:
            args[0] = int(args[0])
            nSample_Ptr_1 = new_uint32_tp()
            mBase_Ptr_2 = new_uint32_tp()
            mExpo_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, portIntfId=%d' % (args[0]))
            ret = xpsSflowGetPortSamplingConfig(args[0],nSample_Ptr_1,mBase_Ptr_2,mExpo_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('nSample = %d' % (uint32_tp_value(nSample_Ptr_1)))
                print('mBase = %d' % (uint32_tp_value(mBase_Ptr_2)))
                print('mExpo = %d' % (uint32_tp_value(mExpo_Ptr_3)))
                pass
            delete_uint32_tp(mExpo_Ptr_3)
            delete_uint32_tp(mBase_Ptr_2)
            delete_uint32_tp(nSample_Ptr_1)

    #/********************************************************************************/
    # command for xpsSflowEnablePortSamplingScope
    #/********************************************************************************/
    def do_sflow_enable_port_sampling_scope(self, arg):
        '''
         xpsSflowEnablePortSamplingScope: Enter [ scopeId,portIntfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIntfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, portIntfId=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsSflowEnablePortSamplingScope(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsSflowEnablePortSampling
    #/********************************************************************************/
    def do_sflow_enable_port_sampling(self, arg):
        '''
         xpsSflowEnablePortSampling: Enter [ portIntfId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIntfId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, portIntfId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsSflowEnablePortSampling(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for pktCmd enumerations
    #/********************************************************************************/
    def complete_sflow_set_pkt_cmd(self, text, line, begidx, endidx):
        tempDict = { 2 : 'pktCmd'}
        return  enumDict.autoCompletion(line, text , tempDict) 

    #/********************************************************************************/
    # command for xpsSflowSetPktCmd
    #/********************************************************************************/
    def do_sflow_set_pkt_cmd(self, arg):
        '''
         xpsSflowSetPktCmd: Enter [ devId,pktCmd ]
         Valid values for pktCmd : <pktCmd>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pktCmd ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, pktCmd=%d' % (args[0],args[1]))
            ret = xpsSflowSetPktCmd(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSflowGetPktCmd
    #/********************************************************************************/
    def do_sflow_get_pkt_cmd(self, arg):
        '''
         xpsSflowGetPktCmd: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            pktCmd_Ptr_1 = new_xpPktCmd_ep()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsSflowGetPktCmd(args[0],pktCmd_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('pktCmd = %d' % (xpPktCmd_ep_value(pktCmd_Ptr_1)))
                pass
            delete_xpPktCmd_ep(pktCmd_Ptr_1)
    #/********************************************************************************/
    # command for xpsSflowSetIntfId
    #/********************************************************************************/
    def do_sflow_set_intf_id(self, arg):
        '''
         xpsSflowSetIntfId: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsSflowSetIntfId(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsSflowGetIntfId
    #/********************************************************************************/
    def do_sflow_get_intf_id(self, arg):
        '''
         xpsSflowGetIntfId: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            intfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsSflowGetIntfId(args[0],intfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_1)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(intfId_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_1)
    #/********************************************************************************/
    # command for xpsSflowSetEnable
    #/********************************************************************************/
    def do_sflow_set_enable(self, arg):
        '''
         xpsSflowSetEnable: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsSflowSetEnable(args[0],args[1])
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


