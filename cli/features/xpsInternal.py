#!/usr/bin/env python
#  xpsInternal.py
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
# The class object for xpsInternal operations
#/**********************************************************************************/

class xpsInternalObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/********************************************************************************/
    # command for xpsMirrorDbGetBktData
    #/********************************************************************************/
    def do_mirror_db_get_bkt_data(self, arg):
        '''
         xpsMirrorDbGetBktData: Enter [ scopeId,bucketId,bucketId,numSessions ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,bucketId,bucketId,numSessions ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsMirrorBktDbEntry_t_Ptr = new_xpsMirrorBktDbEntry_tp()
            xpsMirrorBktDbEntry_t_Ptr.bucketId = int(args[2])
            xpsMirrorBktDbEntry_t_Ptr.numSessions = int(args[3])
            #print('Input Arguments are, scopeId=%d, bucketId=%d, bucketId=%d, numSessions=%d' % (args[0],args[1],xpsMirrorBktDbEntry_t_Ptr.bucketId,xpsMirrorBktDbEntry_t_Ptr.numSessions))
            ret = xpsMirrorDbGetBktData(args[0],args[1],xpsMirrorBktDbEntry_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('bucketId = %d' % (xpsMirrorBktDbEntry_t_Ptr.bucketId))
                print('numSessions = %d' % (xpsMirrorBktDbEntry_t_Ptr.numSessions))
                pass
            delete_xpsMirrorBktDbEntry_tp(xpsMirrorBktDbEntry_t_Ptr)
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
                print('Return Value = %d' % (ret))
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
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsPVlanGetType
    #/********************************************************************************/
    def do_p_vlan_get_type(self, arg):
        '''
         xpsPVlanGetType: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            vlanType_Ptr_2 = new_xpsPrivateVlanType_ep()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsPVlanGetType(args[0],args[1],vlanType_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vlanType = %d' % (xpsPrivateVlanType_ep_value(vlanType_Ptr_2)))
                pass
            delete_xpsPrivateVlanType_ep(vlanType_Ptr_2)

    #/********************************************************************************/
    # command for xpsVlanIsExist
    #/********************************************************************************/
    def do_vlan_is_exist(self, arg):
        '''
         xpsVlanIsExist: Enter [ scopeId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2 
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanIsExist(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsVlanGetIntfTagTypeScope
    #/********************************************************************************/
    def do_vlan_get_intf_tag_type(self, arg):
        '''
         xpsVlanGetIntfTagTypeScope: Enter [ vlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ vlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            tagType_Ptr_2 = new_xpsL2EncapType_ep()
            #print('Input Arguments are, vlanId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsVlanGetIntfTagTypeScope(args[0],args[1],tagType_Ptr_2)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('tagType = %d' % (xpsL2EncapType_ep_value(tagType_Ptr_2)))
                pass
            delete_xpsL2EncapType_ep(tagType_Ptr_2)

    #/********************************************************************************/
    # command for xpsVlanGetEndpointInfo
    #/********************************************************************************/
    def do_vlan_get_endpoint_info(self, arg):
        '''
         xpsVlanGetEndpointInfo: Enter [ scopeId,vlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            tagType_Ptr_3 = new_xpsL2EncapType_ep()
            data_Ptr_4 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, vlanId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanGetEndpointInfo(args[0],args[1],args[2],tagType_Ptr_3,data_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tagType = %d' % (xpsL2EncapType_ep_value(tagType_Ptr_3)))
                print('data = %d' % (uint32_tp_value(data_Ptr_4)))
                pass
            delete_uint32_tp(data_Ptr_4)
            delete_xpsL2EncapType_ep(tagType_Ptr_3)
    #/********************************************************************************/
    # command for xpsVlanGetIntfTagType
    #/********************************************************************************/
    def do_vlan_get_intf_tag_type(self, arg):
        '''
         xpsVlanGetIntfTagType: Enter [ scopeId,vlanId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vlanId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            tagType_Ptr_3 = new_xpsL2EncapType_ep()
            #print('Input Arguments are, scopeId=%d, vlanId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanGetIntfTagType(args[0],args[1],args[2],tagType_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('tagType = %d' % (xpsL2EncapType_ep_value(tagType_Ptr_3)))
                pass
            delete_xpsL2EncapType_ep(tagType_Ptr_3)
    #/********************************************************************************/
    # Auto completion for vlanStgState enumerations
    #/********************************************************************************/
    def complete_vlan_set_ing_stp_state(self, text, line, begidx, endidx):
        tempDict = { 4 : 'vlanStgState'}
        return enumDict.autoCompletion(line, text, tempDict)

    #/********************************************************************************/
    # command for xpsVlanIsFdbLimitReached
    #/********************************************************************************/
    def do_vlan_is_fdb_limit_reached(self, arg):
        '''
         xpsVlanIsFdbLimitReached: Enter [ devId,vlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,vlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            isFdbLimitReached_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, vlanId=%d' % (args[0],args[1]))
            ret = xpsVlanIsFdbLimitReached(args[0],args[1],isFdbLimitReached_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('isFdbLimitReached = %d' % (uint8_tp_value(isFdbLimitReached_Ptr_2)))
                pass
            delete_uint8_tp(isFdbLimitReached_Ptr_2)
    #/********************************************************************************/
    # command for xpsVlanGetVlanMulticastVIF
    #/********************************************************************************/
    def do_vlan_get_vlan_multicast_vif(self, arg):
        '''
         xpsVlanGetVlanMulticastVIF: Enter [ scopeId,vlanId,encapType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vlanId,encapType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            mcVif_Ptr_3 = new_xpVif_tp()
            #print('Input Arguments are, scopeId=%d, vlanId=%d, encapType=%d' % (args[0],args[1],args[2]))
            ret = xpsVlanGetVlanMulticastVIF(args[0],args[1],args[2],mcVif_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('mcVif = %d' % (xpVif_tp_value(mcVif_Ptr_3)))
                pass
            delete_xpVif_tp(mcVif_Ptr_3)
    #/********************************************************************************/
    # command for xpsVlanHandlePortVlanTableRehash
    #/********************************************************************************/
    def do_vlan_handle_port_vlan_table_rehash(self, arg):
        '''
         xpsVlanHandlePortVlanTableRehash: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            indexList_Ptr_1 = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsVlanHandlePortVlanTableRehash(args[0],indexList_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('indexList = %d' % (xpsHashIndexList_tp_value(indexList_Ptr_1)))
                pass
            delete_xpsHashIndexList_tp(indexList_Ptr_1)
    #/********************************************************************************/
    # command for xpsStpIsExist
    #/********************************************************************************/
    def do_stp_is_exist(self, arg):
        '''
         xpsStpIsExist: Enter [ scopeId,stpId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,stpId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, scopeId=%d, stpId=%d' % (args[0],args[1]))
            ret = xpsStpIsExist(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsStgGetIntfStateDb
    #/********************************************************************************/
    def do_stg_get_intf_state_db(self, arg):
        '''
         xpsStgGetIntfStateDb: Enter [ scopeId,stgId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,stgId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            stpState_Ptr_3 = new_xpsStpState_ep()
            #print('Input Arguments are, scopeId=%d, stgId=%d, intfId=%d' % (args[0],args[1],args[2]))
            ret = xpsStgGetIntfStateDb(args[0],args[1],args[2],stpState_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('stpState = %d' % (xpsStpState_ep_value(stpState_Ptr_3)))
                pass
            delete_xpsStpState_ep(stpState_Ptr_3)
    #/********************************************************************************/
    # command for xpsMirrorDbGetBucketId
    #/********************************************************************************/
    def do_mirror_db_get_bucket_id(self, arg):
        '''
         xpsMirrorDbGetBucketId: Enter [ scopeId,analyzerId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,analyzerId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            bucketId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, analyzerId=%d' % (args[0],args[1]))
            ret = xpsMirrorDbGetBucketId(args[0],args[1],bucketId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('bucketId = %d' % (uint32_tp_value(bucketId_Ptr_2)))
                pass
            delete_uint32_tp(bucketId_Ptr_2)
    #/********************************************************************************/
    # command for xpsMirrorGetAnalyzerId
    #/********************************************************************************/
    def do_mirror_get_analyzer_id(self, arg):
        '''
         xpsMirrorGetAnalyzerId: Enter [ scopeId,bucketId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,bucketId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            analyzerId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, bucketId=%d' % (args[0],args[1]))
            ret = xpsMirrorGetAnalyzerId(args[0],args[1],analyzerId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('analyzerId = %d' % (uint32_tp_value(analyzerId_Ptr_2)))
                pass
            delete_uint32_tp(analyzerId_Ptr_2)
    #/********************************************************************************/
    # command for xpsL3HandleTunnelIvifTableRehash
    #/********************************************************************************/
    def do_l3_handle_tunnel_ivif_table_rehash(self, arg):
        '''
         xpsL3HandleTunnelIvifTableRehash: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            indexList_Ptr_1 = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsL3HandleTunnelIvifTableRehash(args[0],indexList_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('indexList = %d' % (xpsHashIndexList_tp_value(indexList_Ptr_1)))
                pass
            delete_xpsHashIndexList_tp(indexList_Ptr_1)
    #/********************************************************************************/
    # command for xpsPortSetPvidHw
    #/********************************************************************************/
    def do_port_set_pvid_hw(self, arg):
        '''
         xpsPortSetPvidHw: Enter [ devId,port,pvid ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,port,pvid ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, port=%d, pvid=%d' % (args[0],args[1],args[2]))
            ret = xpsPortSetPvidHw(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpIpTunnelRemove
    #/********************************************************************************/
    def do_xp_ip_tunnel_remove(self, arg):
        '''
         xpIpTunnelRemove: Enter [ devId,type,tnlIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,type,tnlIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, type=%d, tnlIntfId=%d' % (args[0],args[1],args[2]))
            ret = xpIpTunnelRemove(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIpTunnelGblDbGetData
    #/********************************************************************************/
    def do_ip_tunnel_gbl_db_get_data(self, arg):
        '''
         xpsIpTunnelGblDbGetData: Enter [ scopeId,intfId,lclEpIpAddr,rmtEpIpAddr,type,optionFormat,vpnLabel ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,intfId,lclEpIpAddr,rmtEpIpAddr,type,optionFormat,vpnLabel ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
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
            xpsIpTunnelData_t_Ptr.type = int(args[4])
            xpsIpTunnelData_t_Ptr.optionFormat = int(args[5])
            xpsIpTunnelData_t_Ptr.vpnLabel = int(args[6])
            #print('Input Arguments are, scopeId=%d, intfId=%d, lclEpIpAddr=%s, rmtEpIpAddr=%s, type=%d, optionFormat=%d, vpnLabel=%d' % (args[0],args[1],args[2],args[3],xpsIpTunnelData_t_Ptr.type,xpsIpTunnelData_t_Ptr.optionFormat,xpsIpTunnelData_t_Ptr.vpnLabel))
            ret = xpsIpTunnelGblDbGetData(args[0],args[1],xpsIpTunnelData_t_Ptr)
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
                print('optionFormat = %d' % (xpsIpTunnelData_t_Ptr.optionFormat))
                print('vpnLabel = %d' % (xpsIpTunnelData_t_Ptr.vpnLabel))
                pass
            delete_xpsIpTunnelData_tp(xpsIpTunnelData_t_Ptr)
            delete_ipv4Addr_tp(xpsIpTunnelData_t_Ptr.lclEpIpAddr)
    #/********************************************************************************/
    # command for xpsIpTunnelDbGetTunnelTermIndex
    #/********************************************************************************/
    def do_ip_tunnel_db_get_tunnel_term_index(self, arg):
        '''
         xpsIpTunnelDbGetTunnelTermIndex: Enter [ devId,intfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,intfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            index_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, intfId=%d' % (args[0],args[1]))
            ret = xpsIpTunnelDbGetTunnelTermIndex(args[0],args[1],index_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('index = %d' % (uint32_tp_value(index_Ptr_2)))
                pass
            delete_uint32_tp(index_Ptr_2)
    #/********************************************************************************/
    # command for xpsTunnelHandleTunnelIvifTableRehash
    #/********************************************************************************/
    def do_tunnel_handle_tunnel_ivif_table_rehash(self, arg):
        '''
         xpsTunnelHandleTunnelIvifTableRehash: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            indexList_Ptr_1 = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsTunnelHandleTunnelIvifTableRehash(args[0],indexList_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('indexList = %d' % (xpsHashIndexList_tp_value(indexList_Ptr_1)))
                pass
            delete_xpsHashIndexList_tp(indexList_Ptr_1)
    #/********************************************************************************/
    # command for xpsFdbAddVsiHwEntry
    #/********************************************************************************/
    def do_fdb_add_vsi_hw_entry(self, arg):
        '''
         xpsFdbAddVsiHwEntry: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            fdbEntry_Ptr_1 = new_xpsVsiFdbEntry_tp()
            indexList_Ptr_2 = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsFdbAddVsiHwEntry(args[0],fdbEntry_Ptr_1,indexList_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fdbEntry = %d' % (xpsVsiFdbEntry_tp_value(fdbEntry_Ptr_1)))
                print('indexList = %d' % (xpsHashIndexList_tp_value(indexList_Ptr_2)))
                pass
            delete_xpsHashIndexList_tp(indexList_Ptr_2)
            delete_xpsVsiFdbEntry_tp(fdbEntry_Ptr_1)
    #/********************************************************************************/
    # command for xpsVsiGetBdId
    #/********************************************************************************/
    def do_vsi_get_bd_id(self, arg):
        '''
         xpsVsiGetBdId: Enter [ scopeId,vsiId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vsiId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            bdId_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, scopeId=%d, vsiId=%d' % (args[0],args[1]))
            ret = xpsVsiGetBdId(args[0],args[1],bdId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('bdId = %d' % (uint32_tp_value(bdId_Ptr_2)))
                pass
            delete_uint32_tp(bdId_Ptr_2)
    #/********************************************************************************/
    # command for xpsVsiGetVsiIdFromBdId
    #/********************************************************************************/
    def do_vsi_get_vsi_id_from_bd_id(self, arg):
        '''
         xpsVsiGetVsiIdFromBdId: Enter [ scopeId,bdId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,bdId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            vsiId_Ptr_2 = new_xpsVsi_tp()
            #print('Input Arguments are, scopeId=%d, bdId=%d' % (args[0],args[1]))
            ret = xpsVsiGetVsiIdFromBdId(args[0],args[1],vsiId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vsiId = %d' % (xpsVsi_tp_value(vsiId_Ptr_2)))
                pass
            delete_xpsVsi_tp(vsiId_Ptr_2)
    #/********************************************************************************/
    # command for xpsVsiIsVsiBd
    #/********************************************************************************/
    def do_vsi_is_vsi_bd(self, arg):
        '''
         xpsVsiIsVsiBd: Enter [ scopeId,bdId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,bdId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            isVsi_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, scopeId=%d, bdId=%d' % (args[0],args[1]))
            ret = xpsVsiIsVsiBd(args[0],args[1],isVsi_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('isVsi = %d' % (uint8_tp_value(isVsi_Ptr_2)))
                pass
            delete_uint8_tp(isVsi_Ptr_2)
    #/********************************************************************************/
    # command for xpsVsiGetAccessMode
    #/********************************************************************************/
    def do_vsi_get_access_mode(self, arg):
        '''
         xpsVsiGetAccessMode: Enter [ scopeId,vsiId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,vsiId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            accessMode_Ptr_2 = new_xpsVsiAccessMode_ep()
            #print('Input Arguments are, scopeId=%d, vsiId=%d' % (args[0],args[1]))
            ret = xpsVsiGetAccessMode(args[0],args[1],accessMode_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('accessMode = %d' % (xpsVsiAccessMode_ep_value(accessMode_Ptr_2)))
                pass
            delete_xpsVsiAccessMode_ep(accessMode_Ptr_2)
    #/********************************************************************************/
    # command for xpsVsiServiceInstanceCreate
    #/********************************************************************************/
    def do_vsi_service_instance_create(self, arg):
        '''
         xpsVsiServiceInstanceCreate: Enter [ devId,serviceId,serviceType,vsiId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,serviceId,serviceType,vsiId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            data_Ptr_4 = new_xpsServiceIdData_tp()
            #print('Input Arguments are, devId=%d, serviceId=%d, serviceType=%d, vsiId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsVsiServiceInstanceCreate(args[0],args[1],args[2],args[3],data_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('data = %d' % (xpsServiceIdData_tp_value(data_Ptr_4)))
                pass
            delete_xpsServiceIdData_tp(data_Ptr_4)
    #/********************************************************************************/
    # command for xpsAcIsPresentInDevice
    #/********************************************************************************/
    def do_ac_is_present_in_device(self, arg):
        '''
         xpsAcIsPresentInDevice: Enter [ devId,acIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,acIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, acIntfId=%d' % (args[0],args[1]))
            ret = xpsAcIsPresentInDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcGetAcMatchType
    #/********************************************************************************/
    def do_ac_get_ac_match_type(self, arg):
        '''
         xpsAcGetAcMatchType: Enter [ scopeId,acIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,acIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            matchType_Ptr_2 = new_xpsAcMatchType_ep()
            #print('Input Arguments are, scopeId=%d, acIntfId=%d' % (args[0],args[1]))
            ret = xpsAcGetAcMatchType(args[0],args[1],matchType_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('matchType = %d' % (xpsAcMatchType_ep_value(matchType_Ptr_2)))
                pass
            delete_xpsAcMatchType_ep(matchType_Ptr_2)
    #/********************************************************************************/
    # command for xpsAcGetPhysInterface
    #/********************************************************************************/
    def do_ac_get_phys_interface(self, arg):
        '''
         xpsAcGetPhysInterface: Enter [ scopeId,acIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,acIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            physIntfId_Ptr_2 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, scopeId=%d, acIntfId=%d' % (args[0],args[1]))
            ret = xpsAcGetPhysInterface(args[0],args[1],physIntfId_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('physIntfId = %d' % (xpsInterfaceId_tp_value(physIntfId_Ptr_2)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(physIntfId_Ptr_2))
                pass
            delete_xpsInterfaceId_tp(physIntfId_Ptr_2)
    #/********************************************************************************/
    # command for xpsAcGetAcVid
    #/********************************************************************************/
    def do_ac_get_ac_vid(self, arg):
        '''
         xpsAcGetAcVid: Enter [ scopeId,acIntfId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId,acIntfId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            vid_Ptr_2 = new_xpsVlan_tp()
            #print('Input Arguments are, scopeId=%d, acIntfId=%d' % (args[0],args[1]))
            ret = xpsAcGetAcVid(args[0],args[1],vid_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('vid = %d' % (xpsVlan_tp_value(vid_Ptr_2)))
                pass
            delete_xpsVlan_tp(vid_Ptr_2)
    #/********************************************************************************/
    # command for xpsAcBindWithVsi
    #/********************************************************************************/
    def do_ac_bind_with_vsi(self, arg):
        '''
         xpsAcBindWithVsi: Enter [ devId,acIntfId,vsiId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,acIntfId,vsiId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, acIntfId=%d, vsiId=%d' % (args[0],args[1],args[2]))
            ret = xpsAcBindWithVsi(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcUnbindWithVsi
    #/********************************************************************************/
    def do_ac_unbind_with_vsi(self, arg):
        '''
         xpsAcUnbindWithVsi: Enter [ devId,acIntfId,vsiId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,acIntfId,vsiId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, acIntfId=%d, vsiId=%d' % (args[0],args[1],args[2]))
            ret = xpsAcUnbindWithVsi(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcConfigurePvidOnInterface
    #/********************************************************************************/
    def do_ac_configure_pvid_on_interface(self, arg):
        '''
         xpsAcConfigurePvidOnInterface: Enter [ devId,interfaceId,pvid ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,interfaceId,pvid ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, interfaceId=%d, pvid=%d' % (args[0],args[1],args[2]))
            ret = xpsAcConfigurePvidOnInterface(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcRemovePvidFromInterface
    #/********************************************************************************/
    def do_ac_remove_pvid_from_interface(self, arg):
        '''
         xpsAcRemovePvidFromInterface: Enter [ devId,interfaceId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,interfaceId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, interfaceId=%d' % (args[0],args[1]))
            ret = xpsAcRemovePvidFromInterface(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsAcIsPvidExistForInterface
    #/********************************************************************************/
    def do_ac_is_pvid_exist_for_interface(self, arg):
        '''
         xpsAcIsPvidExistForInterface: Enter [ devId,interfaceId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,interfaceId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            exists_Ptr_2 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, interfaceId=%d' % (args[0],args[1]))
            ret = xpsAcIsPvidExistForInterface(args[0],args[1],exists_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('exists = %d' % (uint8_tp_value(exists_Ptr_2)))
                pass
            delete_uint8_tp(exists_Ptr_2)
    #/********************************************************************************/
    # command for xpsAcHandlePortVlanTableRehash
    #/********************************************************************************/
    def do_ac_handle_port_vlan_table_rehash(self, arg):
        '''
         xpsAcHandlePortVlanTableRehash: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            indexList_Ptr_1 = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsAcHandlePortVlanTableRehash(args[0],indexList_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('indexList = %d' % (xpsHashIndexList_tp_value(indexList_Ptr_1)))
                pass
            delete_xpsHashIndexList_tp(indexList_Ptr_1)
    #/********************************************************************************/
    # command for xpsAcHandleTunnelIvifTableRehash
    #/********************************************************************************/
    def do_ac_handle_tunnel_ivif_table_rehash(self, arg):
        '''
         xpsAcHandleTunnelIvifTableRehash: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            indexList_Ptr_1 = new_xpsHashIndexList_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsAcHandleTunnelIvifTableRehash(args[0],indexList_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('indexList = %d' % (xpsHashIndexList_tp_value(indexList_Ptr_1)))
                pass
            delete_xpsHashIndexList_tp(indexList_Ptr_1)
    #/********************************************************************************/
    # command for xpsAcResolveIngressAc
    #/********************************************************************************/
    def do_ac_resolve_ingress_ac(self, arg):
        '''
         xpsAcResolveIngressAc: Enter [ devId,physIntf,isTagged,tagVlanId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,physIntf,isTagged,tagVlanId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            acIntfId_Ptr_4 = new_xpsInterfaceId_tp()
            #print('Input Arguments are, devId=%d, physIntf=%d, isTagged=%d, tagVlanId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsAcResolveIngressAc(args[0],args[1],args[2],args[3],acIntfId_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('acIntfId = %d' % (xpsInterfaceId_tp_value(acIntfId_Ptr_4)))
                xpShellGlobals.cmdRetVal = (xpsInterfaceId_tp_value(acIntfId_Ptr_4))
                pass
            delete_xpsInterfaceId_tp(acIntfId_Ptr_4)
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
