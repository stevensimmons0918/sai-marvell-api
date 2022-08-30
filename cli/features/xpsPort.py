#!/usr/bin/env python
#  xpsPort.py
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
# The class object for xpsPort operations
#/**********************************************************************************/

class xpsPortObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsPortDbInitScope
    #/********************************************************************************/
    def do_port_db_init_scope(self, arg):
        '''
         xpsPortDbInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsPortDbInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsPortDbInit
    #/********************************************************************************/
    def do_port_db_init(self, arg):
        '''
         xpsPortDbInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsPortDbInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPortDbDeInitScope
    #/********************************************************************************/
    def do_port_db_de_init_scope(self, arg):
        '''
         xpsPortDbDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsPortDbDeInitScope(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPortDbDeInit
    #/********************************************************************************/
    def do_port_db_de_init(self, arg):
        '''
         xpsPortDbDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsPortDbDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPortCreateDbForDevice
    #/********************************************************************************/
    def do_port_create_db_for_device(self, arg):
        '''
         xpsPortCreateDbForDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPortCreateDbForDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPortDeleteDbForDevice
    #/********************************************************************************/
    def do_port_delete_db_for_device(self, arg):
        '''
         xpsPortDeleteDbForDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPortDeleteDbForDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPortInit
    #/********************************************************************************/
    def do_port_init(self, arg):
        '''
         xpsPortInit: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            intfId_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsPortInit(args[0],args[1],intfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_2)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(intfId_Ptr_2)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_2)

    #/********************************************************************************/
    # command for xpsPortGetCPUPortIntfId
    #/********************************************************************************/
    def do_port_get_cpu_port_intf_id(self, arg):
        '''
         xpsPortGetCPUPortIntfId: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            intfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPortGetCPUPortIntfId(args[0],intfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_1)
    #/********************************************************************************/
    # command for xpsPortGetSCPUPortIntfId
    #/********************************************************************************/
    def do_port_get_scpu_port_intf_id(self, arg):
        '''
         xpsPortGetSCPUPortIntfId: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            intfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPortGetSCPUPortIntfId(args[0],intfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_1)
    #/********************************************************************************/
    # command for xpsPortGetLoopback1PortIntfId
    #/********************************************************************************/
    def do_port_get_loopback1_port_intf_id(self, arg):
        '''
         xpsPortGetLoopback1PortIntfId: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            intfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPortGetLoopback1PortIntfId(args[0],intfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_1)
    #/********************************************************************************/
    # command for xpsPortGetLoopback0PortIntfId
    #/********************************************************************************/
    def do_port_get_loopback0_port_intf_id(self, arg):
        '''
         xpsPortGetLoopback0PortIntfId: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            intfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsPortGetLoopback0PortIntfId(args[0],intfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_1)

    #/********************************************************************************/
    # command for xpsPortGetPortIntfId
    #/********************************************************************************/
    def do_port_get_port_intf_id(self, arg):
        '''
         xpsPortGetPortIntfId: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            intfId_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsPortGetPortIntfId(args[0],args[1],intfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_2)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(intfId_Ptr_2)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_2)

    #/********************************************************************************/
    # command for xpsPortGetPortIntfId
    #/********************************************************************************/
    def do_port_get_port_ctrl_intf_id(self, arg):
        '''
         xpsPortGetPortControlIntfId: Enter [ devId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            intfId_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, devId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsPortGetPortControlIntfId(args[0],args[1],intfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('intfId = %d' % (xpsInterfaceId_tp_value(intfId_Ptr_2)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(intfId_Ptr_2)))
                pass
            delete_xpsInterfaceId_tp(intfId_Ptr_2)


     #/********************************************************************************/
    # command for xpsPortIsExistScope
    #/********************************************************************************/
    def do_port_is_exist_scope(self, arg):
        '''
         xpsPortIsExistScope: Enter [ scopeId,portIfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, portIfId=%d' % (args[0],args[1]))
            ret = xpsPortIsExistScope(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
				
    #/********************************************************************************/
    # command for xpsPortIsExist
    #/********************************************************************************/
    def do_port_is_exist(self, arg):
        '''
         xpsPortIsExist: Enter [ portIfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIfId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, portIfId=%d' % (args[0]))
            ret = xpsPortIsExist(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPortGetDevAndPortNumFromIntfScope
    #/********************************************************************************/
    def do_port_get_dev_and_port_num_from_intf_scope(self, arg):
        '''
         xpsPortGetDevAndPortNumFromIntfScope: Enter [ scopeId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            devId_Ptr_2 = new_xpsDevice_tp()
            portNum_Ptr_3 = new_xpsPort_tp()
            #print('Input Arguments are, scopeId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsPortGetDevAndPortNumFromIntfScope(args[0],args[1],devId_Ptr_2,portNum_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('devId = %d' % (xpsDevice_tp_value(devId_Ptr_2)))
                print('portNum = %d' % (xpsPort_tp_value(portNum_Ptr_3)))
                pass
            delete_xpsPort_tp(portNum_Ptr_3)
            delete_xpsDevice_tp(devId_Ptr_2)
			
    #/********************************************************************************/
    # command for xpsPortGetDevAndPortNumFromIntf
    #/********************************************************************************/
    def do_port_get_dev_and_port_num_from_intf(self, arg):
        '''
         xpsPortGetDevAndPortNumFromIntf: Enter [ intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ intfId ]')
        else:
            args[0] = int(args[0])
            devId_Ptr_1 = new_xpsDevice_tp()
            portNum_Ptr_2 = new_xpsPort_tp()
            #print('Input Arguments are, intfId=%d' % (args[0]))
            ret = xpsPortGetDevAndPortNumFromIntf(args[0],devId_Ptr_1,portNum_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('devId = %d' % (xpsDevice_tp_value(devId_Ptr_1)))
                print('portNum = %d' % (xpsPort_tp_value(portNum_Ptr_2)))
                pass
            delete_xpsPort_tp(portNum_Ptr_2)
            delete_xpsDevice_tp(devId_Ptr_1)

    #/********************************************************************************/
    # command for xpsPortSetField
    #/********************************************************************************/
    def do_port_set_field(self, arg):
        '''
         xpsPortSetField: Enter [ devId,portIfId,fNum(portDefaultEXP=1,portDefaultDSCP=2,portDefaultDEI=3,portDefaultPCP=4,portDefaultDP=5,portDefaultTC=6,mplsQosEn=7,IPQosEn=8,layer2QosEn=9,egressFilterId=10,setIngressVif=11,setBridgeDomain=12,acceptedFrameType=13,bypassTunnelVif=14,bypassACLsPBR=15,samplerEn=16,policerEn=17,portState=18,setEgressPortFilter=19,macSAmissCmd=20,bumPolicerEn=21,portACLEn=22,portDebugEn=23,portAclId=24,privateWireEn=25,addPortBasedTag=26,Reserved2=27,bridgeDomain=28,evif=29,mirrorBitMask=30,ingressVif=31,pvidModeAllPkt=32,pvid=33,mplsQosProfileIdx=34,ipQosProfileIdx=35,l2QosProfileIdx=36,etagExists=37) ,fData ]
            eg. To set pvid 25 on port-1 - port_set_field 0 1 33 25
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portIfId,fNum(portDefaultEXP=1,portDefaultDSCP=2,portDefaultDEI=3,portDefaultPCP=4,portDefaultDP=5,portDefaultTC=6,mplsQosEn=7,IPQosEn=8,layer2QosEn=9,egressFilterId=10,setIngressVif=11,setBridgeDomain=12,acceptedFrameType=13,bypassTunnelVif=14,bypassACLsPBR=15,samplerEn=16,policerEn=17,portState=18,setEgressPortFilter=19,macSAmissCmd=20,bumPolicerEn=21,portACLEn=22,portDebugEn=23,portAclId=24,privateWireEn=25,addPortBasedTag=26,Reserved2=27,bridgeDomain=28,evif=29,mirrorBitMask=30,ingressVif=31,pvidModeAllPkt=32,pvid=33,mplsQosProfileIdx=34,ipQosProfileIdx=35,l2QosProfileIdx=36,etagExists=37) ,fData ] eg. To set pvid 25 on interfaceId-1 - port_set_field 0 1 33 25')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, portIfId=%d, fNum=%d, fData=0x%x' % (args[0],args[1],args[2],args[3]))
            ret = xpsPortSetField(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPortGetField
    #/********************************************************************************/
    def do_port_get_field(self, arg):
        '''
         xpsPortGetField: Enter [ devId,portIfId,fNum(portDefaultEXP=1,portDefaultDSCP=2,portDefaultDEI=3,portDefaultPCP=4,portDefaultDP=5,portDefaultTC=6,mplsQosEn=7,IPQosEn=8,layer2QosEn=9,egressFilterId=10,setIngressVif=11,setBridgeDomain=12,acceptedFrameType=13,bypassTunnelVif=14,bypassACLsPBR=15,samplerEn=16,policerEn=17,portState=18,setEgressPortFilter=19,macSAmissCmd=20,bumPolicerEn=21,portACLEn=22,portDebugEn=23,portAclId=24,privateWireEn=25,addPortBasedTag=26,evif=27,bridgeDomain=28,Reserved2=29,mirrorBitMask=30,ingressVif=31,pvidModeAllPkt=32,pvid=33,mplsQosProfileIdx=34,ipQosProfileIdx=35,l2QosProfileIdx=36,etagExists=37) ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portIfId,fNum(portDefaultEXP=1,portDefaultDSCP=2,portDefaultDEI=3,portDefaultPCP=4,portDefaultDP=5,portDefaultTC=6,mplsQosEn=7,IPQosEn=8,layer2QosEn=9,egressFilterId=10,setIngressVif=11,setBridgeDomain=12,acceptedFrameType=13,bypassTunnelVif=14,bypassACLsPBR=15,samplerEn=16,policerEn=17,portState=18,setEgressPortFilter=19,macSAmissCmd=20,bumPolicerEn=21,portACLEn=22,portDebugEn=23,portAclId=24,privateWireEn=25,addPortBasedTag=26,evif=27,bridgeDomain=28,Reserved2=29,mirrorBitMask=30,ingressVif=31,pvidModeAllPkt=32,pvid=33,mplsQosProfileIdx=34,ipQosProfileIdx=35,l2QosProfileIdx=36,etagExists=37) ]eg. To get pvid  on interfaceId-1 - port_get_field 0 1 30')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            fData_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, portIfId=%d, fNum=%d' % (args[0],args[1],args[2]))
            ret = xpsPortGetField(args[0],args[1],args[2],fData_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fData = %d' % (uint32_tp_value(fData_Ptr_3)))
                pass
            delete_uint32_tp(fData_Ptr_3)


    #/********************************************************************************/
    # command for xpsPortSetConfig
    #/********************************************************************************/
    def do_port_set_config(self, arg):
        '''
         xpsPortSetConfig: Enter [ devId,devPort,portDefaultEXP,portDefaultDSCP,portDefaultDEI,portDefaultPCP,portDefaultDP,portDefaultTC,mplsQosEn,IPQosEn,layer2QosEn,egressFilterId,setIngressVif,setBridgeDomain,acceptedFrameType,bypassTunnelVif,bypassACLsPBR,samplerEn,policerEn,portState,setEgressPortFilter,macSAmissCmd,bumPolicerEn,portACLEn,portDebugEn,portAclId,privateWireEn,addPortBasedTag,evif,bridgeDomain,mirrorBitMask,ingressVif,pvidModeAllPkt,pvid,mplsQosProfileIdx,ipQosProfileIdx,l2QosProfileIdx,etagExists ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 38
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,portDefaultEXP,portDefaultDSCP,portDefaultDEI,portDefaultPCP,portDefaultDP,portDefaultTC,mplsQosEn,IPQosEn,layer2QosEn,egressFilterId,setIngressVif,setBridgeDomain,acceptedFrameType,bypassTunnelVif,bypassACLsPBR,samplerEn,policerEn,portState,setEgressPortFilter,macSAmissCmd,bumPolicerEn,portACLEn,portDebugEn,portAclId,privateWireEn,addPortBasedTag,evif,bridgeDomain,mirrorBitMask,ingressVif,pvidModeAllPkt,pvid,mplsQosProfileIdx,ipQosProfileIdx,l2QosProfileIdx,etagExists ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsPortConfig_t_Ptr = new_xpsPortConfig_tp()
            xpsPortConfig_t_Ptr.portDefaultEXP = int(args[2])
            xpsPortConfig_t_Ptr.portDefaultDSCP = int(args[3])
            xpsPortConfig_t_Ptr.portDefaultDEI = int(args[4])
            xpsPortConfig_t_Ptr.portDefaultPCP = int(args[5])
            xpsPortConfig_t_Ptr.portDefaultDP = int(args[6])
            xpsPortConfig_t_Ptr.portDefaultTC = int(args[7])
            xpsPortConfig_t_Ptr.mplsQosEn = int(args[8])
            xpsPortConfig_t_Ptr.IPQosEn = int(args[9])
            xpsPortConfig_t_Ptr.layer2QosEn = int(args[10])
            xpsPortConfig_t_Ptr.egressFilterId = int(args[11])
            xpsPortConfig_t_Ptr.setIngressVif = int(args[12])
            xpsPortConfig_t_Ptr.setBridgeDomain = int(args[13])
            xpsPortConfig_t_Ptr.acceptedFrameType = int(args[14])
            xpsPortConfig_t_Ptr.bypassTunnelVif = int(args[15])
            xpsPortConfig_t_Ptr.bypassACLsPBR = int(args[16])
            xpsPortConfig_t_Ptr.samplerEn = int(args[17])
            xpsPortConfig_t_Ptr.policerEn = int(args[18])
            
            xpsPortConfig_t_Ptr.portState = int(args[19])
            xpsPortConfig_t_Ptr.setEgressPortFilter = int(args[20])
            xpsPortConfig_t_Ptr.macSAmissCmd = int(args[21])
            xpsPortConfig_t_Ptr.bumPolicerEn = int(args[22])
            xpsPortConfig_t_Ptr.portACLEn = int(args[23])
            xpsPortConfig_t_Ptr.portDebugEn = int(args[24])
            xpsPortConfig_t_Ptr.portAclId = int(args[25])
            xpsPortConfig_t_Ptr.privateWireEn = int(args[26])
            xpsPortConfig_t_Ptr.addPortBasedTag = int(args[27])
            xpsPortConfig_t_Ptr.evif = int(args[28])
            xpsPortConfig_t_Ptr.bridgeDomain = int(args[29])
            xpsPortConfig_t_Ptr.mirrorBitMask = int(args[30])
            xpsPortConfig_t_Ptr.ingressVif = int(args[31])
            xpsPortConfig_t_Ptr.pvidModeAllPkt = int(args[32])
            xpsPortConfig_t_Ptr.pvid = int(args[33])
            xpsPortConfig_t_Ptr.mplsQosProfileIdx = int(args[34])
            xpsPortConfig_t_Ptr.ipQosProfileIdx = int(args[35])
            xpsPortConfig_t_Ptr.l2QosProfileIdx = int(args[36])
            xpsPortConfig_t_Ptr.etagExists = int(args[37])

            print('Input Arguments are, devId=%d, devPort=%d, portDefaultEXP=%d, portDefaultDSCP=%d, portDefaultDEI=%d, portDefaultPCP=%d, portDefaultDP=%d, portDefaultTC=%d, mplsQosEn=%d, IPQosEn=%d, layer2QosEn=%d, egressFilterId=%d, setIngressVif=%d, setBridgeDomain=%d, acceptedFrameType=%d, bypassTunnelVif=%d, bypassACLsPBR=%d, samplerEn=%d, policerEn=%d, privateWireEn=%d, addPortBasedTag=%d, evif=%d,portState=%d, setEgressPortFilter=%d, macSAmissCmd=%d, bumPolicerEn=%d,  portACLEn=%d, portDebugEn=%d, portAclId=%d, bridgeDomain=%d, mirrorBitMask=%d, ingressVif=%d, pvidModeAllPkt=%d, pvid=%d, mplsQosProfileIdx=%d, ipQosProfileIdx=%d, l2QosProfileIdx=%d, etagExists=%d' % (args[0],args[1],xpsPortConfig_t_Ptr.portDefaultEXP,xpsPortConfig_t_Ptr.portDefaultDSCP,xpsPortConfig_t_Ptr.portDefaultDEI,xpsPortConfig_t_Ptr.portDefaultPCP,xpsPortConfig_t_Ptr.portDefaultDP,xpsPortConfig_t_Ptr.portDefaultTC,xpsPortConfig_t_Ptr.mplsQosEn,xpsPortConfig_t_Ptr.IPQosEn,xpsPortConfig_t_Ptr.layer2QosEn,xpsPortConfig_t_Ptr.egressFilterId,xpsPortConfig_t_Ptr.setIngressVif,xpsPortConfig_t_Ptr.setBridgeDomain,xpsPortConfig_t_Ptr.acceptedFrameType,xpsPortConfig_t_Ptr.bypassTunnelVif,xpsPortConfig_t_Ptr.bypassACLsPBR,xpsPortConfig_t_Ptr.samplerEn,xpsPortConfig_t_Ptr.policerEn,xpsPortConfig_t_Ptr.privateWireEn,xpsPortConfig_t_Ptr.addPortBasedTag,xpsPortConfig_t_Ptr.evif,xpsPortConfig_t_Ptr.portState,xpsPortConfig_t_Ptr.setEgressPortFilter,xpsPortConfig_t_Ptr.macSAmissCmd,xpsPortConfig_t_Ptr.bumPolicerEn,xpsPortConfig_t_Ptr.portACLEn,xpsPortConfig_t_Ptr.portDebugEn,xpsPortConfig_t_Ptr.portAclId,xpsPortConfig_t_Ptr.bridgeDomain,xpsPortConfig_t_Ptr.mirrorBitMask,xpsPortConfig_t_Ptr.ingressVif,xpsPortConfig_t_Ptr.pvidModeAllPkt,xpsPortConfig_t_Ptr.pvid,xpsPortConfig_t_Ptr.mplsQosProfileIdx,xpsPortConfig_t_Ptr.ipQosProfileIdx,xpsPortConfig_t_Ptr.l2QosProfileIdx,xpsPortConfig_t_Ptr.etagExists))
            ret = xpsPortSetConfig(args[0],args[1],xpsPortConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 0
            if err == 0:
                print('portDefaultEXP = %d' % (xpsPortConfig_t_Ptr.portDefaultEXP))
                print('portDefaultDSCP = %d' % (xpsPortConfig_t_Ptr.portDefaultDSCP))
                print('portDefaultDEI = %d' % (xpsPortConfig_t_Ptr.portDefaultDEI))
                print('portDefaultPCP = %d' % (xpsPortConfig_t_Ptr.portDefaultPCP))
                print('portDefaultDP = %d' % (xpsPortConfig_t_Ptr.portDefaultDP))
                print('portDefaultTC = %d' % (xpsPortConfig_t_Ptr.portDefaultTC))
                print('mplsQosEn = %d' % (xpsPortConfig_t_Ptr.mplsQosEn))
                print('IPQosEn = %d' % (xpsPortConfig_t_Ptr.IPQosEn))
                print('layer2QosEn = %d' % (xpsPortConfig_t_Ptr.layer2QosEn))
                print('egressFilterId = %d' % (xpsPortConfig_t_Ptr.egressFilterId))
                print('setIngressVif = %d' % (xpsPortConfig_t_Ptr.setIngressVif))
                print('setBridgeDomain = %d' % (xpsPortConfig_t_Ptr.setBridgeDomain))
                print('acceptedFrameType = %d' % (xpsPortConfig_t_Ptr.acceptedFrameType))
                print('bypassTunnelVif = %d' % (xpsPortConfig_t_Ptr.bypassTunnelVif))
                print('bypassACLsPBR = %d' % (xpsPortConfig_t_Ptr.bypassACLsPBR))
                print('samplerEn = %d' % (xpsPortConfig_t_Ptr.samplerEn))
                print('policerEn = %d' % (xpsPortConfig_t_Ptr.policerEn))
                print('privateWireEn = %d' % (xpsPortConfig_t_Ptr.privateWireEn))
                print('addPortBasedTag = %d' % (xpsPortConfig_t_Ptr.addPortBasedTag))
                print('evif = %d' % (xpsPortConfig_t_Ptr.evif))
                print('portState = %d' % (xpsPortConfig_t_Ptr.portState))
                print('setEgressPortFilter = %d' % (xpsPortConfig_t_Ptr.setEgressPortFilter))
                print('macSAmissCmd = %d' % (xpsPortConfig_t_Ptr.macSAmissCmd))
                print('bumPolicerEn = %d' % (xpsPortConfig_t_Ptr.bumPolicerEn))
                print('portACLEn = %d' % (xpsPortConfig_t_Ptr.portACLEn))
                print('portDebugEn = %d' % (xpsPortConfig_t_Ptr.portDebugEn))
                print('portAclId = %d' % (xpsPortConfig_t_Ptr.portAclId))
                print('bridgeDomain = %d' % (xpsPortConfig_t_Ptr.bridgeDomain))
                print('mirrorBitMask = %d' % (xpsPortConfig_t_Ptr.mirrorBitMask))
                print('ingressVif = %d' % (xpsPortConfig_t_Ptr.ingressVif))
                print('pvidModeAllPkt = %d' % (xpsPortConfig_t_Ptr.pvidModeAllPkt))
                print('pvid = %d' % (xpsPortConfig_t_Ptr.pvid))
                print('mplsQosProfileIdx = %d' % (xpsPortConfig_t_Ptr.mplsQosProfileIdx))
                print('ipQosProfileIdx = %d' % (xpsPortConfig_t_Ptr.ipQosProfileIdx))
                print('l2QosProfileIdx = %d' % (xpsPortConfig_t_Ptr.l2QosProfileIdx))
                print('etagExists = %d' % (xpsPortConfig_t_Ptr.etagExists))
                pass
            delete_xpsPortConfig_tp(xpsPortConfig_t_Ptr)

    
    #/********************************************************************************/
    # command for xpsPortGetConfig
    #/********************************************************************************/
    def do_port_get_config(self, arg):
        '''
         xpsPortGetConfig: Enter [ devId,devPort ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            devId = args[0]
            devPort = args[1]
            xpsPortConfig_t_Ptr = new_xpsPortConfig_tp()
            
            #print('Input Arguments are, devId=%d, devPort=%d' % (devId, devPort))
            ret = xpsPortGetConfig(args[0],args[1],xpsPortConfig_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('portDefaultEXP = %d' % (xpsPortConfig_t_Ptr.portDefaultEXP))
                print('portDefaultDSCP = %d' % (xpsPortConfig_t_Ptr.portDefaultDSCP))
                print('portDefaultDEI = %d' % (xpsPortConfig_t_Ptr.portDefaultDEI))
                print('portDefaultPCP = %d' % (xpsPortConfig_t_Ptr.portDefaultPCP))
                print('portDefaultDP = %d' % (xpsPortConfig_t_Ptr.portDefaultDP))
                print('portDefaultTC = %d' % (xpsPortConfig_t_Ptr.portDefaultTC))
                print('mplsQosEn = %d' % (xpsPortConfig_t_Ptr.mplsQosEn))
                print('IPQosEn = %d' % (xpsPortConfig_t_Ptr.IPQosEn))
                print('layer2QosEn = %d' % (xpsPortConfig_t_Ptr.layer2QosEn))
                print('egressFilterId = %d' % (xpsPortConfig_t_Ptr.egressFilterId))
                print('setIngressVif = %d' % (xpsPortConfig_t_Ptr.setIngressVif))
                print('setBridgeDomain = %d' % (xpsPortConfig_t_Ptr.setBridgeDomain))
                print('acceptedFrameType = %d' % (xpsPortConfig_t_Ptr.acceptedFrameType))
                print('bypassTunnelVif = %d' % (xpsPortConfig_t_Ptr.bypassTunnelVif))
                print('bypassACLsPBR = %d' % (xpsPortConfig_t_Ptr.bypassACLsPBR))
                print('samplerEn = %d' % (xpsPortConfig_t_Ptr.samplerEn))
                print('policerEn = %d' % (xpsPortConfig_t_Ptr.policerEn))
                print('privateWireEn = %d' % (xpsPortConfig_t_Ptr.privateWireEn))
                print('addPortBasedTag = %d' % (xpsPortConfig_t_Ptr.addPortBasedTag))
                print('evif = %d' % (xpsPortConfig_t_Ptr.evif))
                print('portState = %d' % (xpsPortConfig_t_Ptr.portState))
                print('setEgressPortFilter = %d' % (xpsPortConfig_t_Ptr.setEgressPortFilter))
                print('macSAmissCmd = %d' % (xpsPortConfig_t_Ptr.macSAmissCmd))
                print('bumPolicerEn = %d' % (xpsPortConfig_t_Ptr.bumPolicerEn))
                print('portACLEn = %d' % (xpsPortConfig_t_Ptr.portACLEn))
                print('portDebugEn = %d' % (xpsPortConfig_t_Ptr.portDebugEn))
                print('portAclId = %d' % (xpsPortConfig_t_Ptr.portAclId))
                print('bridgeDomain = %d' % (xpsPortConfig_t_Ptr.bridgeDomain))
                print('mirrorBitMask = %d' % (xpsPortConfig_t_Ptr.mirrorBitMask))
                print('ingressVif = %d' % (xpsPortConfig_t_Ptr.ingressVif))
                print('pvidModeAllPkt = %d' % (xpsPortConfig_t_Ptr.pvidModeAllPkt))
                print('pvid = %d' % (xpsPortConfig_t_Ptr.pvid))
                print('mplsQosProfileIdx = %d' % (xpsPortConfig_t_Ptr.mplsQosProfileIdx))
                print('ipQosProfileIdx = %d' % (xpsPortConfig_t_Ptr.ipQosProfileIdx))
                print('l2QosProfileIdx = %d' % (xpsPortConfig_t_Ptr.l2QosProfileIdx))
                print('etagExists = %d' % (xpsPortConfig_t_Ptr.etagExists))
                pass
            delete_xpsPortConfig_tp(xpsPortConfig_t_Ptr)

    #/********************************************************************************/
    # command for xpsPortDump
    #/********************************************************************************/
    def do_port_dump(self, arg):
        '''
         xpsPortDump: Enter [ devId,devPort,detail ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3 
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,devPort,detail ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = bool(args[2])
            devId   = args[0]
            devPort = args[1]
            detail  = args[2] 
            #print('Input Arguments are, devId=%d, devPort=%d, detail = %d' % (devId, devPort, detail))
            ret = xpsPortDump(args[0],args[1],args[2])
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPortEnableMirroring
    #/********************************************************************************/
    def do_port_enable_mirroring(self, arg):
        '''
         xpsPortEnableMirroring: Enter [ portIfId,analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIfId,analyzerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, portIfId=%d, analyzerId=%d' % (args[0],args[1]))
            ret = xpsPortEnableMirroring(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPortDisableMirroring
    #/********************************************************************************/
    def do_port_disable_mirroring(self, arg):
        '''
         xpsPortDisableMirroring: Enter [ portIfId,analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIfId,analyzerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, portIfId=%d, analyzerId=%d' % (args[0],args[1]))
            ret = xpsPortDisableMirroring(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPortSetPolicingEnable
    #/********************************************************************************/
    def do_port_set_policing_enable(self, arg):
        '''
         xpsPortSetPolicingEnable: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsPortSetPolicingEnable(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPortEnableMirroringScope
    #/********************************************************************************/
    def do_port_enable_mirroring_scope(self, arg):
        '''
         xpsPortEnableMirroringScope: Enter [ scopeId,portIfId,analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIfId,analyzerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, portIfId=%d, analyzerId=%d' % (args[0],args[1],args[2]))
            ret = xpsPortEnableMirroringScope(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPortDisableMirroringScope
    #/********************************************************************************/
    def do_port_disable_mirroring_scope(self, arg):
        '''
         xpsPortDisableMirroringScope: Enter [ scopeId,portIfId,analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIfId,analyzerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, portIfId=%d, analyzerId=%d' % (args[0],args[1],args[2]))
            ret = xpsPortDisableMirroringScope(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPortGetFirstScope
    #/********************************************************************************/
    def do_port_get_first_scope(self, arg):
        '''
         xpsPortGetFirstScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            portIfId_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsPortGetFirstScope(args[0],portIfId_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('portIfId = %d' % (xpsInterfaceId_tp_value(portIfId_Ptr_1)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(portIfId_Ptr_1))
                pass
            delete_xpsInterfaceId_tp(portIfId_Ptr_1)
    #/********************************************************************************/
    # command for xpsPortGetNextScope
    #/********************************************************************************/
    def do_port_get_next_scope(self, arg):
        '''
         xpsPortGetNextScope: Enter [ scopeId,portIfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,portIfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            portIfIdNext_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, portIfId=%d' % (args[0],args[1]))
            ret = xpsPortGetNextScope(args[0],args[1],portIfIdNext_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('portIfIdNext = %d' % (xpsInterfaceId_tp_value(portIfIdNext_Ptr_2)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(portIfIdNext_Ptr_2))
                pass
            delete_xpsInterfaceId_tp(portIfIdNext_Ptr_2)
    #/********************************************************************************/
    # command for xpsPortGetFirst
    #/********************************************************************************/
    def do_port_get_first(self, arg):
        '''
         xpsPortGetFirst: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ ]')
        else:
            portIfId_Ptr_0 = new_xpsInterfaceId_tp()
            #print('Input Arguments are' % ())
            ret = xpsPortGetFirst(portIfId_Ptr_0)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('portIfId = %d' % (xpsInterfaceId_tp_value(portIfId_Ptr_0)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(portIfId_Ptr_0)))
                pass
            delete_xpsInterfaceId_tp(portIfId_Ptr_0)

    #/********************************************************************************/
    # command for xpsPortGetNext
    #/********************************************************************************/
    def do_port_get_next(self, arg):
        '''
         xpsPortGetNext: Enter [ portIfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ portIfId ]')
        else:
            args[0] = int(args[0])
            portIfIdNext_Ptr_1 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, portIfId=%d' % (args[0]))
            ret = xpsPortGetNext(args[0],portIfIdNext_Ptr_1)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('portIfIdNext = %d' % (xpsInterfaceId_tp_value(portIfIdNext_Ptr_1)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(portIfIdNext_Ptr_1)))
                pass
            delete_xpsInterfaceId_tp(portIfIdNext_Ptr_1)

    #/********************************************************************************/
    # command for xpsPortGetMaxNum
    #/********************************************************************************/
    def do_port_get_max_num(self, arg):
        '''
         xpsPortGetMaxNum: Enter [ ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            portIfId_Ptr_0 = new_xpsInterfaceId_tp()
            ret = xpsPortGetMaxNum(args[0], portIfId_Ptr_0)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('MaxPortNumb = %d' % (xpsInterfaceId_tp_value(portIfId_Ptr_0)))
                xpShellGlobals.cmdRetVal = ((xpsInterfaceId_tp_value(portIfId_Ptr_0)))
                pass
            delete_xpsInterfaceId_tp(portIfId_Ptr_0)

    #/********************************************************************************/
    # command for xpsPortSetRandomHash
    #/********************************************************************************/
    def do_port_set_random_hash(self, arg):
        '''
         xpsPortSetRandomHash: Enter [ devId,port,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, port=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsPortSetRandomHash(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPortGetRandomHash
    #/********************************************************************************/
    def do_port_get_random_hash(self, arg):
        '''
         xpsPortGetRandomHash: Enter [ devId,port ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d' % (args[0],args[1]))
            ret = xpsPortGetRandomHash(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsPortSetStaticHash
    #/********************************************************************************/
    def do_port_set_static_hash(self, arg):
        '''
         xpsPortSetStaticHash: Enter [ devId,port,hashType,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,hashType,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, port=%d, hashType=%d, enable=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsPortSetStaticHash(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPortGetStaticHash
    #/********************************************************************************/
    def do_port_get_static_hash(self, arg):
        '''
         xpsPortGetStaticHash: Enter [ devId,port,hashType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,hashType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            enable_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, port=%d, hashType=%d' % (args[0],args[1],args[2]))
            ret = xpsPortGetStaticHash(args[0],args[1],args[2],enable_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_3)))
                pass
            delete_uint32_tp(enable_Ptr_3)
    #/********************************************************************************/
    # command for xpsPortKeepEtag
    #/********************************************************************************/
    def do_port_keep_etag(self, arg):
        '''
         xpsPortKeepEtag: Enter [ devId,port,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, port=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsPortKeepEtag(args[0],args[1],args[2])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPortAddEtag
    #/********************************************************************************/
    def do_port_add_etag(self, arg):
        '''
         xpsPortAddEtag: Enter [ devId,port,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, port=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsPortAddEtag(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPortEnableLongEtagMode
    #/********************************************************************************/
    def do_port_enable_long_etag_mode(self, arg):
        '''
         xpsPortEnableLongEtagMode: Enter [ devId,port,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, port=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsPortEnableLongEtagMode(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsPortBridgePortSetMacSaMissCmd
    #/********************************************************************************/
    def do_port_bridge_port_set_mac_sa_miss_cmd(self, arg):
        '''
         xpsPortBridgePortSetMacSaMissCmd: Enter [devId, port, xpPktCmd_e]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [devId, port, xpPktCmd_e]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, port=%d, saMissCmd=%d' % (args[0],args[1],args[2]))
            ret = xpsPortBridgePortSetMacSaMissCmd(args[0],args[1],args[2])
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

