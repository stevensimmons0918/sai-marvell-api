#!/usr/bin/env python
#  xpsGlobalSwitchControl.py
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

from LagHash import *
from enumDict import *
import xpShellGlobals

#/**********************************************************************************/
# import everything from buildTarget
from buildTarget import *

layerList=[
    'ETHERNET',
    'TU_ETHER',
    'ULL_ETHERNET',
    'PBB',
    'ARP',
    'IPv4',
    'IPv6',
    'IPv6_SPLIT',
    'MPLS',
    'GRE',
    'ICMP',
    'IGMP',
    'ICMPv6',
    'UDP',
    'TCP',
    'FCoE',
    'FCoE_SPLIT',
    'CNM',
    'GENEVE',
    'GRE_NOLAYER',
    'UDPVxLanNsh',
    'NSH',
    'PTP',
    'TUNNEL_IPv6_SPLIT',
    'TUNNEL_IPv6_ICMPv6',
    'TUNNEL_IPv6_UDP',
    'TUNNEL_IPv6_TCP',
    'UDPVxLan',
    'CFM',
    'LAYER_NA',
]

hashInstance=[
'XP_HASHA_ONLY',
'XP_HASHB_ONLY',
'XP_HASHA_AND_HASHB'
]

#/********************************************************************************/
# Auto completion for hashLayer Enums
#/********************************************************************************/
def hashLayerCompletion(text):
    return [i for i in layerList if i.startswith(text.upper())]

def hashInstanceCompletion(text):
    return [i for i in hashInstance if i.startswith(text.upper())]

#/**********************************************************************************/
# The class object for xpsGlobalSwitchControl operations
#/**********************************************************************************/

class xpsGlobalSwitchControlObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsGlobalSwitchControlInit
    #/********************************************************************************/
    def do_global_switch_control_init(self, arg):
        '''
         xpsGlobalSwitchControlInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsGlobalSwitchControlInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlInitScope
    #/********************************************************************************/
    def do_global_switch_control_init_scope(self, arg):
        '''
         xpsGlobalSwitchControlInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsGlobalSwitchControlInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlDeInit
    #/********************************************************************************/
    def do_global_switch_control_de_init(self, arg):
        '''
         xpsGlobalSwitchControlDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsGlobalSwitchControlDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlDeInitScope
    #/********************************************************************************/
    def do_global_switch_control_de_init_scope(self, arg):
        '''
         xpsGlobalSwitchControlDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsGlobalSwitchControlDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlAddDevice
    #/********************************************************************************/
    def do_global_switch_control_add_device(self, arg):
        '''
         xpsGlobalSwitchControlAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlRemoveDevice
    #/********************************************************************************/
    def do_global_switch_control_remove_device(self, arg):
        '''
         xpsGlobalSwitchControlRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsGlobalSwitchControlRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass


    #/********************************************************************************/
    # command for xpsGlobalSwitchControlChipReset
    #/********************************************************************************/
    def do_global_switch_control_chip_reset(self, arg):
        '''
         xpsGlobalSwitchControlChipReset: Enter [ deviceId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId ]')
        else:
            args[0] = int(args[0])
            ret = xpsGlobalSwitchControlChipReset(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGlobalSwitchControlChipIssueDynamicReset
    # Auto completion for xpSlaveResetIdList Enums
    #/********************************************************************************/
    def complete_global_switch_control_chip_issue_dynamic_reset(self, text, line, begidx, endidx):
        tempDict = { 2 : 'slaveResetIdList'}
        return enumDict.autoCompletion(line , text, tempDict)

    def do_global_switch_control_chip_issue_dynamic_reset(self, arg):
        '''
         xpsGlobalSwitchControlChipIssueDynamicReset: Enter [ deviceId, modules fieldID ... ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId, list of modules fieldID ... ]')
        else:
            args[0] = int(args[0])
            postList = args[1].strip("]").strip("[").split(",")
            listLen = len(postList)
            for modulesFieldEnum in postList:
                modulesFieldEnum = modulesFieldEnum.strip()
                if modulesFieldEnum not in enumDict.enumDictionaries['slaveResetIdList'].keys():
                    print "invalid data %s"%(modulesFieldEnum)
                    return
            xpSlaveResetIdPtr = new_xpSlaveResetId_arr(listLen)
            for i in range(0, listLen):
                xpSlaveResetId_arr_setitem(xpSlaveResetIdPtr, i, eval(postList[i]))
            ret = xpsGlobalSwitchControlChipIssueDynamicReset(args[0], xpSlaveResetIdPtr, listLen)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
            delete_xpSlaveResetId_arr(xpSlaveResetIdPtr)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlChipIssueConfigReset
    # Auto completion for xpSlaveResetIdList Enums
    #/********************************************************************************/
    def complete_global_switch_control_chip_issue_config_reset(self, text, line, begidx, endidx):
        tempDict = { 2 : 'slaveResetIdList'}
        return enumDict.autoCompletion(line , text, tempDict)
    def do_global_switch_control_chip_issue_config_reset(self, arg):
        '''
         xpsGlobalSwitchControlChipIssueConfigReset: Enter [ deviceId, modules fieldID ... ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId, list of modules fieldID ... ]')
        else:
            args[0] = int(args[0])
            postList = args[1].strip("]").strip("[").split(",")
            listLen = len(postList)
            for modulesFieldEnum in postList:
                modulesFieldEnum = modulesFieldEnum.strip()
                if modulesFieldEnum not in enumDict.enumDictionaries['slaveResetIdList'].keys():
                    print "invalid data %s"%(modulesFieldEnum)
                    return
            xpSlaveResetIdPtr = new_xpSlaveResetId_arr(listLen)
            for i in range(0, listLen):
                xpSlaveResetId_arr_setitem(xpSlaveResetIdPtr, i, eval(postList[i]))
            ret = xpsGlobalSwitchControlChipIssueConfigReset(args[0], xpSlaveResetIdPtr, listLen)
            if ret != 0:
                delete_xpSlaveResetId_arr(xpSlaveResetIdPtr)
                print('Error returned = %d' % (ret))
            else:
                pass
            delete_xpSlaveResetId_arr(xpSlaveResetIdPtr)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetTCPSymmetricHashing
    #/********************************************************************************/
    def do_global_switch_control_set_tcp_symmetric_hashing(self, arg):
        '''
         xpsGlobalSwitchControlSetTCPSymmetricHashing: Enter [ deviceId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, deviceId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlSetTCPSymmetricHashing(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetUDPSymmetricHashing
    #/********************************************************************************/
    def do_global_switch_control_set_udp_symmetric_hashing(self, arg):
        '''
         xpsGlobalSwitchControlSetUDPSymmetricHashing: Enter [ deviceId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, deviceId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlSetUDPSymmetricHashing(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetIPv4SymmetricHashing
    #/********************************************************************************/
    def do_global_switch_control_set_i_pv4_symmetric_hashing(self, arg):
        '''
         xpsGlobalSwitchControlSetIPv4SymmetricHashing: Enter [ deviceId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, deviceId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlSetIPv4SymmetricHashing(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetVlanRangeCheck
    #/********************************************************************************/
    def do_global_switch_control_set_vlan_range_check(self, arg):
        '''
         xpsGlobalSwitchControlSetVlanRangeCheck: Enter [ deviceId,enable,minVlan,maxVlan ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,enable,minVlan,maxVlan ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, deviceId=%d, enable=%d, minVlan=%d, maxVlan=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsGlobalSwitchControlSetVlanRangeCheck(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetLayerRangeCheck
    #/********************************************************************************/
    def do_global_switch_control_set_layer_range_check(self, arg):
        '''
         xpsGlobalSwitchControlSetLayerRangeCheck: Enter [ deviceId, rangeNum,layer,enable,destPortCheck,minPort,maxPort,resultLocation ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 8
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,rangeNum,layer,enable,destPortCheck,minPort,maxPort,resultLocation ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5])
            args[6] = int(args[6])
            args[7] = int(args[7])
            #print('Input Arguments are, deviceId=%d, enable=%d, destPortCheck=%d, minPort=%d, maxPort=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsGlobalSwitchControlSetLayerRangeCheck(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetLayerRangeCheck
    #/********************************************************************************/
    def do_global_switch_control_get_layer_range_check(self, arg):
        '''
         xpsGlobalSwitchControlGetLayerRangeCheck: Enter [ deviceId, rangeNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,rangeNum]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            layer1 = new_uint32_tp()
            layer2 = new_uint32_tp()
            destPortCheck = new_uint32_tp()
            minPort = new_uint32_tp()
            maxPort = new_uint32_tp()
            resultLocation = new_uint32_tp()

            #print('Input Arguments are, deviceId=%d, enable=%d, destPortCheck=%d, minPort=%d, maxPort=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsGlobalSwitchControlGetLayerRangeCheck(args[0],args[1],layer1, layer2, destPortCheck, minPort, maxPort, resultLocation)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print(' layer1 = %d' % (uint32_tp_value(layer1)))
                print(' layer2 = %d' % (uint32_tp_value(layer2)))
                print(' destPortCheck = %d' % (uint32_tp_value(destPortCheck)))
                print(' minPort = %d' % (uint32_tp_value(minPort)))
                print(' maxPort = %d' % (uint32_tp_value(maxPort)))
                print(' resultLocation= %d' % (uint32_tp_value(resultLocation)))
                pass
            delete_uint32_tp(layer1)
            delete_uint32_tp(layer2)
            delete_uint32_tp(destPortCheck)
            delete_uint32_tp(minPort)
            delete_uint32_tp(maxPort)
            delete_uint32_tp(resultLocation)
    def complete_global_switch_control_get_layer_hash_fields(self, text, line, begidx, endidx):
        return hashLayerCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetLayer2RangeCheck
    #/********************************************************************************/
    def do_global_switch_control_get_layer_hash_fields(self, arg):
        '''
        xpsGlobalSwitchControlGetHashFields : Enter [deviceId, layer]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, layer ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])

            N = 4
            hashFieldData = new_arrPtrUint32(N)
            for i in range(0, N):
                arrUint32 = new_arrUint32(4)
                arrPtrUint32_setitem(hashFieldData, i, arrUint32)

            numFields = new_arrUint32(1)

            ret = xpsGlobalSwitchControlGetHashFields(args[0],args[1], hashFieldData, numFields )
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

            for fieldIndex in range(0, arrUint32_getitem(numFields, 0)):
                # delete sub array
                fieldInfo = arrPtrUint32_getitem(hashFieldData, fieldIndex)
                print('offset%d = %d' % (fieldIndex, arrUint32_getitem(fieldInfo, 0)))
                print('byteLen%d = %d' % (fieldIndex, arrUint32_getitem(fieldInfo, 1)))
                print('hashMaskMSB%d = %d' % (fieldIndex, arrUint32_getitem(fieldInfo, 2)))
                print('mask%d = 0x%02x\n' % (fieldIndex, arrUint32_getitem(fieldInfo, 3)))
                delete_arrUint32(fieldInfo)
            delete_arrPtrUint32(hashFieldData)

    #/********************************************************************************/
    # Auto completion for hasField Enums
    #/********************************************************************************/
    def complete_global_switch_control_set_hash_fields(self, text, line, begidx, endidx):
        return hashFieldCompletion(text)

    def do_global_switch_control_set_hash_fields(self, arg):
        '''
         xpsGlobalSwitchControlSetHashFields: Enter [ deviceId, hashList ]
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
            ret = xpsGlobalSwitchControlSetHashFields(args[0],fields_Ptr_1,listLen)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
            delete_xpHashField_arr(fields_Ptr_1)

    #/********************************************************************************/
    # Auto completion for hasField Enums
    #/********************************************************************************/
    def complete_global_switch_control_set_hash_field_params(self, text, line, begidx, endidx):
        return hashFieldCompletion(text)

    def do_global_switch_control_set_hash_field_params(self, arg):
        '''
         xpsGlobalSwitchControlSetHashFieldParams: Enter [ deviceId, hashField, offset, length ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId, hashField, offset, length ]')
        else:
            args[0] = int(args[0])
            args[2] = int(args[2])
            args[3] = int(args[3])
            hashFieldEnum = args[1].strip()
            if hashFieldEnum not in enumDictionaries['hashField']:
                print "invalid data %s"%(hashFieldEnum)
                return

            ret = xpsGlobalSwitchControlSetHashFieldParams(args[0],eval(hashFieldEnum),args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    def complete_global_switch_control_set_hash_field_params_with_mask(self, text, line, begidx, endidx):
        return hashFieldCompletion(text)

    def do_global_switch_control_set_hash_field_params_with_mask(self, arg):
        '''
         xpsGlobalSwitchControlSetHashFieldParamsWithMask: Enter [ deviceId, hashField, offset, length, hashMaskMSB, mask ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId, hashField, offset, length, hashMaskMSB, mask ]')
        else:
            args[0] = int(args[0])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            args[5] = int(args[5], 16)
            hashFieldEnum = args[1].strip()
            if hashFieldEnum not in enumDictionaries['hashField']:
                print "invalid data %s"%(hashFieldEnum)
                return

            ret = xpsGlobalSwitchControlSetHashFieldParams(args[0],eval(hashFieldEnum),args[2],args[3],args[4],args[5])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    def complete_global_switch_control_set_hash_layer_xor(self, text, line, begidx, endidx):
        return hashLayerCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetHashLayerXor
    #/********************************************************************************/
    def do_global_switch_control_set_hash_layer_xor(self, arg):
        '''
        xpsGlobalSwitchControlSetHashLayerXor : Enter [deviceId, givenLayer, xorFieldType, layerEnable, fieldEnable]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, givenLayer, xorFieldType, layerEnable, fieldEnable ]')
        else:
            args[0] = int(args[0])
            givenLayer = eval(args[1])
            xorFieldType = eval(args[2])
            layerEnable = int(args[3])
            fieldEnable = int(args[4])
            ret = xpsGlobalSwitchControlSetHashLayerXor(args[0], givenLayer,xorFieldType,layerEnable,fieldEnable)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
    def complete_global_switch_control_get_hash_layer_xor(self, text, line, begidx, endidx):
        return hashLayerCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetHashLayerXor
    #/********************************************************************************/
    def do_global_switch_control_get_hash_layer_xor(self, arg):
        '''
        xpsGlobalSwitchControlGetHashLayerXor : Enter [deviceId, givenLayer, xorFieldType]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, givenLayer, xorFieldType ]')
        else:
            args[0] = int(args[0])
            givenLayer = eval(args[1])
            xorFieldType = eval(args[2])
            layerEnable_Ptr = new_uint32_tp()
            fieldEnable_Ptr = new_uint32_tp()
            ret = xpsGlobalSwitchControlGetHashLayerXor(args[0], givenLayer,xorFieldType,layerEnable_Ptr,fieldEnable_Ptr)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print('layerEnable = %d' % (uint32_tp_value(layerEnable_Ptr)))
                print('fieldEnable = %d' % (uint32_tp_value(fieldEnable_Ptr)))
                pass
            delete_uint32_tp(layerEnable_Ptr)
            delete_uint32_tp(fieldEnable_Ptr)

    def complete_global_switch_control_select_hashing_layers(self, text, line, begidx, endidx):
        return hashLayerCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSelectHashingLayers
    #/********************************************************************************/
    def do_global_switch_control_select_hashing_layers(self, arg):
        '''
        xpsGlobalSwitchControlSelectHashingLayers : Enter [deviceId, givenLayer, otherLayers, hashOption, enable]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId, givenLayer, [otherLayers,...], hashOption, enable ]')
        else:
            args[0] = int(args[0])
            givenLayer = eval(args[1])
            otherLayersList = args[2].strip("]").strip("[").split(",")
            listLen = len(otherLayersList)
            for otherLayerEnum in otherLayersList:
                otherLayerEnum = otherLayerEnum.strip()
                if otherLayerEnum not in layerList:
                    print "invalid data %s"%(otherLayerEnum)
                    return
            fields_Ptr_1 = new_xpLayerType_arr(listLen)
            for i in range(0, listLen):
                xpLayerType_arr_setitem(fields_Ptr_1, i, eval(otherLayersList[i]))
            hashOption = int(args[3])
            enable = int(args[4])
            ret = xpsGlobalSwitchControlSelectHashingLayers(args[0], givenLayer, fields_Ptr_1, listLen, hashOption, enable)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass
            delete_xpLayerType_arr(fields_Ptr_1)
    def complete_global_switch_control_disable_layer_hash(self, text, line, begidx, endidx):
        return hashLayerCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlDisableLayerHash
    #/********************************************************************************/
    def do_global_switch_control_disable_layer_hash(self, arg):
        '''
         xpsGlobalSwitchControlDisableLayerHash: Enter [ deviceId,layer ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,layer ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, deviceId=%d, layer=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlDisableLayerHash(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    def complete_global_switch_control_set_lag_hash_polynomial(self, text, line, begidx, endidx):
        return hashInstanceCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetLagHashPolynomial
    #/********************************************************************************/
    def do_global_switch_control_set_lag_hash_polynomial(self, arg):
        '''
         xpsGlobalSwitchControlSetLagHashPolynomial: Enter [ deviceId,instance,polynomialId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,polynomialId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, deviceId=%d, instance=%d, polynomialId=%d' % (args[0],args[1],args[2]))
            ret = xpsGlobalSwitchControlSetLagHashPolynomial(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    def complete_global_switch_control_get_lag_hash_polynomial(self, text, line, begidx, endidx):
        return hashInstanceCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetLagHashPolynomial
    #/********************************************************************************/
    def do_global_switch_control_get_lag_hash_polynomial(self, arg):
        '''
         xpsGlobalSwitchControlGetLagHashPolynomial: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            polynomialId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetLagHashPolynomial(args[0],args[1],polynomialId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('polynomialId = %d' % (uint32_tp_value(polynomialId_Ptr_2)))
                pass
            delete_uint32_tp(polynomialId_Ptr_2)
    def complete_global_switch_control_set_lag_hash_polynomial_seed(self, text, line, begidx, endidx):
        return hashInstanceCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetLagHashPolynomialSeed
    #/********************************************************************************/
    def do_global_switch_control_set_lag_hash_polynomial_seed(self, arg):
        '''
         xpsGlobalSwitchControlSetLagHashPolynomialSeed: Enter [ deviceId,instance,seed ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,seed ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, deviceId=%d, instance=%d, seed=%d' % (args[0],args[1],args[2]))
            ret = xpsGlobalSwitchControlSetLagHashPolynomialSeed(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    def complete_global_switch_control_get_lag_hash_polynomial_seed(self, text, line, begidx, endidx):
        return hashInstanceCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetLagHashPolynomialSeed
    #/********************************************************************************/
    def do_global_switch_control_get_lag_hash_polynomial_seed(self, arg):
        '''
         xpsGlobalSwitchControlGetLagHashPolynomialSeed: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            seed_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetLagHashPolynomialSeed(args[0],args[1],seed_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('seed = %d' % (uint32_tp_value(seed_Ptr_2)))
                pass
            delete_uint32_tp(seed_Ptr_2)
    def complete_global_switch_control_set_l2_ecmp_hash_polynomial(self, text, line, begidx, endidx):
        return hashInstanceCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetL2EcmpHashPolynomial
    #/********************************************************************************/
    def do_global_switch_control_set_l2_ecmp_hash_polynomial(self, arg):
        '''
         xpsGlobalSwitchControlSetL2EcmpHashPolynomial: Enter [ deviceId,instance,polynomialId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,polynomialId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, deviceId=%d, instance=%d, polynomialId=%d' % (args[0],args[1],args[2]))
            ret = xpsGlobalSwitchControlSetL2EcmpHashPolynomial(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    def complete_global_switch_control_get_l2_ecmp_hash_polynomial(self, text, line, begidx, endidx):
        return hashInstanceCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetL2EcmpHashPolynomial
    #/********************************************************************************/
    def do_global_switch_control_get_l2_ecmp_hash_polynomial(self, arg):
        '''
         xpsGlobalSwitchControlGetL2EcmpHashPolynomial: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            polynomialId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetL2EcmpHashPolynomial(args[0],args[1],polynomialId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('polynomialId = %d' % (uint32_tp_value(polynomialId_Ptr_2)))
                pass
            delete_uint32_tp(polynomialId_Ptr_2)
    def complete_global_switch_control_set_l2_ecmp_hash_polynomial_seed(self, text, line, begidx, endidx):
        return hashInstanceCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetL2EcmpHashPolynomialSeed
    #/********************************************************************************/
    def do_global_switch_control_set_l2_ecmp_hash_polynomial_seed(self, arg):
        '''
         xpsGlobalSwitchControlSetL2EcmpHashPolynomialSeed: Enter [ deviceId,instance,seed ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,seed ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, deviceId=%d, instance=%d, seed=%d' % (args[0],args[1],args[2]))
            ret = xpsGlobalSwitchControlSetL2EcmpHashPolynomialSeed(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    def complete_global_switch_control_get_l2_ecmp_hash_polynomial_seed(self, text, line, begidx, endidx):
        return hashInstanceCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetL2EcmpHashPolynomialSeed
    #/********************************************************************************/
    def do_global_switch_control_get_l2_ecmp_hash_polynomial_seed(self, arg):
        '''
         xpsGlobalSwitchControlGetL2EcmpHashPolynomialSeed: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            seed_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetL2EcmpHashPolynomialSeed(args[0],args[1],seed_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('seed = %d' % (uint32_tp_value(seed_Ptr_2)))
                pass
            delete_uint32_tp(seed_Ptr_2)
    def complete_global_switch_control_set_l3_ecmp_hash_polynomial(self, text, line, begidx, endidx):
        return hashInstanceCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetL3EcmpHashPolynomial
    #/********************************************************************************/
    def do_global_switch_control_set_l3_ecmp_hash_polynomial(self, arg):
        '''
         xpsGlobalSwitchControlSetL3EcmpHashPolynomial: Enter [ deviceId,instance,polynomialId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,polynomialId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, deviceId=%d, instance=%d, polynomialId=%d' % (args[0],args[1],args[2]))
            ret = xpsGlobalSwitchControlSetL3EcmpHashPolynomial(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    def complete_global_switch_control_get_l3_ecmp_hash_polynomial(self, text, line, begidx, endidx):
        return hashInstanceCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetL3EcmpHashPolynomial
    #/********************************************************************************/
    def do_global_switch_control_get_l3_ecmp_hash_polynomial(self, arg):
        '''
         xpsGlobalSwitchControlGetL3EcmpHashPolynomial: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            polynomialId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetL3EcmpHashPolynomial(args[0],args[1],polynomialId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('polynomialId = %d' % (uint32_tp_value(polynomialId_Ptr_2)))
                pass
            delete_uint32_tp(polynomialId_Ptr_2)
    def complete_global_switch_control_set_l3_ecmp_hash_polynomial_seed(self, text, line, begidx, endidx):
        return hashInstanceCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetL3EcmpHashPolynomialSeed
    #/********************************************************************************/
    def do_global_switch_control_set_l3_ecmp_hash_polynomial_seed(self, arg):
        '''
         xpsGlobalSwitchControlSetL3EcmpHashPolynomialSeed: Enter [ deviceId,instance,seed ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,seed ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, deviceId=%d, instance=%d, seed=%d' % (args[0],args[1],args[2]))
            ret = xpsGlobalSwitchControlSetL3EcmpHashPolynomialSeed(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    def complete_global_switch_control_get_l3_ecmp_hash_polynomial_seed(self, text, line, begidx, endidx):
        return hashInstanceCompletion(text)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetL3EcmpHashPolynomialSeed
    #/********************************************************************************/
    def do_global_switch_control_get_l3_ecmp_hash_polynomial_seed(self, arg):
        '''
         xpsGlobalSwitchControlGetL3EcmpHashPolynomialSeed: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            seed_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetL3EcmpHashPolynomialSeed(args[0],args[1],seed_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('seed = %d' % (uint32_tp_value(seed_Ptr_2)))
                pass
            delete_uint32_tp(seed_Ptr_2)

    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetLagHashPolynomialShift
    #/********************************************************************************/
    def do_global_switch_control_set_lag_hash_polynomial_shift(self, arg):
        '''
         xpsGlobalSwitchControlSetLagHashPolynomialShift: Enter [ deviceId,instance,polynomial,shift ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,polynomial,shift ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, deviceId=%d, instance=%d, polynomial=%d, shift=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsGlobalSwitchControlSetLagHashPolynomialShift(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetLagHashPolynomialShift
    #/********************************************************************************/
    def do_global_switch_control_get_lag_hash_polynomial_shift(self, arg):
        '''
         xpsGlobalSwitchControlGetLagHashPolynomialShift: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            polynomialId_Ptr_2 = new_uint32_tp()
            shift_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetLagHashPolynomialShift(args[0],args[1],polynomialId_Ptr_2,shift_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('polynomialId = %d' % (uint32_tp_value(polynomialId_Ptr_2)))
                print('shift = %d' % (uint32_tp_value(shift_Ptr_3)))
                pass
            delete_uint32_tp(shift_Ptr_3)
            delete_uint32_tp(polynomialId_Ptr_2)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetLagHashOutputMask
    #/********************************************************************************/
    def do_global_switch_control_set_lag_hash_output_mask(self, arg):
        '''
         xpsGlobalSwitchControlSetLagHashOutputMask: Enter [ deviceId,instance,outputMask ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,outputMask ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, deviceId=%d, instance=%d, outputMask=%d' % (args[0],args[1],args[2]))
            ret = xpsGlobalSwitchControlSetLagHashOutputMask(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetLagHashOutputMask
    #/********************************************************************************/
    def do_global_switch_control_get_lag_hash_output_mask(self, arg):
        '''
         xpsGlobalSwitchControlGetLagHashOutputMask: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            outputMask_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetLagHashOutputMask(args[0],args[1],outputMask_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('outputMask = %d' % (uint32_tp_value(outputMask_Ptr_2)))
                pass
            delete_uint32_tp(outputMask_Ptr_2)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetL2EcmpHashPolynomialShift
    #/********************************************************************************/
    def do_global_switch_control_set_l2_ecmp_hash_polynomial_shift(self, arg):
        '''
         xpsGlobalSwitchControlSetL2EcmpHashPolynomialShift: Enter [ deviceId,instance,polynomialId,shift ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,polynomialId,shift ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, deviceId=%d, instance=%d, polynomialId=%d, shift=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsGlobalSwitchControlSetL2EcmpHashPolynomialShift(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetL2EcmpHashPolynomialShift
    #/********************************************************************************/
    def do_global_switch_control_get_l2_ecmp_hash_polynomial_shift(self, arg):
        '''
         xpsGlobalSwitchControlGetL2EcmpHashPolynomialShift: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            polynomialId_Ptr_2 = new_uint32_tp()
            shift_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetL2EcmpHashPolynomialShift(args[0],args[1],polynomialId_Ptr_2,shift_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('polynomialId = %d' % (uint32_tp_value(polynomialId_Ptr_2)))
                print('shift = %d' % (uint32_tp_value(shift_Ptr_3)))
                pass
            delete_uint32_tp(shift_Ptr_3)
            delete_uint32_tp(polynomialId_Ptr_2)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetL2EcmpHashOutputMask
    #/********************************************************************************/
    def do_global_switch_control_set_l2_ecmp_hash_output_mask(self, arg):
        '''
         xpsGlobalSwitchControlSetL2EcmpHashOutputMask: Enter [ deviceId,instance,outputMask ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,outputMask ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, deviceId=%d, instance=%d, outputMask=%d' % (args[0],args[1],args[2]))
            ret = xpsGlobalSwitchControlSetL2EcmpHashOutputMask(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetL2EcmpHashOutputMask
    #/********************************************************************************/
    def do_global_switch_control_get_l2_ecmp_hash_output_mask(self, arg):
        '''
         xpsGlobalSwitchControlGetL2EcmpHashOutputMask: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            outputMask_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetL2EcmpHashOutputMask(args[0],args[1],outputMask_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('outputMask = %d' % (uint32_tp_value(outputMask_Ptr_2)))
                pass
            delete_uint32_tp(outputMask_Ptr_2)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetL3EcmpHashPolynomialShift
    #/********************************************************************************/
    def do_global_switch_control_set_l3_ecmp_hash_polynomial_shift(self, arg):
        '''
         xpsGlobalSwitchControlSetL3EcmpHashPolynomialShift: Enter [ deviceId,instance,polynomialId,shift ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,polynomialId,shift ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, deviceId=%d, instance=%d, polynomialId=%d, shift=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsGlobalSwitchControlSetL3EcmpHashPolynomialShift(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetL3EcmpHashPolynomialShift
    #/********************************************************************************/
    def do_global_switch_control_get_l3_ecmp_hash_polynomial_shift(self, arg):
        '''
         xpsGlobalSwitchControlGetL3EcmpHashPolynomialShift: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            polynomialId_Ptr_2 = new_uint32_tp()
            shift_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetL3EcmpHashPolynomialShift(args[0],args[1],polynomialId_Ptr_2,shift_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('polynomialId = %d' % (uint32_tp_value(polynomialId_Ptr_2)))
                print('shift = %d' % (uint32_tp_value(shift_Ptr_3)))
                pass
            delete_uint32_tp(shift_Ptr_3)
            delete_uint32_tp(polynomialId_Ptr_2)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetL3EcmpHashOutputMask
    #/********************************************************************************/
    def do_global_switch_control_set_l3_ecmp_hash_output_mask(self, arg):
        '''
         xpsGlobalSwitchControlSetL3EcmpHashOutputMask: Enter [ deviceId,instance,outputMask ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance,outputMask ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, deviceId=%d, instance=%d, outputMask=%d' % (args[0],args[1],args[2]))
            ret = xpsGlobalSwitchControlSetL3EcmpHashOutputMask(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetL3EcmpHashOutputMask
    #/********************************************************************************/
    def do_global_switch_control_get_l3_ecmp_hash_output_mask(self, arg):
        '''
         xpsGlobalSwitchControlGetL3EcmpHashOutputMask: Enter [ deviceId,instance ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,instance ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            outputMask_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, instance=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetL3EcmpHashOutputMask(args[0],args[1],outputMask_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('outputMask = %d' % (uint32_tp_value(outputMask_Ptr_2)))
                pass
            delete_uint32_tp(outputMask_Ptr_2)

    #/********************************************************************************/
    # command for xpsGlobalSwitchControlEnableTimeStamp
    #/********************************************************************************/
    def do_global_switch_control_enable_time_stamp(self, arg):
        '''
         xpsGlobalSwitchControlEnableTimeStamp: Enter [ devId,portNum,enable,direction,remove ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,enable,direction,remove ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, portNum=%d, enable=%d, direction=%d, remove=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsGlobalSwitchControlEnableTimeStamp(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlEnableTimeStampReplace
    #/********************************************************************************/
    def do_global_switch_control_enable_time_stamp_replace(self, arg):
        '''
         xpsGlobalSwitchControlEnableTimeStampReplace: Enter [ devId,portNum,enable]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, portNum=%d, enable=%d, direction=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsGlobalSwitchControlEnableTimeStampReplace(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetTimeStampInfo
    #/********************************************************************************/
    def do_global_switch_control_get_time_stamp_info(self, arg):
        '''
         xpsGlobalSwitchControlGetTimeStampInfo: Enter [ devId,portNum,direction(0-egress, 1-ingress) ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,portNum,direction(0-egress, 1-ingress) ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            enable_ptr = new_uint32_tp()
            replace_ptr = new_uint32_tp()
            remove_ptr = new_uint32_tp()
            ret = xpsGlobalSwitchControlGetTimeStampInfo(args[0],args[1],args[2],enable_ptr,replace_ptr,remove_ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_ptr)))
                print('replace = %d' % (uint32_tp_value(replace_ptr)))
                print('remove = %d' % (uint32_tp_value(remove_ptr)))
                pass
            delete_uint32_tp(enable_ptr)
            delete_uint32_tp(replace_ptr)
            delete_uint32_tp(remove_ptr)


    #/********************************************************************************/
    # command for xpsGlobalSwitchControlEnableErrorPacketForwarding
    #/********************************************************************************/
    def do_global_switch_control_enable_error_packet_forwarding(self, arg):
        '''
         xpsGlobalSwitchControlEnableErrorPacketForwarding: Enter [ deviceId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, deviceId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlEnableErrorPacketForwarding(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetRxCutThruEnable
    #/********************************************************************************/
    def do_global_switch_control_set_rx_cut_thru_enable(self, arg):
        '''
         xpsGlobalSwitchControlSetRxCutThruEnable: Enter [ deviceId,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,portNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, deviceId=%d, portNum=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsGlobalSwitchControlSetRxCutThruEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetRxCutThruEnable
    #/********************************************************************************/
    def do_global_switch_control_get_rx_cut_thru_enable(self, arg):
        '''
         xpsGlobalSwitchControlGetRxCutThruEnable: Enter [ deviceId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetRxCutThruEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetTxCutThruEnable
    #/********************************************************************************/
    def do_global_switch_control_set_tx_cut_thru_enable(self, arg):
        '''
         xpsGlobalSwitchControlSetTxCutThruEnable: Enter [ deviceId,portNum,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,portNum,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, deviceId=%d, portNum=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsGlobalSwitchControlSetTxCutThruEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetTxCutThruEnable
    #/********************************************************************************/
    def do_global_switch_control_get_tx_cut_thru_enable(self, arg):
        '''
         xpsGlobalSwitchControlGetTxCutThruEnable: Enter [ deviceId,portNum ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,portNum ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, deviceId=%d, portNum=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlGetTxCutThruEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint32_tp_value(enable_Ptr_2)))
                pass
            delete_uint32_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlSetTxCutThruMode
    #/********************************************************************************/
    def do_global_switch_control_set_tx_cut_thru_mode(self, arg):
        '''
         xpsGlobalSwitchControlSetTxCutThruMode: Enter [ deviceId,bypassMode ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId,bypassMode ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, deviceId=%d, bypassMode=%d' % (args[0],args[1]))
            ret = xpsGlobalSwitchControlSetTxCutThruMode(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetTxCutThruMode
    #/********************************************************************************/
    def do_global_switch_control_get_tx_cut_thru_mode(self, arg):
        '''
         xpsGlobalSwitchControlGetTxCutThruMode: Enter [ deviceId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId ]')
        else:
            args[0] = int(args[0])
            bypassMode_Ptr_1 = new_xpBypassMode_ep()
            #print('Input Arguments are, deviceId=%d' % (args[0]))
            ret = xpsGlobalSwitchControlGetTxCutThruMode(args[0],bypassMode_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('bypassMode = %d' % (xpBypassMode_ep_value(bypassMode_Ptr_1)))
                pass
            delete_xpBypassMode_ep(bypassMode_Ptr_1)

    #/*****************************************************************************************************/
    # command for xpsGlobalSwitchControlSetCutThruDefaultPktSize
    #/*****************************************************************************************************/

    def do_global_switch_control_set_cut_thru_default_pkt_size(self, arg):
        '''
        xpsGlobalSwitchControlSetCutThruDefaultPktSize: Enter [ deviceId,defaultPktSize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,pktSize in bytes ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            ret = xpsGlobalSwitchControlSetCutThruDefaultPktSize(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/*****************************************************************************************************/
    # command for xpsGlobalSwitchControlGetCutThruDefaultPktSize
    #/*****************************************************************************************************/

    def do_global_switch_control_get_cut_thru_default_pkt_size(self, arg):
        '''
        xpsGlobalSwitchControlGetCutThruDefaultPktSize: Enter [ deviceId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        pktSize = new_uint32_tp()
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            ret = xpsGlobalSwitchControlGetCutThruDefaultPktSize(args[0],pktSize)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                print(' Packet Size = %d bytes' % (uint32_tp_value(pktSize)))
                pass
        delete_uint32_tp(pktSize)

    #/********************************************************************************/
    # command for xpsGlobalSwitchControlGetCpuPhysicalPortNum
    #/********************************************************************************/
    def do_global_switch_control_get_cpu_physical_port_num(self, arg):
        '''
         xpsGlobalSwitchControlGetCpuPhysicalPortNum: Enter [ deviceId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ deviceId ]')
        else:
            args[0] = int(args[0])
            portNum_Ptr_1 = new_uint8_tp()
            #print('Input Arguments are, deviceId=%d' % (args[0]))
            ret = xpsGlobalSwitchControlGetCpuPhysicalPortNum(args[0],portNum_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('portNum = %d' % (uint8_tp_value(portNum_Ptr_1)))
                pass
            delete_uint8_tp(portNum_Ptr_1)

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
