#!/usr/bin/env python
#  xpsCommon.py
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
# The class object for xpsCommon operations
#/**********************************************************************************/

class xpsCommonObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # Auto completion for vlanStgState enumerations
    #/********************************************************************************/
    def complete_vlan_set_ing_stp_state(self, text, line, begidx, endidx):
        tempDict = { 4 : 'vlanStgState'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanSetIngStpState
    #/********************************************************************************/
    def do_vlan_set_ing_stp_state(self, arg):
        '''
         xpsVlanSetIngStpState: Enter [ devId,vlanId,intfId,stpState ]
         Valid values for stpState : <vlanStgState>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,intfId,stpState ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, vlanId=%d, intfId=%d, stpState=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsVlanSetIngStpState(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanSetRouterAclEnable
    #/********************************************************************************/
    def do_vlan_set_router_acl_enable(self, arg):
        '''
         xpsVlanSetRouterAclEnable: Enter [ devId,vlanId,enable ]
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
            ret = xpsVlanSetRouterAclEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanSetRouterAclId
    #/********************************************************************************/
    def do_vlan_set_router_acl_id(self, arg):
        '''
         xpsVlanSetRouterAclId: Enter [ devId,vlanId,aclId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId,aclId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vlanId=%d, aclId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanSetRouterAclId(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanGetRouterAclEnable
    #/********************************************************************************/
    def do_vlan_get_router_acl_enable(self, arg):
        '''
         xpsVlanGetRouterAclEnable: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            enable_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetRouterAclEnable(args[0],args[1],enable_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('enable = %d' % (uint8_tp_value(enable_Ptr_2)))
                pass
            delete_uint8_tp(enable_Ptr_2)
    #/********************************************************************************/
    # command for xpsVlanGetRouterAclId
    #/********************************************************************************/
    def do_vlan_get_router_acl_id(self, arg):
        '''
         xpsVlanGetRouterAclId: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            aclId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanGetRouterAclId(args[0],args[1],aclId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('aclId = %d' % (uint32_tp_value(aclId_Ptr_2)))
                pass
            delete_uint32_tp(aclId_Ptr_2)
    #/********************************************************************************/
    # command for xpsVlanIncrementFdbCount
    #/********************************************************************************/
    def do_vlan_increment_fdb_count(self, arg):
        '''
         xpsVlanIncrementFdbCount: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanIncrementFdbCount(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanDecrementFdbCount
    #/********************************************************************************/
    def do_vlan_decrement_fdb_count(self, arg):
        '''
         xpsVlanDecrementFdbCount: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanDecrementFdbCount(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPVlanAddFdb
    #/********************************************************************************/
    def do_p_vlan_add_fdb(self, arg):
        '''
         xpsPVlanAddFdb: Enter [ devId,vlanType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanType ]')
        else:
            args[0] = int(args[0])
            fdbEntry_Ptr_1 = new_xpsFdbEntry_tp()
            args[1] = eval(args[1])
            indexList_Ptr_3 = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d, vlanType=%d' % (args[0],args[1]))
            ret = xpsPVlanAddFdb(args[0],fdbEntry_Ptr_1,args[1],indexList_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fdbEntry = %d' % (xpsFdbEntry_tp_value(fdbEntry_Ptr_1)))
                print('indexList = %d' % (xpsHashIndexList_tp_value(indexList_Ptr_3)))
                pass
            delete_xpsHashIndexList_tp(indexList_Ptr_3)
            delete_xpsFdbEntry_tp(fdbEntry_Ptr_1)
    #/********************************************************************************/
    # command for xpsAclAllocateAcmBanks
    #/********************************************************************************/
    def do_acl_allocate_acm_banks(self, arg):
        '''
         xpsAclAllocateAcmBanks: Enter [ devId,aclType,numDbs,keySize ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,aclType,numDbs,keySize ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            numBanks_Ptr_4 = new_uint8_tp()
            bankStartPipe0_Ptr_5 = new_uint8_tp()
            bankStartPipe1_Ptr_6 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, aclType=%d, numDbs=%d, keySize=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsAclAllocateAcmBanks(args[0],args[1],args[2],args[3],numBanks_Ptr_4,bankStartPipe0_Ptr_5,bankStartPipe1_Ptr_6)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numBanks = %d' % (uint8_tp_value(numBanks_Ptr_4)))
                print('bankStartPipe0 = %d' % (uint8_tp_value(bankStartPipe0_Ptr_5)))
                print('bankStartPipe1 = %d' % (uint8_tp_value(bankStartPipe1_Ptr_6)))
                pass
            delete_uint8_tp(bankStartPipe1_Ptr_6)
            delete_uint8_tp(bankStartPipe0_Ptr_5)
            delete_uint8_tp(numBanks_Ptr_4)
    #/********************************************************************************/
    # command for xpsAclReleaseAcmBanks
    #/********************************************************************************/
    def do_acl_release_acm_banks(self, arg):
        '''
         xpsAclReleaseAcmBanks: Enter [ devId,aclType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,aclType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, aclType=%d' % (args[0],args[1]))
            ret = xpsAclReleaseAcmBanks(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsStgAddVlanDb
    #/********************************************************************************/
    def do_stg_add_vlan_db(self, arg):
        '''
         xpsStgAddVlanDb: Enter [ scopeId,stgId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,stgId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, stgId=%d, vlanId=%d' % (args[0],args[1],args[2]))
            ret = xpsStgAddVlanDb(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsStgRemoveVlanDb
    #/********************************************************************************/
    def do_stg_remove_vlan_db(self, arg):
        '''
         xpsStgRemoveVlanDb: Enter [ scopeId,stgId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,stgId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, scopeId=%d, stgId=%d, vlanId=%d' % (args[0],args[1],args[2]))
            ret = xpsStgRemoveVlanDb(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # Auto completion for vlanStgState enumerations
    #/********************************************************************************/
    def complete_stp_set_ing_state(self, text, line, begidx, endidx):
        tempDict = { 4 : 'vlanStgState'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsStpSetIngState
    #/********************************************************************************/
    def do_stp_set_ing_state(self, arg):
        '''
         xpsStpSetIngState: Enter [ devId,stpId,intfId,stpState ]
         Valid values for stpState : <vlanStgState>
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,stpId,intfId,stpState ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, stpId=%d, intfId=%d, stpState=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsStpSetIngState(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanAddPortToLag
    #/********************************************************************************/
    def do_vlan_add_port_to_lag(self, arg):
        '''
         xpsVlanAddPortToLag: Enter [ devId,lagIntfId,portIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntfId,portIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, lagIntfId=%d, portIntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanAddPortToLag(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVlanRemovePortFromLag
    #/********************************************************************************/
    def do_vlan_remove_port_from_lag(self, arg):
        '''
         xpsVlanRemovePortFromLag: Enter [ devId,lagIntfId,portIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,lagIntfId,portIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, lagIntfId=%d, portIntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanRemovePortFromLag(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelCreate
    #/********************************************************************************/
    def do_ip_tunnel_create(self, arg):
        '''
         xpsIpTunnelCreate: Enter [ scopeId,type,lclEpIpAddr,rmtEpIpAddr,type,vpnLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,type,lclEpIpAddr,rmtEpIpAddr,type,vpnLabel ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            xpsIpTunnelData_t_Ptr = new_xpsIpTunnelData_tp()

            args[2] = args[2].replace(".",":").replace(",",":")
            postList = args[2].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsIpTunnelData_t_Ptr.lclEpIpAddr[listLen - ix - 1] = int(postList[ix])

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsIpTunnelData_t_Ptr.rmtEpIpAddr[listLen - ix - 1] = int(postList[ix])
            xpsIpTunnelData_t_Ptr.type = eval(args[4])
            xpsIpTunnelData_t_Ptr.vpnLabel = eval(args[5])
            tnlIntfId_Ptr_6 = new_xpsInterfaceId_tp()

            #print('Input Arguments are, scopeId=%d, type=%d, lclEpIpAddr=%s, rmtEpIpAddr=%s, type=%d, vpnLabel=%d' % (args[0],args[1],args[2],args[3],xpsIpTunnelData_t_Ptr.type,xpsIpTunnelData_t_Ptr.vpnLabel))
            ret = xpsIpTunnelCreate(args[0],args[1],xpsIpTunnelData_t_Ptr,tnlIntfId_Ptr_6)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lclEpIpAddr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsIpTunnelData_t_Ptr.lclEpIpAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('rmtEpIpAddr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsIpTunnelData_t_Ptr.rmtEpIpAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('type = %d' % (xpsIpTunnelData_t_Ptr.type))
                print('vpnLabel = %d' % (xpsIpTunnelData_t_Ptr.vpnLabel))
                print('tnlIntfId = %d' % (xpsInterfaceId_tp_value(tnlIntfId_Ptr_6)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(tnlIntfId_Ptr_6))
                pass
            delete_xpsInterfaceId_tp(tnlIntfId_Ptr_6)
            delete_xpsIpTunnelData_tp(xpsIpTunnelData_t_Ptr)
            delete_ipv4Addr_tp(xpsIpTunnelData_t_Ptr.lclEpIpAddr)
    #/********************************************************************************/
    # command for xpsIpTunnelDelete
    #/********************************************************************************/
    def do_ip_tunnel_delete(self, arg):
        '''
         xpsIpTunnelDelete: Enter [ scopeId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsIpTunnelDelete(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelAdd
    #/********************************************************************************/
    def do_ip_tunnel_add(self, arg):
        '''
         xpsIpTunnelAdd: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsIpTunnelAdd(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelAddOrigination
    #/********************************************************************************/
    def do_ip_tunnel_add_origination(self, arg):
        '''
         xpsIpTunnelAddOrigination: Enter [ devId,intfId,baseIntfId,lclEpIpAddr,rmtEpIpAddr,type,vpnLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,baseIntfId,lclEpIpAddr,rmtEpIpAddr,type,vpnLabel ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            xpsIpTunnelData_t_Ptr = new_xpsIpTunnelData_tp()

            args[3] = args[3].replace(".",":").replace(",",":")
            postList = args[3].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsIpTunnelData_t_Ptr.lclEpIpAddr[listLen - ix - 1] = int(postList[ix])

            args[4] = args[4].replace(".",":").replace(",",":")
            postList = args[4].strip(",").strip("'").strip("]").strip("[").split(":")
            listLen = len(postList)
            for ix in range(listLen-1, -1, -1):
                xpsIpTunnelData_t_Ptr.rmtEpIpAddr[listLen - ix - 1] = int(postList[ix])
            xpsIpTunnelData_t_Ptr.type = eval(args[5])
            xpsIpTunnelData_t_Ptr.vpnLabel = eval(args[6])

            #print('Input Arguments are, devId=%d, intfId=%d, baseIntfId=%d, lclEpIpAddr=%s, rmtEpIpAddr=%s, type=%d, vpnLabel=%d' % (args[0],args[1],args[2],args[3],args[4],xpsIpTunnelData_t_Ptr.type,xpsIpTunnelData_t_Ptr.vpnLabel))
            ret = xpsIpTunnelAddOrigination(args[0],args[1],args[2],xpsIpTunnelData_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('lclEpIpAddr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsIpTunnelData_t_Ptr.lclEpIpAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('rmtEpIpAddr = '),
                for ix in range(0, listLen):
                    print('%02x' % (xpsIpTunnelData_t_Ptr.rmtEpIpAddr[ix])),
                    if ix < (listLen-1):
                        sys.stdout.write(':'),
                print('')
                print('type = %d' % (xpsIpTunnelData_t_Ptr.type))
                print('vpnLabel = %d' % (xpsIpTunnelData_t_Ptr.vpnLabel))
                pass
            delete_xpsIpTunnelData_tp(xpsIpTunnelData_t_Ptr)
            delete_ipv4Addr_tp(xpsIpTunnelData_t_Ptr.lclEpIpAddr)
    #/********************************************************************************/
    # command for xpsErspanTunnelAdd
    #/********************************************************************************/
    def do_erspan_tunnel_add(self, arg):
        '''
         xpsErspanTunnelAdd: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsErspanTunnelAdd(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelGetConfig
    #/********************************************************************************/
    def do_ip_tunnel_get_config(self, arg):
        '''
         xpsIpTunnelGetConfig: Enter [ devId,tnlIntfId,type ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,type ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            cfg_Ptr_3 = new_xpsIpTunnelConfig_tp()
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, type=%d' % (args[0],args[1],args[2]))
            ret = xpsIpTunnelGetConfig(args[0],args[1],args[2],cfg_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('cfg = %d' % (xpsIpTunnelConfig_tp_value(cfg_Ptr_3)))
                pass
            delete_xpsIpTunnelConfig_tp(cfg_Ptr_3)
    #/********************************************************************************/
    # command for xpsIpTunnelDbAddEntry
    #/********************************************************************************/
    def do_ip_tunnel_db_add_entry(self, arg):
        '''
         xpsIpTunnelDbAddEntry: Enter [ devId,intfId,tnlTableIndex,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId,tnlTableIndex,nhId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            #print('Input Arguments are, devId=%d, intfId=%d, tnlTableIndex=%d, nhId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsIpTunnelDbAddEntry(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelDbRemoveEntry
    #/********************************************************************************/
    def do_ip_tunnel_db_remove_entry(self, arg):
        '''
         xpsIpTunnelDbRemoveEntry: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsIpTunnelDbRemoveEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTnlGblDbAddOptionalInterface
    #/********************************************************************************/
    def do_ip_tnl_gbl_db_add_optional_interface(self, arg):
        '''
         xpsIpTnlGblDbAddOptionalInterface: Enter [ devId,baseIntfId,optIntfId ]
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
            ret = xpsIpTnlGblDbAddOptionalInterface(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpGreTunnelAttachL3Intf
    #/********************************************************************************/
    def do_ip_gre_tunnel_attach_l3_intf(self, arg):
        '''
         xpsIpGreTunnelAttachL3Intf: Enter [ devId,tnlIntfId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, l3IntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsIpGreTunnelAttachL3Intf(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpGreTunnelDetachL3Intf
    #/********************************************************************************/
    def do_ip_gre_tunnel_detach_l3_intf(self, arg):
        '''
         xpsIpGreTunnelDetachL3Intf: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsIpGreTunnelDetachL3Intf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVpnGreStrictModeTunnelAttachL3Intf
    #/********************************************************************************/
    def do_vpn_gre_strict_mode_tunnel_attach_l3_intf(self, arg):
        '''
         xpsVpnGreStrictModeTunnelAttachL3Intf: Enter [ devId,tnlIntfId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, l3IntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsVpnGreStrictModeTunnelAttachL3Intf(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsVpnGreStrictModeTunnelDetachL3Intf
    #/********************************************************************************/
    def do_vpn_gre_strict_mode_tunnel_detach_l3_intf(self, arg):
        '''
         xpsVpnGreStrictModeTunnelDetachL3Intf: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsVpnGreStrictModeTunnelDetachL3Intf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpinIpTunnelAttachL3Intf
    #/********************************************************************************/
    def do_ipin_ip_tunnel_attach_l3_intf(self, arg):
        '''
         xpsIpinIpTunnelAttachL3Intf: Enter [ devId,tnlIntfId,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, l3IntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsIpinIpTunnelAttachL3Intf(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpinIpTunnelDetachL3Intf
    #/********************************************************************************/
    def do_ipin_ip_tunnel_detach_l3_intf(self, arg):
        '''
         xpsIpinIpTunnelDetachL3Intf: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsIpinIpTunnelDetachL3Intf(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelSetConfig
    #/********************************************************************************/
    def do_ip_tunnel_set_config(self, arg):
        '''
         xpsIpTunnelSetConfig: Enter [ devId,tnlIntfId,type,cfg ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId,type,cfg ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d, type=%d, cfg=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsIpTunnelSetConfig(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelUpdateNexthop
    #/********************************************************************************/
    def do_ip_tunnel_update_nexthop(self, arg):
        '''
         xpsIpTunnelUpdateNexthop: Enter [ devId,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, tnlIntfId=%d' % (args[0],args[1]))
            ret = xpsIpTunnelUpdateNexthop(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelGetRemoteIp
    #/********************************************************************************/
    def do_ip_tunnel_get_remote_ip(self, arg):
        '''
         xpsIpTunnelGetRemoteIp: Enter [ devId,type,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,type,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            rmtEpIpAddr_Ptr_3 = new_ipv4Addr_tp()
            #print('Input Arguments are, devId=%d, type=%d, tnlIntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsIpTunnelGetRemoteIp(args[0],args[1],args[2],rmtEpIpAddr_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('rmtEpIpAddr = %d' % (ipv4Addr_tp_value(rmtEpIpAddr_Ptr_3)))
                pass
            delete_ipv4Addr_tp(rmtEpIpAddr_Ptr_3)
    #/********************************************************************************/
    # command for xpsIpTunnelRemoveLocalVtep
    #/********************************************************************************/
    def do_ip_tunnel_remove_local_vtep(self, arg):
        '''
         xpsIpTunnelRemoveLocalVtep: Enter [ devId,localIp,tnlType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,localIp,tnlType ]')
        else:
            args[0] = int(args[0])
            args[1] = args[1].replace(".", ":").replace(",", ":")
            localIpList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(localIpList)
            localIp = ipv4Addr_t()
            for ix in range(listLen-1, -1, -1):
                localIp[listLen - ix - 1] = int(localIpList[ix])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, localIp=%s, tnlType=%d' % (args[0],args[1],args[2]))
            ret = xpsIpTunnelRemoveLocalVtep(args[0],localIp,args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelAddLocalVtep
    #/********************************************************************************/
    def do_ip_tunnel_add_local_vtep(self, arg):
        '''
         xpsIpTunnelAddLocalVtep: Enter [ devId,localIp,tnlType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,localIp,tnlType ]')
        else:
            args[0] = int(args[0])
            args[1] = args[1].replace(".", ":").replace(",", ":")
            localIpList = args[1].strip("'").strip("]").strip("[").split(":")
            listLen = len(localIpList)
            localIp = ipv4Addr_t()
            for ix in range(listLen-1, -1, -1):
                localIp[listLen - ix - 1] = int(localIpList[ix])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, localIp=%s, tnlType=%d' % (args[0],args[1],args[2]))
            ret = xpsIpTunnelAddLocalVtep(args[0],localIp,args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTnlSetMcOIF
    #/********************************************************************************/
    def do_ip_tnl_set_mc_oif(self, arg):
        '''
         xpsIpTnlSetMcOIF: Enter [ devId,tunIntfId,tunnelData,l3IntfId,egressIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,tunIntfId,tunnelData,l3IntfId,egressIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsIpMcOIFData_t_Ptr = new_xpsIpMcOIFData_tp()
            xpsIpMcOIFData_t_Ptr.tunnelData = int(args[2], 16)
            xpsIpMcOIFData_t_Ptr.l3IntfId = int(args[3])
            xpsIpMcOIFData_t_Ptr.egressIntfId = int(args[4])

            #print('Input Arguments are, devId=%d, tunIntfId=%d, tunnelData=0x%x, l3IntfId=%d, egressIntfId=%d' % (args[0],args[1],xpsIpMcOIFData_t_Ptr.tunnelData,xpsIpMcOIFData_t_Ptr.l3IntfId,xpsIpMcOIFData_t_Ptr.egressIntfId))
            ret = xpsIpTnlSetMcOIF(args[0],args[1],xpsIpMcOIFData_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tunnelData = %d' % (xpsIpMcOIFData_t_Ptr.tunnelData))
                print('l3IntfId = %d' % (xpsIpMcOIFData_t_Ptr.l3IntfId))
                print('egressIntfId = %d' % (xpsIpMcOIFData_t_Ptr.egressIntfId))
                pass
            delete_xpsIpMcOIFData_tp(xpsIpMcOIFData_t_Ptr)
    #/********************************************************************************/
    # command for xpsIpTnlSetNexthopData
    #/********************************************************************************/
    def do_ip_tnl_set_nexthop_data(self, arg):
        '''
         xpsIpTnlSetNexthopData: Enter [ devId,vifId,nhId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vifId,nhId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vifId=%d, nhId=%d' % (args[0],args[1],args[2]))
            ret = xpsIpTnlSetNexthopData(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMplsRemoveTunnelOrigination
    #/********************************************************************************/
    def do_mpls_remove_tunnel_origination(self, arg):
        '''
         xpsMplsRemoveTunnelOrigination: Enter [ devId,mplsTnlId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,mplsTnlId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, mplsTnlId=%d' % (args[0],args[1]))
            ret = xpsMplsRemoveTunnelOrigination(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMplsAddVpnEntry
    #/********************************************************************************/
    def do_mpls_add_vpn_entry(self, arg):
        '''
         xpsMplsAddVpnEntry: Enter [ devId,vpnLabel,l3IntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vpnLabel,l3IntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, vpnLabel=%d, l3IntfId=%d' % (args[0],args[1],args[2]))
            ret = xpsMplsAddVpnEntry(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMplsRemoveVpnEntry
    #/********************************************************************************/
    def do_mpls_remove_vpn_entry(self, arg):
        '''
         xpsMplsRemoveVpnEntry: Enter [ devId,vpnLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vpnLabel ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, vpnLabel=%d' % (args[0],args[1]))
            ret = xpsMplsRemoveVpnEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsMirrorDbAddAnalyzer
    #/********************************************************************************/
    def do_mirror_db_add_analyzer(self, arg):
        '''
         xpsMirrorDbAddAnalyzer: Enter [ scopeId,analyzerId,bucketId,mirrorType,mirrorData ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,analyzerId,bucketId,mirrorType,mirrorData ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            args[4] = int(args[4], 16)
            #print('Input Arguments are, scopeId=%d, analyzerId=%d, bucketId=%d, mirrorType=%d, mirrorData=0x%x' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsMirrorDbAddAnalyzer(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsFdbAddHwEntry
    #/********************************************************************************/
    def do_fdb_add_hw_entry(self, arg):
        '''
         xpsFdbAddHwEntry: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            fdbEntry_Ptr_1 = new_xpsFdbEntry_tp()
            indexList_Ptr_2 = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsFdbAddHwEntry(args[0],fdbEntry_Ptr_1,indexList_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fdbEntry = %d' % (xpsFdbEntry_tp_value(fdbEntry_Ptr_1)))
                print('indexList = %d' % (xpsHashIndexList_tp_value(indexList_Ptr_2)))
                pass
            delete_xpsHashIndexList_tp(indexList_Ptr_2)
            delete_xpsFdbEntry_tp(fdbEntry_Ptr_1)
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
                print('Return Value = %d' % (ret))
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
