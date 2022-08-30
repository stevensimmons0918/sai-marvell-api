#!/usr/bin/env python
#  xpsIacl.py
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
# The class object for xpsIacl operations
#/**********************************************************************************/
mapxpIaclV4KeyByteMask = {
    0: 0x1,  #'XP_IACL_KEY_TYPE_V4'
    1: 0x1,  #
    2: 0x3f, #XP_IACL_MAC_DA
    3: 0x3f, #XP_IACL_MAC_SA
    4: 0x3,  #XP_IACL_V4_ETHER_TYPE
    5: 0x3,  #XP_IACL_CTAG_VID_DEI_PCP
    6: 0x3,  #XP_IACL_STAG_VID_DEI_PCP
    7: 0xf,  #XP_IACL_DIP_V4
    8: 0xf,  #XP_IACL_SIP_V4
    9: 0x3,  #XP_IACL_L4_DEST_PORT
    10: 0x3, #XP_IACL_L4_SRC_PORT
    11: 0x3, #XP_IACL_IVIF
    12: 0x1, #XP_IACL_ICMP_CODE,     
    13: 0x1, #XP_IACL_ICMP_MSG_TYPE   
    14: 0x1, #XP_IACL_PROTOCOL
    15: 0x1, #XP_IACL_DSCP_HAS_CTAG_STAG
    16: 0x3, #XP_IACL_BD
    17: 0x1, #XP_IACL_TTL,            
    18: 0x1, #XP_IACL_PKTLEN,        
    19: 0x1, #XP_IACL_TCP_FLAGS,    
    20: 0x3, #XP_IACL_VRF_ID,      
    21: 0x1, #XP_IACL_TAG_FRAGMENT_INFO,
}
mapxpIaclV6KeyByteMask = {
    0: 0x1,   #XP_IACL_KEY_TYPE_V6
    1: 0x1,   #XP_IACL_V6_ID
    2: 0xffff,#XP_IACL_DIP_V6
    3: 0xffff,#XP_IACL_SIP_V6
    4: 0x1,   #XP_IACL_NXT_HDR
    5: 0x3,   #XP_IACL_L4_V6_DEST_PORT
    6: 0x3,   #XP_IACL_L4_V6_SRC_PORT
    7: 0x1,   #XP_IACL_ICMP_V6_MSG_TYPE
    8: 0x1,   #XP_IACL_HOP_LIMIT
    9: 0x1,  #XP_IACL_ROUTER_MAC_ISTCP_ISUDP
    10: 0x1, #XP_IACL_V6_PROTOCOL,    //8 
    11: 0x1, #XP_IACL_V6_TCP_FLAGS,   //8     
    12: 0x1, #XP_IACL_V6_PKTLEN,     //8 
    13: 0x1, #XP_IACL_TC_ROUTERMAC,   //8
};

mapxpIaclTypeKeyByteMask = {
   0:{
    0:  0x0, #start
    1:  0x1, #keyType
    2:  0x1, #aclId
    3:  0x3f,#DMAC
    4:  0x3f,#SMAC
    5:  0xf, #DIP
    6:  0xf, #SIP
    7:  0x3, #ETHER TYPE
    8:  0x3, #C-Tag Virtual ID + DEI + PCP
    9:  0x3, #S-Tag Virtual ID + DEI + PCP
    10: 0x3, #L4 destport
    11: 0x3, #L4 srcport
    12: 0x3, #ivif
    13: 0x1, #icmpCode
    14: 0x1, #icmpType
    15: 0x1, #protocol
    16: 0x1, #dscpEcn
    17: 0x3, #bd
    18: 0x1, #ttl
    19: 0x1, #pktLen
    20: 0x1, #tcpflag
    21: 0x3, #vrfId
    22: 0x1, #tagfragment info + pkt info
    23: 0x1, #Router MAC & Traffic Class
    24: 0x1F,#Packet Len & MPLS Label and EXP
    25: 0x1, #source port
    26: 0x3, #VID
    27: 0x1, #ip type + fragment
    28: 0x1, #ip type
    29: 0x3, #dscp
    30: 0x3, #Egress BD
    31: 0x3, #Egress Vif
    32: 0x1, #(Traffic Class + Drop priority + KeyType)
    33: 0x3, #VlanId + Iptype
    34: 0x1, #Fragment +DSCP
    35: 0x1, #PCP + DEI
    36: 0x3, #UCAST Routed
    37: 0x7, #GTP version
    38: 0x1, #message type
    39: 0xf, #TEID
    40: 0x3,  #isUcastRouted or metadata 
    41: 0x1, #ip fragment
   },
   1:{
    0:  0x0, #start
    1:  0x1, #keyType
    2:  0x1, #aclId
    3:  0x3f,#DMAC
    4:  0x3f,#SMAC
    5:  0xffff, #DIP
    6:  0xffff, #SIP
    7:  0x3, #ETHER TYPE
    8:  0x3, #C-Tag Virtual ID + DEI + PCP
    9:  0x3, #S-Tag Virtual ID + DEI + PCP
    10: 0x3, #L4 destport
    11: 0x3, #L4 srcport
    12: 0x3, #ivif
    13: 0x1, #icmpCode
    14: 0x1, #icmpType
    15: 0x1, #protocol
    16: 0x1, #dscpEcn
    17: 0x3, #bd
    18: 0x1, #ttl
    19: 0x1, #pktLen
    20: 0x1, #tcpflag
    21: 0x3, #vrfId
    22: 0x1, #tagfragment info + pkt info
    23: 0x1, #Router MAC & Traffic Class
    24: 0x1F,#Packet Len & MPLS Label and EXP
    25: 0x1, #source port
    26: 0x3, #VID
    27: 0x1, #ip type + fragment
    28: 0x1, #ip type
    29: 0x3, #dscp
    30: 0x3, #Egress BD
    31: 0x3, #Egress Vif
    32: 0x1, #(Traffic Class + Drop priority + KeyType)
    33: 0x3, #VlanId + Iptype
    34: 0x1, #Fragment +DSCP
    35: 0x1, #PCP + DEI
    36: 0x3, #UCAST Routed
    37: 0x7, #GTP version
    38: 0x1, #message type
    39: 0xf, #TEID
    40: 0x3, #isUcastRouted or metadata 
    41: 0x1  #ip fragment
   },
   2:{
    0:  0x0, #start
    1:  0x1, #keyType
    2:  0x1, #aclId
    3:  0x3f,#DMAC
    4:  0x3f,#SMAC
    5:  0xf, #DIP
    6:  0xf, #SIP
    7:  0x3, #ETHER TYPE
    8:  0x3, #C-Tag Virtual ID + DEI + PCP
    9:  0x3, #S-Tag Virtual ID + DEI + PCP
    10: 0x3, #L4 destport
    11: 0x3, #L4 srcport
    12: 0x3, #ivif
    13: 0x1, #icmpCode
    14: 0x1, #icmpType
    15: 0x1, #protocol
    16: 0x1, #dscpEcn
    17: 0x3, #bd
    18: 0x1, #ttl
    19: 0x1, #pktLen
    20: 0x1, #tcpflag
    21: 0x3, #vrfId
    22: 0x1, #tagfragment info + pkt info
    23: 0x1, #Router MAC & Traffic Class
    24: 0x1F,#Packet Len & MPLS Label and EXP
    25: 0x1, #source port
    26: 0x3, #VID
    27: 0x1, #ip type + fragment
    28: 0x1, #ip type
    29: 0x3, #dscp
    30: 0x3, #Egress BD
    31: 0x3, #Egress Vif
    32: 0x1, #(Traffic Class + Drop priority + KeyType)
    33: 0x3, #VlanId + Iptype
    34: 0x1, #Fragment +DSCP
    35: 0x1, #PCP + DEI
    36: 0x3, #UCAST Routed
    37: 0x7, #GTP version
    38: 0x1, #message type
    39: 0xf, #TEID
    40: 0x3,  #isUcastRouted or metadata 
    41: 0x1, #ip fragment
   }   
}

class xpsIaclObj(Cmd):

    doc_header   = 'Available commands (type help <topic>):'
    undoc_header = 'Utility commands'

    #/*****************************************************************************************************/
    # Validating sequence of IPv4/v6 key-fields
    #/*****************************************************************************************************/

    def validate_keyfields_sequence (self, KeyFieldsList):
        prevkeyfld = 0
        keyValuesinOrder = []
        sortedList = []
        for keyIdx in range(len(KeyFieldsList)):
                if((eval(KeyFieldsList[keyIdx])) < prevkeyfld):
                    print 'Error : Key fields are not in order'
                    print 'Correct order : ' ,
                    for index in range(len(KeyFieldsList)):
                        keyValuesinOrder.append(eval(KeyFieldsList[index]))
                    keyValuesinOrder.sort()
                    for keyIdx in range(len(keyValuesinOrder)):
                         for valIdx in range(len(KeyFieldsList)):
                             if(keyValuesinOrder[keyIdx] == eval(KeyFieldsList[valIdx])):
                                 sortedList.append(KeyFieldsList[valIdx])
                                 break
                    print sortedList
                    return 1
                prevkeyfld = eval(KeyFieldsList[keyIdx])
        return 0

    #/********************************************************************************/
    # command for xpsIaclInit
    #/********************************************************************************/
                
    def do_iacl_init(self, arg):
        '''     
         xpsIaclInit: Enter [ void ]
        '''     
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            print('Input Arguments are : Not required' % ())
            ret = xpsIaclInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIaclInitScope
    #/********************************************************************************/
    def do_iacl_init_scope(self, arg):
        '''
         xpsIaclInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsIaclInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIaclDeInit
    #/********************************************************************************/
    def do_iacl_de_init(self, arg):
        '''
         xpsIaclDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            print('Input Arguments are : Not required' % ())
            ret = xpsIaclDeInit()
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIaclDeInitScope
    #/********************************************************************************/
    def do_iacl_de_init_scope(self, arg):
        '''
         xpsIaclDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsIaclDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIaclAddDevice
    #/********************************************************************************/
    def do_iacl_add_device(self, arg):
        '''
         xpsIaclAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsIaclAddDevice(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIaclRemoveDevice
    #/********************************************************************************/
    def do_iacl_remove_device(self, arg):
        '''
         xpsIaclRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsIaclRemoveDevice(args[0])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIaclCreateTable
    #/********************************************************************************/
    def do_iacl_create_table(self, arg):
        '''
         xpsIaclCreateTable: Enter [ devId,numTables,iaclTblType,keySize,numDb]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numTables,iaclTblType,keySize,numDb] \n <NOTE:- Arguments iaclTblType,keySize and numDb expect (,) seprated vlaue > check iacl_help command for example ')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            tblTypeList = args[2].split(',')
            defKeySize = args[3].split(',')
            numDbList = args[4].split(',')
            keySize = []
            xpIaclTableProfile_t_Ptr = new_xpIaclTableProfile_t()
            xpIaclTableProfile_t_Ptr.numTables = args[1]
            arrIaclTableInfo = new_xpIaclTableInfo_arr(args[1])
            for idx in range(args[1]):
                tableProfile = new_xpIaclTableInfo_t()
                if(int(defKeySize[idx]) <= 64):
                    keySize.append(65)
                elif(int(defKeySize[idx]) > 64 and int(defKeySize[idx]) <= 128):
                    keySize.append(130)
                elif(int(defKeySize[idx]) > 128 and int(defKeySize[idx]) <= 192):
                    keySize.append(195)
                else:
                    keySize.append(390)
                tableProfile.tblType = eval(tblTypeList[idx])
                tableProfile.keySize = keySize[idx]
                tableProfile.numDb = int(numDbList[idx])
                xpIaclTableInfo_arr_setitem(arrIaclTableInfo, idx, tableProfile)
            xpIaclTableProfile_t_Ptr.tableProfile = arrIaclTableInfo
            
            #print('Input Arguments are, devId=%d, numTables=3, tblType=%d, keySize=%d, numDb=%d, tblType=%d, keySize=%d, numDb=%d, tblType=%d, keySize=%d, numDb=%d' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9]))
            ret = xpsIaclCreateTable(args[0],xpIaclTableProfile_t_Ptr)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIaclDeleteTable
    #/********************************************************************************/
    def do_iacl_delete_table(self, arg):
        '''
         xpsIaclDeleteTable: Enter [ devId,iaclType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,iaclType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, iaclType=%d' % (args[0],args[1]))
            ret = xpsIaclDeleteTable(args[0],args[1])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIaclCreateTable
    #/********************************************************************************/
    def do_iacl_create_fixed_table(self, arg):
        '''
         xpsIaclCreateFixedTable: Enter [ devId,numTables,iaclTblType,keySize,numDb,tblMode]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numTables,iaclTblType,keySize,numDb,tblMode] \n <NOTE:- Arguments iaclTblType,keySize and numDb expect (,) seprated vlaue > check iacl_help command for example ')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            tblTypeList = args[2].split(',')
            defKeySize = args[3].split(',')
            numDbList = args[4].split(',')
            keySize = []
            tblMode = int(args[5])
            xpIaclTableProfile_t_Ptr = new_xpIaclTableProfile_t()
            xpIaclTableProfile_t_Ptr.numTables = args[1]
            arrIaclTableInfo = new_xpIaclTableInfo_arr(args[1])
            for idx in range(args[1]):
                tableProfile = new_xpIaclTableInfo_t()
                if(int(defKeySize[idx]) <= 64):
                    keySize.append(65)
                elif(int(defKeySize[idx]) > 64 and int(defKeySize[idx]) <= 128):
                    keySize.append(130)
                elif(int(defKeySize[idx]) > 128 and int(defKeySize[idx]) <= 192):
                    keySize.append(195)
                else:
                    keySize.append(390)
                tableProfile.tblType = eval(tblTypeList[idx])
                tableProfile.keySize = keySize[idx]
                tableProfile.numDb = int(numDbList[idx])
                xpIaclTableInfo_arr_setitem(arrIaclTableInfo, idx, tableProfile)
            xpIaclTableProfile_t_Ptr.tableProfile = arrIaclTableInfo

            #print('Input Arguments are, devId=%d, numTables=3, tblType=%d, keySize=%d, numDb=%d, tblType=%d, keySize=%d, numDb=%d, tblType=%d, keySize=%d, numDb=%d' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9]))
            ret = xpsIaclCreateFixedTable(args[0],xpIaclTableProfile_t_Ptr,tblMode)
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIaclDefinePaclKey
    #/********************************************************************************/
    def do_iacl_define_pacl_key(self, arg):
        '''
         xpsIaclDefinePaclKey: Enter [ devId,keyType,numFlds,isValid,flds]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,keyType,numFlds,isValid,flds ] \n NOTE:-Arguments flds, value and mask expect (,) seprated i/p values. check iacl_help command for example')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[2])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[3])
            xpIaclkeyFieldList_t_Ptr.type = args[1]
            fldList = args[4].strip().split(',')
            if((self.validate_keyfields_sequence(fldList)) != 0):
                delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
                return

            xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
            xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
            for fldIdx,fldEnm in enumerate(fldList):
                   if(xpIaclkeyFieldList_t_Ptr.type == 0):
                       xpIaclkeyField_t_ptr.keyFlds = eval(fldEnm)
                       xpIaclkeyField_t_ptr.fld.v4Fld = eval("XP_IACL_IPV4_START")
                       byteSize = mapxpIaclTypeKeyByteMask[0][eval(fldEnm)]
                   else:
                       xpIaclkeyField_t_ptr.keyFlds = eval(fldEnm)
                       xpIaclkeyField_t_ptr.fld.v6Fld = eval("XP_IACL_IPV6_START")
                       byteSize = mapxpIaclTypeKeyByteMask[1][eval(fldEnm)]
                   byteLen = bin(byteSize).count('1')
                   valPtr = new_uint8Arr_tp(byteLen)
                   maskPtr = new_uint8Arr_tp(byteLen)
                   xpIaclkeyField_t_ptr.value = valPtr 
                   xpIaclkeyField_t_ptr.mask = maskPtr 
                   xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr) 
               
            xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr
            #print('Input Arguments are, devId=%d, keyType=%s isValid=%d, numFlds=%d, fld=%s value=%s mask=%s' % (args[0],args[1],xpIaclkeyFieldList_t_Ptr.isValid,xpIaclkeyFieldList_t_Ptr.numFlds,fldList,valueList,maskList))
            ret = xpsIaclDefinePaclKey(args[0],args[1],xpIaclkeyFieldList_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass
            del valPtr 
            del maskPtr
            del xpIaclkeyField_t_arr
            delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
            delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclDefineBaclKey
    #/********************************************************************************/
    def do_iacl_define_bacl_key(self, arg):
        '''
         xpsIaclDefineBaclKey: Enter [ devId,keyType,numFlds,isValid,flds]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,keyType,numFlds,isValid,flds \n NOTE:-Arguments flds, value and mask expect (,) seprated i/p values. check iacl_help command for example ')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[2])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[3])
            xpIaclkeyFieldList_t_Ptr.type = args[1]
            fldList = args[4].strip().split(',')
            if((self.validate_keyfields_sequence(fldList)) != 0):
                delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
                return

            xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
            xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
            for fldIdx,fldEnm in enumerate(fldList):
                   if(xpIaclkeyFieldList_t_Ptr.type == 0):
                       xpIaclkeyField_t_ptr.keyFlds = eval(fldEnm)
                       xpIaclkeyField_t_ptr.fld.v4Fld = eval("XP_IACL_IPV4_START")
                       byteSize = mapxpIaclTypeKeyByteMask[0][eval(fldEnm)]
                   else:
                       xpIaclkeyField_t_ptr.keyFlds = eval(fldEnm)
                       xpIaclkeyField_t_ptr.fld.v6Fld = eval("XP_IACL_IPV6_START")
                       byteSize = mapxpIaclTypeKeyByteMask[1][eval(fldEnm)]
                   byteLen = bin(byteSize).count('1')
                   valPtr = new_uint8Arr_tp(byteLen)
                   maskPtr = new_uint8Arr_tp(byteLen)
                   xpIaclkeyField_t_ptr.value = valPtr 
                   xpIaclkeyField_t_ptr.mask = maskPtr 
                   xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr) 
               
            xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr
            #print('Input Arguments are, devId=%d, keyType=%s isValid=%d, numFlds=%d, fld=%s value=%s mask=%s' % (args[0],args[1],xpIaclkeyFieldList_t_Ptr.isValid,xpIaclkeyFieldList_t_Ptr.numFlds,fldList,valueList,maskList))
            ret = xpsIaclDefineBaclKey(args[0],args[1],xpIaclkeyFieldList_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass
            del valPtr 
            del maskPtr
            del xpIaclkeyField_t_arr
            delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
            delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclDefineRaclKey
    #/********************************************************************************/
    def do_iacl_define_racl_key(self, arg):
        '''
         xpsIaclDefineRaclKey: Enter [ devId,keyType,numFlds,isValid,fld]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,keyType,numFlds,isValid,flds \n NOTE:-Arguments flds, value and mask expect (,) seprated i/p values. check iacl_help command for example')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[2])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[3])
            xpIaclkeyFieldList_t_Ptr.type = args[1]
            fldList = args[4].strip().split(',') 
            if((self.validate_keyfields_sequence(fldList)) != 0):
                delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
                return

            xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
            xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
            for fldIdx,fldEnm in enumerate(fldList):
                   if(xpIaclkeyFieldList_t_Ptr.type == 0):
                       xpIaclkeyField_t_ptr.keyFlds = eval(fldEnm)
                       xpIaclkeyField_t_ptr.fld.v4Fld = eval("XP_IACL_IPV4_START")
                       byteSize = mapxpIaclTypeKeyByteMask[0][eval(fldEnm)]
                   else:
                       xpIaclkeyField_t_ptr.keyFlds = eval(fldEnm)
                       xpIaclkeyField_t_ptr.fld.v6Fld = eval("XP_IACL_IPV6_START")
                       byteSize = mapxpIaclTypeKeyByteMask[1][eval(fldEnm)]
                   byteLen = bin(byteSize).count('1')
                   valPtr = new_uint8Arr_tp(byteLen)
                   maskPtr = new_uint8Arr_tp(byteLen)
                   xpIaclkeyField_t_ptr.value = valPtr 
                   xpIaclkeyField_t_ptr.mask = maskPtr 
                   xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr)
               
            xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr
            #print('Input Arguments are, devId=%d, keyType=%s isValid=%d, numFlds=%d, fld=%s value=%s mask=%s' % (args[0],args[1],xpIaclkeyFieldList_t_Ptr.isValid,xpIaclkeyFieldList_t_Ptr.numFlds,fldList,valueList,maskList))
            ret = xpsIaclDefineRaclKey(args[0],args[1],xpIaclkeyFieldList_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass
            del valPtr 
            del maskPtr
            del xpIaclkeyField_t_arr
            delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
            delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclWritePaclKey
    #/********************************************************************************/
    def do_iacl_write_pacl_key(self, arg):
        '''
         xpsIaclWritePaclKey: Enter [ devId,camIndex,numFlds,isValid,type, flds,value,mask flds,value,mask .. upto the num of key wants to be program.]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,numFlds,isValid,type, flds,value,mask flds,value,mask ...append flds,value,mask for num of key wants to be program] \n \
            e.g.:- \n \
            iacl_write_pacl_key 0 0 6 1 XP_IACL_V4_TYPE XP_IACL_KEY_TYPE_V4,0,0 XP_IACL_ID,1,0 XP_IACL_CTAG_VID_DEI_PCP,21763,0 XP_IACL_MAC_DA,0:0:0:0:0:0,0 XP_IACL_MAC_SA,0:0:0:0:0:0,0')
        else:
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[2])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[3])
            xpIaclkeyFieldList_t_Ptr.type = eval(args[4])
            fldList = []
            valueList = []
            maskList = []
            fldvalMask = ''
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 3:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
            if len(fldList) == int(args[2]) and len(valueList) == int(args[2]) and len(maskList) == int(args[2]):
                if((self.validate_keyfields_sequence(fldList)) != 0):
                    delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
                    return
                xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                       if(eval(args[4]) == 0):
                           xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                           xpIaclkeyField_t_ptr.fld.v4Fld = eval("XP_IACL_IPV4_START")
                           byteSize = mapxpIaclTypeKeyByteMask[0][eval(defFld)]
                       else:
                           xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                           xpIaclkeyField_t_ptr.fld.v6Fld = eval("XP_IACL_IPV6_START")
                           byteSize = mapxpIaclTypeKeyByteMask[1][eval(defFld)]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       if(not defFld in fldList):
                           for valIdx in range(byteLen):
                               uint8Arr_tp_setitem(valPtr, valIdx, 0) 
                               uint8Arr_tp_setitem(maskPtr, valIdx,0xff) 
                       else:
                           hexFlag = 0
                           fldMask = []
                           fldVal = []
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split('.')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['255'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split('.')
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split(':')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['ff'] * len(fldVal)
                               else:
                                   if(maskList[fldIdx] == '255'):
                                       fldMask = ['ff'] * len(fldVal)
                                   else:
                                       fldMask = maskList[fldIdx].strip().split(':')
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               if maskList[fldIdx] == '0':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('00')
                               elif maskList[fldIdx] == '1':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('ff')
                               else:
                                   tmp = int(maskList[fldIdx])
                                   for valIdx in range(byteLen):
                                       tmp1 = tmp%256
                                       fldMask.append(hex(tmp1).replace('0x',''))
                                       tmp = tmp/256
                                   fldMask = fldMask[::-1]
                               hexFlag = 1
                           for valIdx in range(byteLen):
                               if(hexFlag):
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16)) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1],16))
                               else:
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1])) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1]))
                           hexFlag = 0
                       xpIaclkeyField_t_ptr.value = valPtr 
                       xpIaclkeyField_t_ptr.mask = maskPtr 
                       xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr) 
                xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr
                   
                #print('Input Arguments are, devId=%d, camIndex=%d isValid=%d, numFlds=%d, type=%d fld=%s value=%s mask=%s' % (args[0],args[1],xpIaclkeyFieldList_t_Ptr.isValid,xpIaclkeyFieldList_t_Ptr.numFlds,xpIaclkeyFieldList_t_Ptr.type,fldList,valueList,maskList))
                ret = xpsIaclWritePaclKey(args[0],args[1],xpIaclkeyFieldList_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
                delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args " 
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsIaclWriteBaclKey
    #/********************************************************************************/
    def do_iacl_write_bacl_key(self, arg):
        '''
         xpsIaclWriteBaclKey: Enter [ devId,camIndex,numFlds,isValid,type, flds,value,mask flds,value,mask .. upto ithe num of key wants to be program.]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,numFlds,isValid,type, flds,value,mask flds,value,mask ...append flds,value,mask for num of key wants to be program] \n \
            e.g.:-\n \
            iacl_write_bacl_key 0 0 6 1 XP_IACL_V4_TYPE XP_IACL_KEY_TYPE_V4,0,0 XP_IACL_ID,1,0 XP_IACL_CTAG_VID_DEI_PCP,21763,0 XP_IACL_MAC_DA,0:0:0:0:0:0,0 XP_IACL_MAC_SA,0:0:0:0:0:0,0')
        else:
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[2])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[3])
            xpIaclkeyFieldList_t_Ptr.type = eval(args[4])
            fldList = []
            valueList = []
            maskList = []
            fldvalMask = ''
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 3:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
            if len(fldList) == int(args[2]) and len(valueList) == int(args[2]) and len(maskList) == int(args[2]):
                if((self.validate_keyfields_sequence(fldList)) != 0):
                    delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
                    return
                xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                       if(eval(args[4]) == 0):
                           xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                           xpIaclkeyField_t_ptr.fld.v4Fld = eval("XP_IACL_IPV4_START")
                           byteSize = mapxpIaclTypeKeyByteMask[0][eval(defFld)]
                       else:
                           xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                           xpIaclkeyField_t_ptr.fld.v6Fld = eval("XP_IACL_IPV6_START")
                           byteSize = mapxpIaclTypeKeyByteMask[1][eval(defFld)]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       if(not defFld in fldList):
                           for valIdx in range(byteLen):
                               uint8Arr_tp_setitem(valPtr, valIdx, 0) 
                               uint8Arr_tp_setitem(maskPtr, valIdx,0xff) 
                       else:
                           hexFlag = 0
                           fldMask = []
                           fldVal = []
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split('.')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['255'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split('.')
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split(':')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['ff'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split(':')
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               if maskList[fldIdx] == '0':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('00')
                               elif maskList[fldIdx] == '1':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('ff')
                               else:
                                   tmp = int(maskList[fldIdx])
                                   for valIdx in range(byteLen):
                                       tmp1 = tmp%256
                                       fldMask.append(hex(tmp1).replace('0x',''))
                                       tmp = tmp/256
                                   fldMask = fldMask[::-1]
                               hexFlag = 1
                           for valIdx in range(len(fldVal)):
                               if(hexFlag):
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16)) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1],16))
                               else:
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1])) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1]))
                           hexFlag = 0
                       xpIaclkeyField_t_ptr.value = valPtr 
                       xpIaclkeyField_t_ptr.mask = maskPtr 
                       xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr) 
                xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr
                   
                #print('Input Arguments are, devId=%d, camIndex=%d isValid=%d, numFlds=%d, type=%d fld=%s value=%s mask=%s' % (args[0],args[1],xpIaclkeyFieldList_t_Ptr.isValid,xpIaclkeyFieldList_t_Ptr.numFlds,xpIaclkeyFieldList_t_Ptr.type,fldList,valueList,maskList))
                ret = xpsIaclWriteBaclKey(args[0],args[1],xpIaclkeyFieldList_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
                delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args " 
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsIaclWriteRaclKey
    #/********************************************************************************/
    def do_iacl_write_racl_key(self, arg):
        '''
         xpsIaclWriteRaclKey: Enter [ devId,camIndex,numFlds,isValid,type, flds,value,mask flds,value,mask .. upto the num of key wants to be program.]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,numFlds,isValid,type, flds,value,mask flds,value,mask ...append flds,value,mask for num of key wants to be program] \n \
            e.g.:- \n \
            iacl_write_racl_key 0 0 6 1 XP_IACL_V4_TYPE XP_IACL_KEY_TYPE_V4,0,0 XP_IACL_ID,1,0 XP_IACL_CTAG_VID_DEI_PCP,21763,0 XP_IACL_MAC_DA,0:0:0:0:0:0,0 XP_IACL_MAC_SA,0:0:0:0:0:0,0')
        else:
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[2])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[3])
            xpIaclkeyFieldList_t_Ptr.type = eval(args[4])
            fldList = []
            valueList = []
            maskList = []
            fldvalMask = ''
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 3:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
            if len(fldList) == int(args[2]) and len(valueList) == int(args[2]) and len(maskList) == int(args[2]):
                if((self.validate_keyfields_sequence(fldList)) != 0):
                    delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
                    return
                xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                       if(eval(args[4]) == 0):
                           xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                           xpIaclkeyField_t_ptr.fld.v4Fld = eval("XP_IACL_IPV4_START")
                           byteSize = mapxpIaclTypeKeyByteMask[0][eval(defFld)]
                       else:
                           xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                           xpIaclkeyField_t_ptr.fld.v6Fld = eval("XP_IACL_IPV6_START")
                           byteSize = mapxpIaclTypeKeyByteMask[1][eval(defFld)]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       if(not defFld in fldList):
                           for valIdx in range(byteLen):
                               uint8Arr_tp_setitem(valPtr, valIdx, 0) 
                               uint8Arr_tp_setitem(maskPtr, valIdx,0xff) 
                       else:
                           hexFlag = 0
                           fldMask = []
                           fldVal = []
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split('.')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['255'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split('.')
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split(':')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['ff'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split(':')
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               if maskList[fldIdx] == '0':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('00')
                               elif maskList[fldIdx] == '1':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('ff')
                               else:
                                   tmp = int(maskList[fldIdx])
                                   for valIdx in range(byteLen):
                                       tmp1 = tmp%256
                                       fldMask.append(hex(tmp1).replace('0x',''))
                                       tmp = tmp/256
                                   fldMask = fldMask[::-1]
                               hexFlag = 1
                           for valIdx in range(len(fldVal)):
                               if(hexFlag):
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16)) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1],16))
                               else:
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1])) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1]))
                           hexFlag = 0
                       xpIaclkeyField_t_ptr.value = valPtr 
                       xpIaclkeyField_t_ptr.mask = maskPtr 
                       xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr) 
                xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr
                   
                #print('Input Arguments are, devId=%d, camIndex=%d isValid=%d, numFlds=%d, type=%d fld=%s value=%s mask=%s' % (args[0],args[1],xpIaclkeyFieldList_t_Ptr.isValid,xpIaclkeyFieldList_t_Ptr.numFlds,xpIaclkeyFieldList_t_Ptr.type,fldList,valueList,maskList))
                ret = xpsIaclWriteRaclKey(args[0],args[1],xpIaclkeyFieldList_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
                delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args " 
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsIaclWritePaclData
    #/********************************************************************************/
    def do_iacl_write_pacl_data(self, arg):
        '''
         xpsIaclWritePaclData: Enter [ devId,camIndex,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 23
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmdOrEcmpSize,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsIaclData_t_Ptr = new_xpsIaclData_tp()
            xpsIaclData_t_Ptr.iAclDataType.aclData.isTerminal = int(args[2])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[3])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[4])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enDpRemark = int(args[5])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enPolicer = int(args[6])
            xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[7])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[8])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enTcRemark = int(args[9])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[10])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[11])
            xpsIaclData_t_Ptr.iAclDataType.aclData.tc = int(args[13])
            xpsIaclData_t_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[14])
            xpsIaclData_t_Ptr.iAclDataType.aclData.encapType = eval(args[15])
            xpsIaclData_t_Ptr.iAclDataType.aclData.policerId = int(args[17])
            if(xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif == 1):
                xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.eVifId = int(args[16])
                xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.ecmpSize = int(args[12])
            else:
                xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command = int(args[12])
                xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode = int(args[18])
            xpsIaclData_t_Ptr.iAclDataType.aclData.policerId = int(args[17])
            xpsIaclData_t_Ptr.iAclDataType.aclData.dp = int(args[19])
            if (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw == 1):
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[20])
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[21])
            else:
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[22])
            ret = xpsIaclWritePaclData(args[0],args[1],xpsIaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsIaclData_tp(xpsIaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclWriteBaclData
    #/********************************************************************************/
    def do_iacl_write_bacl_data(self, arg):
        '''
         xpsIaclWriteBaclData: Enter [ devId,camIndex,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 23
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [  devId,camIndex,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsIaclData_t_Ptr = new_xpsIaclData_tp()
            xpsIaclData_t_Ptr.iAclDataType.aclData.isTerminal = int(args[2])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[3])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[4])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enDpRemark = int(args[5])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enPolicer = int(args[6])
            xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[7])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[8])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enTcRemark = int(args[9])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[10])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[11])
            xpsIaclData_t_Ptr.iAclDataType.aclData.tc = int(args[13])
            xpsIaclData_t_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[14])
            xpsIaclData_t_Ptr.iAclDataType.aclData.encapType = int(args[15])
            xpsIaclData_t_Ptr.iAclDataType.aclData.policerId = int(args[17])

            if(xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif == 1):
                xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.eVifId = int(args[16])
                xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.ecmpSize = int(args[12])
            else:
                xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command = int(args[12])
                xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode = int(args[18])

            xpsIaclData_t_Ptr.iAclDataType.aclData.dp = int(args[19])
            if (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw == 1):
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[20])
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[21])
            else:
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[22])
            ret = xpsIaclWriteBaclData(args[0],args[1],xpsIaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsIaclData_tp(xpsIaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclWriteRaclData
    #/********************************************************************************/
    def do_iacl_write_racl_data(self, arg):
        '''
         xpsIaclWriteRaclData: Enter [ devId,camIndex,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 23
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpsIaclData_t_Ptr = new_xpsIaclData_tp()
            xpsIaclData_t_Ptr.iAclDataType.aclData.isTerminal = int(args[2])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[3])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[4])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enDpRemark = int(args[5])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enPolicer = int(args[6])
            xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[7])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[8])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enTcRemark = int(args[9])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[10])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[11])
            xpsIaclData_t_Ptr.iAclDataType.aclData.tc = int(args[13])
            xpsIaclData_t_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[14])
            xpsIaclData_t_Ptr.iAclDataType.aclData.encapType = int(args[15])
            xpsIaclData_t_Ptr.iAclDataType.aclData.policerId = int(args[17])

            if(xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif == 1):
                xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.eVifId = int(args[16])
                xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.ecmpSize = int(args[12])
            else:
                xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command = int(args[12])
                xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode = int(args[18])

            xpsIaclData_t_Ptr.iAclDataType.aclData.policerId = int(args[17])
            xpsIaclData_t_Ptr.iAclDataType.aclData.dp = int(args[19])
            if (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw == 1):
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[20])
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[21])
            else:
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[22])
            ret = xpsIaclWriteRaclData(args[0],args[1],xpsIaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsIaclData_tp(xpsIaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclWritePaclEntry
    #/********************************************************************************/
    def do_iacl_write_pacl_entry(self, arg):
        '''
         xpsIaclWritePaclEntry: Enter [ devId,camIndex,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan flds,value,mask flds,value,mask ...upto number of keys wants to be programe ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 27
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan flds,value,mask flds,value,mask ...upto number of keys wants to be programe]\
            e.g.:- \n \
            iacl_write_pacl_entry 0 0 6 1 XP_IACL_V4_TYPE 0 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 19 0 0 0 0 852 XP_IACL_KEY_TYPE_V4,0,0 XP_IACL_ID,1,0 XP_IACL_CTAG_VID_DEI_PCP,21763,0 XP_IACL_MAC_DA,0:0:0:0:0:0,0 XP_IACL_MAC_SA,0:0:0:0:0:0,0')
        else:
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[2])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[3])
            xpIaclkeyFieldList_t_Ptr.type = eval(args[4])
            fldList = []
            valueList = []
            maskList = []
            fldvalMask = ''
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 3:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
            if len(fldList) == int(args[2]) and len(valueList) == int(args[2]) and len(maskList) == int(args[2]):
                xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                       if(eval(args[4]) == 0):
                           xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                           xpIaclkeyField_t_ptr.fld.v4Fld = eval("XP_IACL_IPV4_START")
                           byteSize = mapxpIaclTypeKeyByteMask[0][eval(defFld)]
                       else:
                           xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                           xpIaclkeyField_t_ptr.fld.v6Fld = eval("XP_IACL_IPV6_START")
                           byteSize = mapxpIaclTypeKeyByteMask[1][eval(defFld)]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       if(not defFld in fldList):
                           for valIdx in range(byteLen):
                               uint8Arr_tp_setitem(valPtr, valIdx, 0) 
                               uint8Arr_tp_setitem(maskPtr, valIdx,0xff) 
                       else:
                           hexFlag = 0
                           fldMask = []
                           fldVal = []
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split('.')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['255'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split('.')
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split(':')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['ff'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split(':')
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               if maskList[fldIdx] == '0':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('00')
                               elif maskList[fldIdx] == '1':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('ff')
                               else:
                                   tmp = int(maskList[fldIdx])
                                   for valIdx in range(byteLen):
                                       tmp1 = tmp%256
                                       fldMask.append(hex(tmp1).replace('0x',''))
                                       tmp = tmp/256
                                   fldMask = fldMask[::-1]
                               hexFlag = 1
                           for valIdx in range(len(fldVal)):
                               if(hexFlag):
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16)) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1],16))
                               else:
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1])) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1]))
                           hexFlag = 0
                       xpIaclkeyField_t_ptr.value = valPtr 
                       xpIaclkeyField_t_ptr.mask = maskPtr 
                       xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr) 
                xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr
                xpsIaclData_t_Ptr = new_xpsIaclData_tp()
                xpsIaclData_t_Ptr.iAclDataType.aclData.isTerminal = int(args[5])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[6])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[7])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enDpRemark = int(args[8])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enPolicer = int(args[9])
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[10])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[11])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enTcRemark = int(args[12])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[13])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[14])
                xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmd = int(args[15])
                xpsIaclData_t_Ptr.iAclDataType.aclData.tc = int(args[16])
                xpsIaclData_t_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[17])
                xpsIaclData_t_Ptr.iAclDataType.aclData.encapType = int(args[18])
                xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifId = int(args[19])
                xpsIaclData_t_Ptr.iAclDataType.aclData.policerId = int(args[20])
                xpsIaclData_t_Ptr.iAclDataType.aclData.rsnCode = int(args[21])
                xpsIaclData_t_Ptr.iAclDataType.aclData.dp = int(args[22])
                if (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw == 1):
                    xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[23])
                    xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[24])
                else:
                    xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[25])
                ret = xpsIaclWritePaclEntry(args[0],args[1],xpIaclkeyFieldList_t_Ptr,xpsIaclData_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpsIaclData_tp(xpsIaclData_t_Ptr)
                delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
                delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args " 
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsIaclWriteBaclEntry
    #/********************************************************************************/
    def do_iacl_write_bacl_entry(self, arg):
        '''
         xpsIaclWriteBaclEntry: Enter [ devId,camIndex,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan flds,value,mask flds,value,mask ...upto number of keys wants to be programe ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 27
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan flds,value,mask flds,value,mask ...append flds,value,mask for number of keys wants to be programe ]\n\
            e.g.:- \n \
            iacl_write_bacl_entry 0 0 6 1 XP_IACL_V4_TYPE 0 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 19 0 0 0 852 XP_IACL_KEY_TYPE_V4,0,0 XP_IACL_ID,1,0 XP_IACL_CTAG_VID_DEI_PCP,21763,0 XP_IACL_MAC_DA,0:0:0:0:0:0,0 XP_IACL_MAC_SA,0:0:0:0:0:0,0')
        else:
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[2])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[3])
            xpIaclkeyFieldList_t_Ptr.type = eval(args[4])
            fldList = []
            valueList = []
            maskList = []
            fldvalMask = ''
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 3:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
            if len(fldList) == int(args[2]) and len(valueList) == int(args[2]) and len(maskList) == int(args[2]):
                xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                       if(eval(args[4]) == 0):
                           xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                           xpIaclkeyField_t_ptr.fld.v4Fld = eval("XP_IACL_IPV4_START")
                           byteSize = mapxpIaclTypeKeyByteMask[0][eval(defFld)]
                       else:
                           xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                           xpIaclkeyField_t_ptr.fld.v6Fld = eval("XP_IACL_IPV6_START")
                           byteSize = mapxpIaclTypeKeyByteMask[1][eval(defFld)]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       if(not defFld in fldList):
                           for valIdx in range(byteLen):
                               uint8Arr_tp_setitem(valPtr, valIdx, 0) 
                               uint8Arr_tp_setitem(maskPtr, valIdx,0xff) 
                       else:
                           hexFlag = 0
                           fldMask = []
                           fldVal = []
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split('.')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['255'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split('.')
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split(':')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['ff'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split(':')
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               if maskList[fldIdx] == '0':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('00')
                               elif maskList[fldIdx] == '1':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('ff')
                               else:
                                   tmp = int(maskList[fldIdx])
                                   for valIdx in range(byteLen):
                                       tmp1 = tmp%256
                                       fldMask.append(hex(tmp1).replace('0x',''))
                                       tmp = tmp/256
                                   fldMask = fldMask[::-1]
                               hexFlag = 1
                           for valIdx in range(len(fldVal)):
                               if(hexFlag):
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16)) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1],16))
                               else:
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1])) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1]))
                           hexFlag = 0
                       xpIaclkeyField_t_ptr.value = valPtr 
                       xpIaclkeyField_t_ptr.mask = maskPtr 
                       xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr) 
                xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr
                xpsIaclData_t_Ptr = new_xpsIaclData_tp()
                xpsIaclData_t_Ptr.iAclDataType.aclData.isTerminal = int(args[5])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[6])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[7])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enDpRemark = int(args[8])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enPolicer = int(args[9])
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[10])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[11])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enTcRemark = int(args[12])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[13])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[14])
                xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmd = int(args[15])
                xpsIaclData_t_Ptr.iAclDataType.aclData.tc = int(args[16])
                xpsIaclData_t_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[17])
                xpsIaclData_t_Ptr.iAclDataType.aclData.encapType = int(args[18])
                xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifId = int(args[19])
                xpsIaclData_t_Ptr.iAclDataType.aclData.policerId = int(args[20])
                xpsIaclData_t_Ptr.iAclDataType.aclData.rsnCode = int(args[21])
                xpsIaclData_t_Ptr.iAclDataType.aclData.dp = int(args[22])
                if (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw == 1):
                    xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[23])
                    xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[24])
                else:
                    xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[25])
                ret = xpsIaclWriteBaclEntry(args[0],args[1],xpIaclkeyFieldList_t_Ptr,xpsIaclData_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpsIaclData_tp(xpsIaclData_t_Ptr)
                delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
                delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args " 
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsIaclWriteRaclEntry
    #/********************************************************************************/
    def do_iacl_write_racl_entry(self, arg):
        '''
         xpsIaclWriteRaclEntry: Enter [ devId,camIndex,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan flds,value,mask flds,value,mask ...upto number of keys wants to be programe ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 27
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan flds,value,mask flds,value,mask ...append flds,value,mask for number of keys wants to be programe ]\n\
            e.g.:-\n \
            iacl_write_racl_entry 0 0 6 1 XP_IACL_V4_TYPE 0 1 0 1 0 0 0 0 0 0 0 0 0 0 0 0 19 0 0 0 0 852 XP_IACL_KEY_TYPE_V4,0,0 XP_IACL_ID,1,0 XP_IACL_CTAG_VID_DEI_PCP,21763,0 XP_IACL_MAC_DA,0:0:0:0:0:0,0 XP_IACL_MAC_SA,0:0:0:0:0:0,0')
        else:
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[2])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[3])
            xpIaclkeyFieldList_t_Ptr.type = eval(args[4])
            fldList = []
            valueList = []
            maskList = []
            fldvalMask = ''
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 3:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
            if len(fldList) == int(args[2]) and len(valueList) == int(args[2]) and len(maskList) == int(args[2]):
                xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                       if(eval(args[4]) == 0):
                           xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                           xpIaclkeyField_t_ptr.fld.v4Fld = eval("XP_IACL_IPV4_START")
                           byteSize = mapxpIaclTypeKeyByteMask[0][eval(defFld)]
                       else:
                           xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                           xpIaclkeyField_t_ptr.fld.v6Fld = eval("XP_IACL_IPV6_START")
                           byteSize = mapxpIaclTypeKeyByteMask[1][eval(defFld)]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       if(not defFld in fldList):
                           for valIdx in range(byteLen):
                               uint8Arr_tp_setitem(valPtr, valIdx, 0) 
                               uint8Arr_tp_setitem(maskPtr, valIdx,0xff) 
                       else:
                           hexFlag = 0
                           fldMask = []
                           fldVal = []
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split('.')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['255'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split('.')
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split(':')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['ff'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split(':')
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               if maskList[fldIdx] == '0':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('00')
                               elif maskList[fldIdx] == '1':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('ff')
                               else:
                                   tmp = int(maskList[fldIdx])
                                   for valIdx in range(byteLen):
                                       tmp1 = tmp%256
                                       fldMask.append(hex(tmp1).replace('0x',''))
                                       tmp = tmp/256
                                   fldMask = fldMask[::-1]
                               hexFlag = 1
                           for valIdx in range(len(fldVal)):
                               if(hexFlag):
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16)) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1],16))
                               else:
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1])) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1]))
                           hexFlag = 0
                       xpIaclkeyField_t_ptr.value = valPtr 
                       xpIaclkeyField_t_ptr.mask = maskPtr 
                       xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr) 
                xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr

                xpsIaclData_t_Ptr = new_xpsIaclData_tp()
                xpsIaclData_t_Ptr.iAclDataType.aclData.isTerminal = int(args[5])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[6])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[7])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enDpRemark = int(args[8])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enPolicer = int(args[9])
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[10])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[11])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enTcRemark = int(args[12])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[13])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[14])
                xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmd = int(args[15])
                xpsIaclData_t_Ptr.iAclDataType.aclData.tc = int(args[16])
                xpsIaclData_t_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[17])
                xpsIaclData_t_Ptr.iAclDataType.aclData.encapType = int(args[18])
                xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifId = int(args[19])
                xpsIaclData_t_Ptr.iAclDataType.aclData.policerId = int(args[20])
                xpsIaclData_t_Ptr.iAclDataType.aclData.rsnCode = int(args[21])
                xpsIaclData_t_Ptr.iAclDataType.aclData.dp = int(args[22])
                if (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw == 1):
                    xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[23])
                    xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[24])
                else:
                    xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[25])
                ret = xpsIaclWriteRaclEntry(args[0],args[1],xpIaclkeyFieldList_t_Ptr,xpsIaclData_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpsIaclData_tp(xpsIaclData_t_Ptr)
                delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
                delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args " 
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsIaclUpdatePaclKeyField
    #/********************************************************************************/
    def do_iacl_update_pacl_key_field(self, arg):
        '''
         xpsIaclUpdatePaclKeyField: Enter [ devId,camIndex,keyType,keyField,fieldVal,fieldMask ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,keyType,keyField,fieldVal,fieldMask ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            keyFld = eval(args[3])
            if(eval(args[2]) == 0):
                byteSize = mapxpIaclTypeKeyByteMask[0][eval(args[3])]
            else:
                byteSize = mapxpIaclTypeKeyByteMask[1][eval(args[3])]
            byteLen = bin(byteSize).count('1')
            fieldVal_Ptr_3 = new_uint8Arr_tp(byteLen)
            fieldMask_Ptr_4 = new_uint8Arr_tp(byteLen)

            fldVal = []
            fldMask = []
            hexFlg = 0
            if('.' in args[4]):
                fldVal = args[4].strip().split('.')
                if(len(args[5]) == 1):
                    if(args[5] == '0'):
                        fldMask = ['00'] * len(fldVal)
                    else:
                        fldMask = ['255'] * len(fldVal)
                else:
                    fldMask = args[5].strip().split('.')
            elif(':' in args[4]):
                hexFlg = 1
                fldVal = args[4].strip().split(':')
                if(len(args[5]) == 1):
                    if(args[5] == '0'):
                        fldMask = ['00'] * len(fldVal)
                    else:
                        fldMask = ['ff'] * len(fldVal)
                else:
                    fldMask = args[5].strip().split(':')
            else:
                tmpValue = format(int(args[4]), '0' + str(byteLen*2) + 'X')
                fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                if args[5] == '0':
                    del fldMask[:]
                    for valIdx in range(byteLen):
                        fldMask.append('00')
                elif args[5] == '1':
                    del fldMask[:]
                    for valIdx in range(byteLen):
                        fldMask.append('ff')
                else:
                    tmp = int(args[5])
                    for valIdx in range(byteLen):
                        tmp1 = tmp%256
                        fldMask.append(hex(tmp1).replace('0x',''))
                        tmp = tmp/256
                    fldMask = fldMask[::-1]
                hexFlg = 1
            for valIdx in range(byteLen):
                if(hexFlg): 
                    uint8Arr_tp_setitem(fieldVal_Ptr_3, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16))
                    uint8Arr_tp_setitem(fieldMask_Ptr_4, valIdx, int(fldMask[len(fldVal) - valIdx - 1],16))
                else:
                    uint8Arr_tp_setitem(fieldVal_Ptr_3, valIdx, int(fldVal[len(fldVal) - valIdx - 1]))
                    uint8Arr_tp_setitem(fieldMask_Ptr_4, valIdx, int(fldMask[len(fldVal) - valIdx - 1]))
            hexFlg = 0
            #print('Input Arguments are, devId=%d, camIndex=%d, keyField=%d' % (args[0],args[1],eval(args[2])))
            ret = xpsIaclUpdatePaclKeyField(args[0],args[1],keyFld,fieldVal_Ptr_3,fieldMask_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_uint8_tp(fieldMask_Ptr_4)
            delete_uint8_tp(fieldVal_Ptr_3)

    #/********************************************************************************/
    # command for xpsIaclUpdateBaclKeyField
    #/********************************************************************************/
    def do_iacl_update_bacl_key_field(self, arg):
        '''
         xpsIaclUpdateBaclKeyField: Enter [ devId,camIndex,keyType,keyField,fieldVal,fieldMask ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,keyType,keyField,fieldVal,fieldMask ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            keyFld = eval(args[3])
            if(eval(args[2]) == 0):
                byteSize = mapxpIaclTypeKeyByteMask[0][eval(args[3])]
            else:
                byteSize = mapxpIaclTypeKeyByteMask[1][eval(args[3])]
            byteLen =  bin(byteSize).count('1')
            fieldVal_Ptr_3 = new_uint8Arr_tp(byteLen)
            fieldMask_Ptr_4 = new_uint8Arr_tp(byteLen)
            fldVal = []
            fldMask = []
            hexFlg = 0
            if('.' in args[4]):
                fldVal = args[4].strip().split('.')
                if(len(args[5]) == 1):
                    if(args[5] == '0'):
                        fldMask = ['00'] * len(fldVal)
                    else:
                        fldMask = ['255'] * len(fldVal)
                else:
                    fldMask = args[5].strip().split('.')
            elif(':' in args[4]):
                hexFlg = 1
                fldVal = args[4].strip().split(':')
                if(len(args[5]) == 1):
                    if(args[5] == '0'):
                        fldMask = ['00'] * len(fldVal)
                    else:
                        fldMask = ['ff'] * len(fldVal)
                else:
                    fldMask = args[5].strip().split(':')
            else:
                tmpValue = format(int(args[4]), '0' + str(byteLen*2) + 'X')
                fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                if args[5] == '0':
                    del fldMask[:]
                    for valIdx in range(byteLen):
                        fldMask.append('00')
                elif args[5] == '1':
                    del fldMask[:]
                    for valIdx in range(byteLen):
                        fldMask.append('ff')
                else:
                    tmp = int(args[5])
                    for valIdx in range(byteLen):
                        tmp1 = tmp%256
                        fldMask.append(hex(tmp1).replace('0x',''))
                        tmp = tmp/256
                    fldMask = fldMask[::-1]
                hexFlg = 1
            for valIdx in range(byteLen):
                if(hexFlg): 
                    uint8Arr_tp_setitem(fieldVal_Ptr_3, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16))
                    uint8Arr_tp_setitem(fieldMask_Ptr_4, valIdx, int(fldMask[len(fldVal) - valIdx - 1],16))
                else:
                    uint8Arr_tp_setitem(fieldVal_Ptr_3, valIdx, int(fldVal[len(fldVal) - valIdx - 1]))
                    uint8Arr_tp_setitem(fieldMask_Ptr_4, valIdx, int(fldMask[len(fldVal) - valIdx - 1]))
            hexFlg = 0
             
            #print('Input Arguments are, devId=%d, camIndex=%d, keyField=%d' % (args[0],args[1],eval(args[2])))
            ret = xpsIaclUpdateBaclKeyField(args[0],args[1],keyFld,fieldVal_Ptr_3,fieldMask_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_uint8_tp(fieldMask_Ptr_4)
            delete_uint8_tp(fieldVal_Ptr_3)

    #/********************************************************************************/
    # command for xpsIaclUpdateRaclKeyField
    #/********************************************************************************/
    def do_iacl_update_racl_key_field(self, arg):
        '''
         xpsIaclUpdateRaclKeyField: Enter [ devId,camIndex,keyType,keyField,fieldVal,fieldMask ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,keyType,keyField,fieldVal,fieldMask ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            keyFld = eval(args[3])
            if(eval(args[2]) == 0):
                byteSize = mapxpIaclTypeKeyByteMask[0][eval(args[3])]
            else:
                byteSize = mapxpIaclTypeKeyByteMask[1][eval(args[3])]
            byteLen = bin(byteSize).count('1')
            fieldVal_Ptr_3 = new_uint8Arr_tp(byteLen)
            fieldMask_Ptr_4 = new_uint8Arr_tp(byteLen)
            fldVal = []
            fldMask = []
            hexFlg = 0
            if('.' in args[4]):
                fldVal = args[4].strip().split('.')
                if(len(args[5]) == 1):
                    if(args[5] == '0'):
                        fldMask = ['00'] * len(fldVal)
                    else:
                        fldMask = ['255'] * len(fldVal)
                else:
                    fldMask = args[5].strip().split('.')
            elif(':' in args[4]):
                hexFlg = 1
                fldVal = args[4].strip().split(':')
                if(len(args[5]) == 1):
                    if(args[5] == '0'):
                        fldMask = ['00'] * len(fldVal)
                    else:
                        fldMask = ['ff'] * len(fldVal)
                else:
                    fldMask = args[5].strip().split(':')
            else:
                tmpValue = format(int(args[4]), '0' + str(byteLen*2) + 'X')
                fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                if args[5] == '0':
                    del fldMask[:]
                    for valIdx in range(byteLen):
                        fldMask.append('00')
                elif args[5] == '1':
                    del fldMask[:]
                    for valIdx in range(byteLen):
                        fldMask.append('ff')
                else:
                    tmp = int(args[5])
                    for valIdx in range(byteLen):
                        tmp1 = tmp%256
                        fldMask.append(hex(tmp1).replace('0x',''))
                        tmp = tmp/256
                    fldMask = fldMask[::-1]
                hexFlg = 1
            for valIdx in range(byteLen):
                if(hexFlg): 
                    uint8Arr_tp_setitem(fieldVal_Ptr_3, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16))
                    uint8Arr_tp_setitem(fieldMask_Ptr_4, valIdx, int(fldMask[len(fldVal) - valIdx - 1],16))
                else:
                    uint8Arr_tp_setitem(fieldVal_Ptr_3, valIdx, int(fldVal[len(fldVal) - valIdx - 1]))
                    uint8Arr_tp_setitem(fieldMask_Ptr_4, valIdx, int(fldMask[len(fldVal) - valIdx - 1]))
            hexFlg = 0
             
            #print('Input Arguments are, devId=%d, camIndex=%d, keyField=%d' % (args[0],args[1],eval(args[2])))
            ret = xpsIaclUpdateRaclKeyField(args[0],args[1],keyFld,fieldVal_Ptr_3,fieldMask_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_uint8_tp(fieldMask_Ptr_4)
            delete_uint8_tp(fieldVal_Ptr_3)

    #/********************************************************************************/
    # command for xpsIaclPaclMallocKeyFieldList
    #/********************************************************************************/
    def do_iacl_pacl_malloc_key_field_list(self, arg):
        '''
         xpsIaclPaclMallocKeyFieldList: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fieldList_Ptr_2 = new_xpsIaclkeyFieldList_tp()
            fieldList_Ptr_pp = new_xpsIaclkeyFieldList_arr(1)
            xpsIaclkeyFieldList_arr_setitem(fieldList_Ptr_pp, 0,fieldList_Ptr_2)
            ret = xpsIaclPaclMallocKeyFieldList(args[0],args[1],fieldList_Ptr_pp)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsIaclkeyFieldList_tp(fieldList_Ptr_2)

    #/********************************************************************************/
    # command for xpsIaclBaclMallocKeyFieldList
    #/********************************************************************************/
    def do_iacl_bacl_malloc_key_field_list(self, arg):
        '''
         xpsIaclBaclMallocKeyFieldList: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fieldList_Ptr_2 = new_xpsIaclkeyFieldList_tp()
            fieldList_Ptr_pp = new_xpsIaclkeyFieldList_arr(1)
            xpsIaclkeyFieldList_arr_setitem(fieldList_Ptr_pp, 0,fieldList_Ptr_2)
            ret = xpsIaclBaclMallocKeyFieldList(args[0],args[1],fieldList_Ptr_pp)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsIaclkeyFieldList_tp(fieldList_Ptr_2)

    #/********************************************************************************/
    # command for xpsIaclRaclMallocKeyFieldList
    #/********************************************************************************/
    def do_iacl_racl_malloc_key_field_list(self, arg):
        '''
         xpsIaclRaclMallocKeyFieldList: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fieldList_Ptr_2 = new_xpsIaclkeyFieldList_tp()
            fieldList_Ptr_pp = new_xpsIaclkeyFieldList_arr(1)
            xpsIaclkeyFieldList_arr_setitem(fieldList_Ptr_pp, 0,fieldList_Ptr_2)
            ret = xpsIaclRaclMallocKeyFieldList(args[0],args[1],fieldList_Ptr_pp)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsIaclkeyFieldList_tp(fieldList_Ptr_2)

    #/********************************************************************************/
    # command for xpsIaclGetPaclKeySize
    #/********************************************************************************/
    def do_iacl_get_pacl_key_size(self, arg):
        '''
         xpsIaclGetPaclKeySize: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            keySize_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsIaclGetPaclKeySize(args[0],keySize_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('keySize = %d' % (uint32_tp_value(keySize_Ptr_1)))
                pass
            delete_uint32_tp(keySize_Ptr_1)

    #/********************************************************************************/
    # command for xpsIaclGetBaclKeySize
    #/********************************************************************************/
    def do_iacl_get_bacl_key_size(self, arg):
        '''
         xpsIaclGetBaclKeySize: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            keySize_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsIaclGetBaclKeySize(args[0],keySize_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('keySize = %d' % (uint32_tp_value(keySize_Ptr_1)))
                pass
            delete_uint32_tp(keySize_Ptr_1)

    #/********************************************************************************/
    # command for xpsIaclGetRaclKeySize
    #/********************************************************************************/
    def do_iacl_get_racl_key_size(self, arg):
        '''
         xpsIaclGetRaclKeySize: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            keySize_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsIaclGetRaclKeySize(args[0],keySize_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('keySize = %d' % (uint32_tp_value(keySize_Ptr_1)))
                pass
            delete_uint32_tp(keySize_Ptr_1)

    #/********************************************************************************/
    # command for xpsIaclGetPaclKeyNumFlds
    #/********************************************************************************/
    def do_iacl_get_pacl_key_num_flds(self, arg):
        '''
         xpsIaclGetPaclKeyNumFlds: Enter [ devId,keyType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,keyType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            numFlds_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, keyType=%d' % (args[0],args[1]))
            ret = xpsIaclGetPaclKeyNumFlds(args[0],args[1],numFlds_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numFlds = %d' % (uint32_tp_value(numFlds_Ptr_2)))
                pass
            delete_uint32_tp(numFlds_Ptr_2)

    #/********************************************************************************/
    # command for xpsIaclGetBaclKeyNumFlds
    #/********************************************************************************/
    def do_iacl_get_bacl_key_num_flds(self, arg):
        '''
         xpsIaclGetBaclKeyNumFlds: Enter [ devId,keyType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,keyType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            numFlds_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, keyType=%d' % (args[0],args[1]))
            ret = xpsIaclGetBaclKeyNumFlds(args[0],args[1],numFlds_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numFlds = %d' % (uint32_tp_value(numFlds_Ptr_2)))
                pass
            delete_uint32_tp(numFlds_Ptr_2)

    #/********************************************************************************/
    # command for xpsIaclGetRaclKeyNumFlds
    #/********************************************************************************/
    def do_iacl_get_racl_key_num_flds(self, arg):
        '''
         xpsIaclGetRaclKeyNumFlds: Enter [ devId,keyType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,keyType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            numFlds_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, keyType=%d' % (args[0],args[1]))
            ret = xpsIaclGetRaclKeyNumFlds(args[0],args[1],numFlds_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numFlds = %d' % (uint32_tp_value(numFlds_Ptr_2)))
                pass
            delete_uint32_tp(numFlds_Ptr_2)

    #/********************************************************************************/
    # command for xpsIaclGetPaclEntryKeyMask
    #/********************************************************************************/
    def do_iacl_get_pacl_entry_key_mask(self, arg):
        '''
         xpsIaclGetPaclEntryKeyMask: Enter [ devId,camIndex,flds(expected (,) sepreted flds) ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,flds ] \n NOTE:-Arguments flds, value and mask expect (,) seprated i/p values. check iacl_help command for example')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fldList = args[2].strip().split(',')
            xpIaclKeyMask_t_Ptr = new_xpIaclKeyMask_tp()
            keySize = new_uint32_tp()
            ret = xpsIaclGetPaclKeySize(args[0],keySize )
            byteLen = uint32_tp_value(keySize)
            remainder = byteLen % 8
            if (remainder):
               byteLen = byteLen/8 + 1
            else:
                byteLen /= 8
            value = new_uint8Arr_tp(byteLen)
            mask = new_uint8Arr_tp(byteLen)
            xpIaclKeyMask_t_Ptr.value = value 
            xpIaclKeyMask_t_Ptr.mask = mask
            fldList = args[2].strip().split(',')
            #print('Input Arguments are, devId=%d, camIndex=%d flds=%s' % (args[0],args[1],fldList))
            ret = xpsIaclGetPaclEntryKeyMask(args[0],args[1],xpIaclKeyMask_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                Idx = 0
                for fldIdx in range(len(fldList)):
                    try:
                        fldByteSize =mapxpIaclV4KeyByteMask[eval(fldList[fldIdx])]
                    except KeyError:
                        fldByteSize =mapxpIaclV6KeyByteMask[eval(fldList[fldIdx])]
                    fldByteLen = bin(fldByteSize).count('1')
                    valueLst = []
                    maskLst = []
                    for valIdx in range(fldByteLen):
                        valX = uint8Arr_tp_getitem(xpIaclKeyMask_t_Ptr.value, Idx)
                        maskX = uint8Arr_tp_getitem(xpIaclKeyMask_t_Ptr.mask, Idx)
                        if(remainder):
                           valX &= 0XFF >> (8 - remainder)
                           maskX &= 0XFF >> (8 - remainder)
                        if(valX == 0 and maskX == 0):
                            valueLst.append(str(valX))
                            maskLst.append(str(maskX))
                        elif(valX == 0):
                            valueLst.append(str(valX))
                            maskLst.append(hex(maskX).replace('0x','').rstrip('L'))
                        elif(maskX == 0):
                            maskLst.append(str(maskX))
                            valueLst.append(hex(valX).replace('0x','').rstrip('L'))
                        else:
                            maskLst.append(hex(maskX).replace('0x','').rstrip('L'))
                            valueLst.append(hex(valX).replace('0x','').rstrip('L'))
                        Idx = Idx + 1
                    if("IP" in fldList[fldIdx] or "MAC" in fldList[fldIdx]):
                        print(str('value'+str(fldIdx)) + '= %s' % (valueLst))
                        print(str('mask'+str(fldIdx)) + '= %s' % (maskLst))
                    else:
                        value = ''.join(reversed(valueLst))
                        mask = ''.join(reversed(maskLst))
                        print(str('value'+str(fldIdx)) + '= %d' % (int(value,16)))
                        print(str('mask'+str(fldIdx)) + '= %s' % (mask))
                pass
            delete_uint32_tp(keySize)
            delete_uint8Arr_tp(value)
            delete_uint8Arr_tp(mask)
            delete_xpIaclKeyMask_tp(xpIaclKeyMask_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclGetBaclEntryKeyMask
    #/********************************************************************************/
    def do_iacl_get_bacl_entry_key_mask(self, arg):
        '''
         xpsIaclGetBaclEntryKeyMask: Enter [ devId,camIndex,flds ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,flds ] \n NOTE:-Arguments flds, value and mask expect (,) seprated i/p values. check iacl_help command for example')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fldList = args[2].strip().split(',')
            xpIaclKeyMask_t_Ptr = new_xpIaclKeyMask_tp()
            keySize = new_uint32_tp()
            numFlds_ptr = new_uint32_tp()
            ret = xpsIaclGetBaclKeySize(args[0],keySize )
            byteLen = uint32_tp_value(keySize)
            remainder = byteLen % 8
            if (remainder):
               byteLen = byteLen/8 + 1
            else:
               byteLen /= 8
            value = new_uint8Arr_tp(byteLen)
            mask = new_uint8Arr_tp(byteLen)
            xpIaclKeyMask_t_Ptr.value = value 
            xpIaclKeyMask_t_Ptr.mask = mask
            fldList = args[2].strip().split(',')
            #print('Input Arguments are, devId=%d, camIndex=%d flds=%s' % (args[0],args[1],fldList))
            ret = xpsIaclGetBaclEntryKeyMask(args[0],args[1],xpIaclKeyMask_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                Idx = 0
                for fldIdx in range(len(fldList)):
                    try:
                        fldByteSize =mapxpIaclV4KeyByteMask[eval(fldList[fldIdx])]
                    except KeyError:
                        fldByteSize =mapxpIaclV6KeyByteMask[eval(fldList[fldIdx])]
                    fldByteLen = bin(fldByteSize).count('1')
                    valueLst = []
                    maskLst = []
                    for valIdx in range(fldByteLen):
                        valX = uint8Arr_tp_getitem(xpIaclKeyMask_t_Ptr.value, Idx)
                        maskX = uint8Arr_tp_getitem(xpIaclKeyMask_t_Ptr.mask, Idx)
                        if(remainder):
                           valX &= 0XFF >> (8 - remainder)
                           maskX &= 0XFF >> (8 - remainder)
                        if(valX == 0 and maskX == 0):
                            valueLst.append(str(valX))
                            maskLst.append(str(maskX))
                        elif(valX == 0):
                            valueLst.append(str(valX))
                            maskLst.append(hex(maskX).replace('0x','').rstrip('L'))
                        elif(maskX == 0):
                            maskLst.append(str(maskX))
                            valueLst.append(hex(valX).replace('0x','').rstrip('L'))
                        else:
                            maskLst.append(hex(maskX).replace('0x','').rstrip('L'))
                            valueLst.append(hex(valX).replace('0x','').rstrip('L'))
                        Idx = Idx + 1
                    if("IP" in fldList[fldIdx] or "MAC" in fldList[fldIdx]):
                        print(str('value'+str(fldIdx)) + '= %s' % (valueLst))
                        print(str('mask'+str(fldIdx)) + '= %s' % (maskLst))
                    else:
                        value = ''.join(reversed(valueLst))
                        mask = ''.join(reversed(maskLst))
                        print(str('value'+str(fldIdx)) + '= %d' % (int(value,16)))
                        print(str('mask'+str(fldIdx)) + '= %s' % (mask))
                pass
            delete_uint32_tp(keySize)
            delete_uint8Arr_tp(value)
            delete_uint8Arr_tp(mask)
            delete_xpIaclKeyMask_tp(xpIaclKeyMask_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclGetRaclEntryKeyMask
    #/********************************************************************************/
    def do_iacl_get_racl_entry_key_mask(self, arg):
        '''
         xpsIaclGetRaclEntryKeyMask: Enter [ devId,camIndex,flds]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,flds ] \n NOTE:-Arguments flds, value and mask expect (,) seprated i/p values. check iacl_help command for example')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fldList = args[2].strip().split(',')
            xpIaclKeyMask_t_Ptr = new_xpIaclKeyMask_tp()
            keySize = new_uint32_tp()
            ret = xpsIaclGetRaclKeySize(args[0],keySize )
            byteLen = uint32_tp_value(keySize)
            remainder = byteLen % 8
            if (remainder):
               byteLen = byteLen/8 + 1
            else:
               byteLen /= 8
            value = new_uint8Arr_tp(byteLen)
            mask = new_uint8Arr_tp(byteLen)
            xpIaclKeyMask_t_Ptr.value = value 
            xpIaclKeyMask_t_Ptr.mask = mask
            fldList = args[2].strip().split(',')
            #print('Input Arguments are, devId=%d, camIndex=%d flds=%s' % (args[0],args[1],fldList))
            ret = xpsIaclGetRaclEntryKeyMask(args[0],args[1],xpIaclKeyMask_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                Idx = 0
                for fldIdx in range(len(fldList)):
                    try:
                        fldByteSize =mapxpIaclV4KeyByteMask[eval(fldList[fldIdx])]
                    except KeyError:
                        fldByteSize =mapxpIaclV6KeyByteMask[eval(fldList[fldIdx])]
                    fldByteLen = bin(fldByteSize).count('1')
                    valueLst = []
                    maskLst = []
                    for valIdx in range(fldByteLen):
                        valX = uint8Arr_tp_getitem(xpIaclKeyMask_t_Ptr.value, Idx)
                        maskX = uint8Arr_tp_getitem(xpIaclKeyMask_t_Ptr.mask, Idx)
                        if(remainder):
                           valX &= 0XFF >> (8 - remainder)
                           maskX &= 0XFF >> (8 - remainder)
                        if(valX == 0 and maskX == 0):
                            valueLst.append(str(valX))
                            maskLst.append(str(maskX))
                        elif(valX == 0):
                            valueLst.append(str(valX))
                            maskLst.append(hex(maskX).replace('0x','').rstrip('L'))
                        elif(maskX == 0):
                            maskLst.append(str(maskX))
                            valueLst.append(hex(valX).replace('0x','').rstrip('L'))
                        else:
                            maskLst.append(hex(maskX).replace('0x','').rstrip('L'))
                            valueLst.append(hex(valX).replace('0x','').rstrip('L'))
                        Idx = Idx + 1
                    if("IP" in fldList[fldIdx] or "MAC" in fldList[fldIdx]):
                        print(str('value'+str(fldIdx)) + '= %s' % (valueLst))
                        print(str('mask'+str(fldIdx)) + '= %s' % (maskLst))
                    else:
                        value = ''.join(reversed(valueLst))
                        mask = ''.join(reversed(maskLst))
                        print(str('value'+str(fldIdx)) + '= %d' % (int(value,16)))
                        print(str('mask'+str(fldIdx)) + '= %s' % (mask))
                pass
            delete_uint32_tp(keySize)
            delete_uint8Arr_tp(value)
            delete_uint8Arr_tp(mask)
            delete_xpIaclKeyMask_tp(xpIaclKeyMask_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclGetNumOfValidPaclEntries
    #/********************************************************************************/
    def do_iacl_get_num_of_valid_pacl_entries(self, arg):
        '''
         xpsIaclGetNumOfValidPaclEntries: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            noOfEntries_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsIaclGetNumOfValidPaclEntries(args[0],noOfEntries_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('noOfEntries = %d' % (uint32_tp_value(noOfEntries_Ptr_1)))
                pass
            delete_uint32_tp(noOfEntries_Ptr_1)

    #/********************************************************************************/
    # command for xpsIaclGetNumOfValidBaclEntries
    #/********************************************************************************/
    def do_iacl_get_num_of_valid_bacl_entries(self, arg):
        '''
         xpsIaclGetNumOfValidBaclEntries: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            noOfEntries_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsIaclGetNumOfValidBaclEntries(args[0],noOfEntries_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('noOfEntries = %d' % (uint32_tp_value(noOfEntries_Ptr_1)))
                pass
            delete_uint32_tp(noOfEntries_Ptr_1)

    #/********************************************************************************/
    # command for xpsIaclGetNumOfValidRaclEntries
    #/********************************************************************************/
    def do_iacl_get_num_of_valid_racl_entries(self, arg):
        '''
         xpsIaclGetNumOfValidRaclEntries: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            noOfEntries_Ptr_1 = new_uint32_tp()
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsIaclGetNumOfValidRaclEntries(args[0],noOfEntries_Ptr_1)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('noOfEntries = %d' % (uint32_tp_value(noOfEntries_Ptr_1)))
                pass
            delete_uint32_tp(noOfEntries_Ptr_1)
    
    #/********************************************************************************/
    # command for xpsIaclReadPaclEntry
    #/********************************************************************************/
    def do_iacl_read_pacl_entry(self, arg):
        '''
         xpsIaclReadPaclEntry: Enter [ devId,index]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fieldList_Ptr_2 = new_xpsIaclkeyFieldList_tp()
            fieldList_Ptr_pp = new_xpsIaclkeyFieldList_arr(1)
            xpsIaclkeyFieldList_arr_setitem(fieldList_Ptr_pp, 0,fieldList_Ptr_2)
            xpIaclkeyFieldList_t_Ptr = xpsIaclkeyFieldList_arr_getitem(fieldList_Ptr_pp, 0)
            xpsIaclData_t_Ptr = new_xpsIaclData_tp()
            #print('Input Arguments are, devId=%d, index=%d, numFlds=%d, isValid=%d, type=%d, fld=%s' % (args[0],args[1],xpIaclkeyFieldList_t_Ptr.numFlds,xpIaclkeyFieldList_t_Ptr.isValid,xpIaclkeyFieldList_t_Ptr.type,fldList))
            ret = xpsIaclReadPaclEntry(args[0],args[1],xpIaclkeyFieldList_t_Ptr,xpsIaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('****DataFields****')
                print('isTerminal = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.isTerminal))
                print('enPktCmdUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enPktCmdUpd))
                print('enRedirectToEvif = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif))
                print('enPolicer = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enPolicer))
                print('enMirrorSsnUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enMirrorSsnUpd))
                print('enTcRemark = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enTcRemark))
                print('enDpRemark = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enDpRemark))
                print('enDscpRemarkOrVrfUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd))
                print('enPcpRemarkOrStagRw = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw))
                print('qosOrVlanRw = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw))
                print('encapType = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.encapType))
                print('mirrorSessionId = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.mirrorSessionId))
                print('policerId = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.policerId))
                print('tc = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.tc))
                print('dp = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.dp))
                if(xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif == 1):
                    print('evifId = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.eVifId))
                    print('ecmpSize = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.ecmpSize))
                else:
                    print('pktCmd= %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command))
                    print('rsnCode = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode))

                if xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw == 1:
                    print('dscp = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp))
                    print('pcp = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp))
                else:
                    print('vlan = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.vlan))
            delete_xpsIaclData_tp(xpsIaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclReadBaclEntry
    #/********************************************************************************/
    def do_iacl_read_bacl_entry(self, arg):
        '''
         xpsIaclReadBaclEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fieldList_Ptr_2 = new_xpsIaclkeyFieldList_tp()
            fieldList_Ptr_pp = new_xpsIaclkeyFieldList_arr(1)
            xpsIaclkeyFieldList_arr_setitem(fieldList_Ptr_pp, 0,fieldList_Ptr_2)
            xpIaclkeyFieldList_t_Ptr = xpsIaclkeyFieldList_arr_getitem(fieldList_Ptr_pp, 0)
            xpsIaclData_t_Ptr = new_xpsIaclData_tp()
            #print('Input Arguments are, devId=%d, index=%d, numFlds=%d, isValid=%d, type=%d, fld=%s' % (args[0],args[1],xpIaclkeyFieldList_t_Ptr.numFlds,xpIaclkeyFieldList_t_Ptr.isValid,xpIaclkeyFieldList_t_Ptr.type,fldList))
            ret = xpsIaclReadBaclEntry(args[0],args[1],xpIaclkeyFieldList_t_Ptr,xpsIaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('****DataFields****')
                print('isTerminal = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.isTerminal))
                print('enPktCmdUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enPktCmdUpd))
                print('enRedirectToEvif = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif))
                print('enPolicer = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enPolicer))
                print('enMirrorSsnUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enMirrorSsnUpd))
                print('enTcRemark = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enTcRemark))
                print('enDpRemark = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enDpRemark))
                print('enDscpRemarkOrVrfUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd))
                print('enPcpRemarkOrStagRw = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw))
                print('qosOrVlanRw = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw))
                print('encapType = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.encapType))
                print('pktCmd= %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmd))
                print('mirrorSessionId = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.mirrorSessionId))
                print('rsnCode = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode))
                print('policerId = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.policerId))
                print('tc = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.tc))
                print('dp = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.dp))
                if xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw == 1:
                    print('dscp = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp))
                    print('pcp = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp))
                else:
                    print('vlan = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.vlan))
            delete_xpsIaclData_tp(xpsIaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclReadRaclEntry
    #/********************************************************************************/
    def do_iacl_read_racl_entry(self, arg):
        '''
         xpsIaclReadRaclEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            fieldList_Ptr_2 = new_xpsIaclkeyFieldList_tp()
            fieldList_Ptr_pp = new_xpsIaclkeyFieldList_arr(1)
            xpsIaclkeyFieldList_arr_setitem(fieldList_Ptr_pp, 0,fieldList_Ptr_2)
            xpIaclkeyFieldList_t_Ptr = xpsIaclkeyFieldList_arr_getitem(fieldList_Ptr_pp, 0)
            xpsIaclData_t_Ptr = new_xpsIaclData_tp()
            #print('Input Arguments are, devId=%d, index=%d, numFlds=%d, isValid=%d, type=%d, fld=%s' % (args[0],args[1],xpIaclkeyFieldList_t_Ptr.numFlds,xpIaclkeyFieldList_t_Ptr.isValid,xpIaclkeyFieldList_t_Ptr.type,fldList))
            ret = xpsIaclReadRaclEntry(args[0],args[1],xpIaclkeyFieldList_t_Ptr,xpsIaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('****DataFields****')
                print('isTerminal = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.isTerminal))
                print('enPktCmdUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enPktCmdUpd))
                print('enRedirectToEvif = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif))
                print('enPolicer = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enPolicer))
                print('enMirrorSsnUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enMirrorSsnUpd))
                print('enTcRemark = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enTcRemark))
                print('enDpRemark = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enDpRemark))
                print('enDscpRemarkOrVrfUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd))
                print('enPcpRemarkOrStagRw = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw))
                print('qosOrVlanRw = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw))
                print('encapType = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.encapType))
                print('pktCmd= %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmd))
                print('mirrorSessionId = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.mirrorSessionId))
                print('rsnCode = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode))
                print('policerId = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.policerId))
                print('tc = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.tc))
                print('dp = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.dp))
                if xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw == 1:
                    print('dscp = %d' % ( xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp))
                    print('pcp = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp))
                else:
                    print('vlan = %d' % ( xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.vlan))
            delete_xpsIaclData_tp(xpsIaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclReadPaclKeyField
    #/********************************************************************************/
    def do_iacl_read_pacl_entry_field(self, arg):
        '''
         xpsIaclReadPaclKeyField: Enter [ devId,index,fldId]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,fldId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpIaclKeyFieldId_t_ptr = xpIaclKeyFieldId_t()
            v4FldList = ['XP_IACL_KEY_TYPE_V4','XP_IACL_ID','XP_IACL_MAC_DA','XP_IACL_MAC_SA','XP_IACL_V4_ETHER_TYPE','XP_IACL_CTAG_VID_DEI_PCP','XP_IACL_STAG_VID_DEI_PCP','XP_IACL_DIP_V4','XP_IACL_SIP_V4','XP_IACL_L4_DEST_PORT','XP_IACL_L4_SRC_PORT','XP_IACL_IVIF','XP_IACL_ICMP_CODE','XP_IACL_ICMP_MSG_TYPE','XP_IACL_PROTOCOL','XP_IACL_DSCP_ECN','XP_IACL_BD','XP_IACL_TTL','XP_IACL_PKTLEN','XP_IACL_TCP_FLAGS','XP_IACL_VRF_ID','XP_IACL_TAG_FRAGMENT_INFO']
            v6FldList = ['XP_IACL_KEY_TYPE_V6','XP_IACL_V6_ID','XP_IACL_DIP_V6','XP_IACL_SIP_V6','XP_IACL_NXT_HDR','XP_IACL_L4_V6_DEST_PORT','XP_IACL_L4_V6_SRC_PORT','XP_IACL_ICMP_V6_CODE','XP_IACL_ICMP_V6_MSG_TYPE','XP_IACL_HOP_LIMIT','XP_IACL_V6_PROTOCOL','XP_IACL_V6_TCP_FLAGS','XP_IACL_V6_PKTLEN','XP_IACL_TC_ROUTERMAC']
            if(args[2] in v4FldList):
                xpIaclKeyFieldId_t_ptr.v4Fld = eval(args[2])
                byteSize =mapxpIaclV4KeyByteMask[eval(args[2])]
            elif(args[2] in v6FldList):
                xpIaclKeyFieldId_t_ptr.v6Fld = eval(args[2])
                byteSize =mapxpIaclV6KeyByteMask[eval(args[2])]
            byteLen = bin(byteSize).count('1')
            fieldVal_Ptr_3 = new_uint8Arr_tp(byteLen)
            fieldMask_Ptr_4 = new_uint8Arr_tp(byteLen)
            ret = xpsIaclReadPaclKeyField(args[0],args[1],xpIaclKeyFieldId_t_ptr,fieldVal_Ptr_3,fieldMask_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            delete_uint8_tp(fieldMask_Ptr_4)
            delete_uint8_tp(fieldVal_Ptr_3)

    #/********************************************************************************/
    # command for xpsIaclReadBaclKeyField
    #/********************************************************************************/
    def do_iacl_read_bacl_entry_field(self, arg):
        '''
         xpsIaclReadBaclKeyField: Enter [ devId,index,fldId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,fldId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpIaclKeyFieldId_t_ptr = xpIaclKeyFieldId_t()
            v4FldList = ['XP_IACL_KEY_TYPE_V4','XP_IACL_ID','XP_IACL_MAC_DA','XP_IACL_MAC_SA','XP_IACL_V4_ETHER_TYPE','XP_IACL_CTAG_VID_DEI_PCP','XP_IACL_STAG_VID_DEI_PCP','XP_IACL_DIP_V4','XP_IACL_SIP_V4','XP_IACL_L4_DEST_PORT','XP_IACL_L4_SRC_PORT','XP_IACL_IVIF','XP_IACL_ICMP_CODE','XP_IACL_ICMP_MSG_TYPE','XP_IACL_PROTOCOL','XP_IACL_DSCP_ECN','XP_IACL_BD','XP_IACL_TTL','XP_IACL_PKTLEN','XP_IACL_TCP_FLAGS','XP_IACL_VRF_ID','XP_IACL_TAG_FRAGMENT_INFO']
            v6FldList = ['XP_IACL_KEY_TYPE_V6','XP_IACL_V6_ID','XP_IACL_DIP_V6','XP_IACL_SIP_V6','XP_IACL_NXT_HDR','XP_IACL_L4_V6_DEST_PORT','XP_IACL_L4_V6_SRC_PORT','XP_IACL_ICMP_V6_CODE','XP_IACL_ICMP_V6_MSG_TYPE','XP_IACL_HOP_LIMIT','XP_IACL_V6_PROTOCOL','XP_IACL_V6_TCP_FLAGS','XP_IACL_V6_PKTLEN','XP_IACL_TC_ROUTERMAC']
            if(args[2] in v4FldList):
                xpIaclKeyFieldId_t_ptr.v4Fld = eval(args[2])
                byteSize = mapxpIaclV4KeyByteMask[eval(args[2])]
            elif(args[2] in v6FldList):
                xpIaclKeyFieldId_t_ptr.v6Fld = eval(args[2])
                byteSize = mapxpIaclV6KeyByteMask[eval(args[2])]
            byteLen =  bin(byteSize).count('1')
            fieldVal_Ptr_3 = new_uint8Arr_tp(byteLen)
            fieldMask_Ptr_4 = new_uint8Arr_tp(byteLen)
            #print('Input Arguments are, devId=%d, index=%d, fldId=%s' % (args[0],args[1],args[2]))
            ret = xpsIaclReadBaclKeyField(args[0],args[1],xpIaclKeyFieldId_t_ptr,fieldVal_Ptr_3,fieldMask_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            delete_uint8_tp(fieldMask_Ptr_4)
            delete_uint8_tp(fieldVal_Ptr_3)

    #/********************************************************************************/
    # command for xpsIaclReadRaclKeyField
    #/********************************************************************************/
    def do_iacl_read_racl_entry_field(self, arg):
        '''
         xpsIaclReadRaclKeyField: Enter [ devId,index,fldId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,fldId ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            xpIaclKeyFieldId_t_ptr = xpIaclKeyFieldId_t()
            v4FldList = ['XP_IACL_KEY_TYPE_V4','XP_IACL_ID','XP_IACL_MAC_DA','XP_IACL_MAC_SA','XP_IACL_V4_ETHER_TYPE','XP_IACL_CTAG_VID_DEI_PCP','XP_IACL_STAG_VID_DEI_PCP','XP_IACL_DIP_V4','XP_IACL_SIP_V4','XP_IACL_L4_DEST_PORT','XP_IACL_L4_SRC_PORT','XP_IACL_IVIF','XP_IACL_ICMP_CODE','XP_IACL_ICMP_MSG_TYPE','XP_IACL_PROTOCOL','XP_IACL_DSCP_ECN','XP_IACL_BD','XP_IACL_TTL','XP_IACL_PKTLEN','XP_IACL_TCP_FLAGS','XP_IACL_VRF_ID','XP_IACL_TAG_FRAGMENT_INFO']
            v6FldList = ['XP_IACL_KEY_TYPE_V6','XP_IACL_V6_ID','XP_IACL_DIP_V6','XP_IACL_SIP_V6','XP_IACL_NXT_HDR','XP_IACL_L4_V6_DEST_PORT','XP_IACL_L4_V6_SRC_PORT','XP_IACL_ICMP_V6_CODE','XP_IACL_ICMP_V6_MSG_TYPE','XP_IACL_HOP_LIMIT','XP_IACL_V6_PROTOCOL','XP_IACL_V6_TCP_FLAGS','XP_IACL_V6_PKTLEN','XP_IACL_TC_ROUTERMAC']
            if(args[2] in v4FldList):
                xpIaclKeyFieldId_t_ptr.v4Fld = eval(args[2])
                byteSize =mapxpIaclV4KeyByteMask[eval(args[2])]
            elif(args[2] in v6FldList):
                xpIaclKeyFieldId_t_ptr.v6Fld = eval(args[2])
                byteSize =mapxpIaclV6KeyByteMask[eval(args[2])]
            byteLen =  bin(byteSize).count('1')
            fieldVal_Ptr_3 = new_uint8Arr_tp(byteLen)
            fieldMask_Ptr_4 = new_uint8Arr_tp(byteLen)
            #print('Input Arguments are, devId=%d, index=%d, fldId=%s' % (args[0],args[1],args[2]))
            ret = xpsIaclReadRaclKeyField(args[0],args[1],xpIaclKeyFieldId_t_ptr,fieldVal_Ptr_3,fieldMask_Ptr_4)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            delete_uint8_tp(fieldMask_Ptr_4)
            delete_uint8_tp(fieldVal_Ptr_3)

    #/********************************************************************************/
    # command for xpsIaclSetRuleValid
    #/********************************************************************************/
    def do_iacl_set_rule_valid(self, arg):
        '''
         xpsIaclSetRuleValid: Enter [ devId,iaclType,index,valid ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,iaclType,index,valid ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, iaclType=%d, index=%d, valid=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsIaclSetRuleValid(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Error returned = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIaclGetRuleValidBit
    #/********************************************************************************/
    def do_iacl_get_rule_valid_bit(self, arg):
        '''
         xpsIaclGetRuleValidBit: Enter [ devId,iaclType,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,iaclType,index ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            valid_Ptr_3 = new_uint8_tp()
            #print('Input Arguments are, devId=%d, iaclType=%d, index=%d' % (args[0],args[1],args[2]))
            ret = xpsIaclGetRuleValidBit(args[0],args[1],args[2],valid_Ptr_3)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                print('valid = %d' % (uint8_tp_value(valid_Ptr_3)))
                pass
            delete_uint8_tp(valid_Ptr_3)

    #/********************************************************************************/
    # command for iacl help 
    #/********************************************************************************/
    def do_iacl_help(self, arg):
        '''
         xpsIaclInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            print('Input Arguments are : Not required' % ())
            helpBuff = "================================ IACL_configGuide===================================\n \
            use help <CMD>, required input argument \n\n \
            1-Initialize the iacl module if not already done by the SDK \n \
            e.g iacl_init  \n\n \
            2-Create table for IACL module for all the three types of ACL \n \
            port,bridge and router along with the keySize \n \
            e.g iacl_create_table 0 2 XP_IACL0,XP_IACL1 390,390 1,1\n\n \
            3-Define the key for port,bridge and router \n \
            e.g iacl_define_pacl_key 0 XP_IACL_V4_TYPE 6 1 XP_IACL_KEY_TYPE_V4,XP_IACL_ID,XP_IACL_CTAG_VID_DEI_PCP,XP_IACL_DSCP_ECN,XP_IACL_MAC_DA,XP_IACL_MAC_SA \n\n \
            4-Write the key values into the HW for port,bridge and router. \n \
            e.g iacl_write_pacl_key 0 0 6 1 XP_IACL_V4_TYPE XP_IACL_KEY_TYPE_V4,0,0 XP_IACL_ID,1,0 XP_IACL_CTAG_VID_DEI_PCP,21763,0 XP_IACL_DSCP_HAS_CTAG_STAG,1,0 XP_IACL_MAC_DA,0:0:0:0:0:0,0 XP_IACL_MAC_SA,0:0:0:0:0:0,0 \n \
            At last append key want to be program in this formate 'fld,vlaue,mask'. numFlds and number of keys you append must match.\n \
            *For mask, \n \
            0 : to set all mask bit to 0000 0000 \n \
            1 : to set all mask bit to 1111 1111 \n \
            63 : to set particular bit mask i.e [ 63 in dec = 0011 1111] \n\n \
            255 : To set only mac and Ip key mask to all 1's  e.g. XP_IACL_MAC_SA,0:0:0:0:0:0,255\n \
            5-Write rule data into the HW for port,bridge and router. \n \
            e.g iacl_write_pacl_data 0 0 0 1 0 1 0 0 0 0 0 0 1 0 0 0 0 0 19 0 0 0 850\n\n \
            6-Set the valid bit for a particular ACL rule . \n \
            e.g iacl_set_rule_valid 0 XP_IACL0 1 1 \n\n \
            7-Delete table for IACL module for a acl type. \n \
            e.g iacl_delete_table 0 XP_IACL0 \n\n \
            Possible KeyType : XP_IACL_V4_TYPE and XP_IACL_V6_TYPE \n \
            Possible Fields in IPV4 key : XP_IACL_KEY_TYPE_V4,XP_IACL_ID,XP_IACL_MAC_DA,XP_IACL_MAC_SA,XP_IACL_V4_ETHER_TYPE,XP_IACL_CTAG_VID_DEI_PCP,XP_IACL_STAG_VID_DEI_PCP, \n \
                                          XP_IACL_DIP_V4,XP_IACL_SIP_V4,XP_IACL_L4_DEST_PORT,XP_IACL_L4_SRC_PORT,XP_IACL_IVIF,XP_IACL_ICMP_CODE,XP_IACL_ICMP_MSG_TYPE,XP_IACL_PROTOCOL, \n \
                                          XP_IACL_DSCP_ECN,XP_IACL_BD,XP_IACL_TTL,XP_IACL_PKTLEN,XP_IACL_TCP_FLAGS,XP_IACL_VRF_ID,XP_IACL_TAG_FRAGMENT_INFO \n \
            Possible Fields in IPV6 Key : XP_IACL_KEY_TYPE_V6,XP_IACL_V6_ID,XP_IACL_DIP_V6,XP_IACL_SIP_V6,XP_IACL_NXT_HDR,XP_IACL_L4_V6_DEST_PORT, \n \
                                          XP_IACL_L4_V6_SRC_PORT,XP_IACL_ICMP_V6_MSG_TYPE,XP_IACL_HOP_LIMIT,XP_IACL_V6_PROTOCOL,XP_IACL_V6_TCP_FLAGS,XP_IACL_V6_PKTLEN,XP_IACL_TC_ROUTERMAC \n "
            print helpBuff.strip(' ')
            pass

    #/********************************************************************************/
    # command for xpsIaclSetCountingEnable
    #/********************************************************************************/
    def do_iacl_set_counting_enable(self, arg):
        '''
         xpsIaclSetCountingEnable: Enter [ devId,iaclType,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,iaclType,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, iaclType=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsIaclSetCountingEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIaclSetPolicingEnable
    #/********************************************************************************/
    def do_iacl_set_policing_enable(self, arg):
        '''
         xpsIaclSetPolicingEnable: Enter [ devId,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, enable=%d' % (args[0],args[1]))
            ret = xpsIaclSetPolicingEnable(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsIaclUpdatePaclDataField
    #/********************************************************************************/
    def do_iacl_update_pacl_data_field(self, arg):
        '''
         xpsIaclUpdatePaclDataField: Enter [ devId,camIndex,dataEnType,dataEnValue,dataType ,dataValue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,dataEnType,dataEnValue,dataType ,dataValue ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            xpIaclDataUpdate_t_Ptr = new_xpIaclDataUpdate_tp()
            xpIaclDataUpdate_t_Ptr.dataEnFlds = eval(args[2])
            xpIaclDataUpdate_t_Ptr.dataFlds = eval(args[4])
            xpIaclDataUpdate_t_Ptr.dataEnValue =int(args[3])
            xpIaclDataUpdate_t_Ptr.dataValue = int(args[5])

            #print('Input Arguments are, devId=%d, camIndex=%d, dataEnType =%d,dataEnValue=%d,dataType=%d dataValue=%d' % (args[0],args[1],xpIaclDataUpdate_t_Ptr.dataEnFlds,xpIaclDataUpdate_t_Ptr.dataEnValue,xpIaclDataUpdate_t_Ptr.dataFlds,xpIaclDataUpdate_t_Ptr.dataValue))
            ret = xpsIaclUpdatePaclDataField(args[0],args[1],xpIaclDataUpdate_t_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIaclUpdateBaclDataField
    #/********************************************************************************/
    def do_iacl_update_bacl_data_field(self, arg):
        '''
         xpsIaclUpdateBaclDataField: Enter [ devId,camIndex,dataEnType,dataEnValue,dataType ,dataValue ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [  devId,camIndex,dataEnType,dataEnValue,dataType ,dataValue]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            xpIaclDataUpdate_t_Ptr = new_xpIaclDataUpdate_tp()
            xpIaclDataUpdate_t_Ptr.dataEnFlds = eval(args[2])
            xpIaclDataUpdate_t_Ptr.dataFlds = eval(args[4])
            xpIaclDataUpdate_t_Ptr.dataEnValue =int(args[3])
            xpIaclDataUpdate_t_Ptr.dataValue = int(args[5])
            #print('Input Arguments are, devId=%d, camIndex=%d, dataEnType =%d,dataEnValue=%d,dataType=%d dataValue=%d' % (args[0],args[1],xpIaclDataUpdate_t_Ptr.dataEnFlds,xpIaclDataUpdate_t_Ptr.dataEnValue,xpIaclDataUpdate_t_Ptr.dataFlds,xpIaclDataUpdate_t_Ptr.dataValue))
            ret = xpsIaclUpdateBaclDataField(args[0],args[1],xpIaclDataUpdate_t_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIaclUpdateRaclDataField
    #/********************************************************************************/
    def do_iacl_update_racl_data_field(self, arg):
        '''
         xpsIaclUpdateRaclDataField: Enter [ devId,camIndex,dataEnType,dataEnValue,dataType,dataValue]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [  devId,camIndex,dataEnType,dataEnValue,dataType ,dataValue]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            xpIaclDataUpdate_t_Ptr = new_xpIaclDataUpdate_tp()
            xpIaclDataUpdate_t_Ptr.dataEnFlds = eval(args[2])
            xpIaclDataUpdate_t_Ptr.dataFlds = eval(args[4])
            xpIaclDataUpdate_t_Ptr.dataEnValue =int(args[3])
            xpIaclDataUpdate_t_Ptr.dataValue = int(args[5])
            #print('Input Arguments are, devId=%d, camIndex=%d, dataEnType =%d,dataEnValue=%d,dataType=%d dataValue=%d' % (args[0],args[1],xpIaclDataUpdate_t_Ptr.dataEnFlds,xpIaclDataUpdate_t_Ptr.dataEnValue,xpIaclDataUpdate_t_Ptr.dataFlds,xpIaclDataUpdate_t_Ptr.dataValue))

            ret = xpsIaclUpdateRaclDataField(args[0],args[1],xpIaclDataUpdate_t_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIaclTypeCreateTable
    #/********************************************************************************/
    def do_iacl_type_create_table(self, arg):
        '''
         xpsIaclTypeCreateTable: Enter [ devId,numTables,iaclTblType,keySize,numDb ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numTables,iaclTblType,keySize,numDb] \n <NOTE:- Arguments iaclTblType,keySize and numDb expect (,) seprated vlaue > check iacl_help command for example ')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            tblTypeList = args[2].split(',')
            defKeySize = args[3].split(',')
            numDbList = args[4].split(',')
            keySize = []
            xpIaclTableProfile_t_Ptr = xpIaclTableProfile_t()
            xpIaclTableProfile_t_Ptr.numTables = args[1]
            arrIaclTableInfo = new_xpIaclTableInfo_arr(args[1])
            for idx in range(args[1]):
                tableProfile = xpIaclTableInfo_t()
                if(int(defKeySize[idx]) <= 64):
                    keySize.append(65)
                elif(int(defKeySize[idx]) > 64 and int(defKeySize[idx]) <= 128):
                    keySize.append(130)
                elif(int(defKeySize[idx]) > 128 and int(defKeySize[idx]) <= 192):
                    keySize.append(195)
                else:
                    keySize.append(390)
                tableProfile.tblType = eval(tblTypeList[idx])
                tableProfile.keySize = keySize[idx]
                tableProfile.numDb = int(numDbList[idx])
                xpIaclTableInfo_arr_setitem(arrIaclTableInfo, idx, tableProfile)
                xpIaclTableProfile_t_Ptr.tableProfile = arrIaclTableInfo
                #print('Input Arguments are, devId=%d, numTables=3, tblType=%d, keySize=%d, numDb=%d, tblType=%d, keySize=%d, numDb=%d, tblType=%d, keySize=%d, numDb=%d' % (args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],args[8],args[9]))

            ret = xpsIaclTypeCreateTable(args[0],xpIaclTableProfile_t_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIaclTypeDeleteTable
    #/********************************************************************************/
    def do_iacl_type_delete_table(self, arg):
        '''
         xpsIaclTypeDeleteTable: Enter [ devId,iaclType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,iaclType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, iaclType=%d' % (args[0],args[1]))
            ret = xpsIaclTypeDeleteTable(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIaclTypeDefineKey
    #/********************************************************************************/
    def do_iacl_type_define_key(self, arg):
        '''
         xpsIaclTypeDefineKey: Enter [ devId,iaclType,keyType,numFlds,isValid,flds]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
             print('Invalid input, Enter [ devId,iaclType,keyType,numFlds,isValid,flds ] \n NOTE:-Arguments flds, value and mask expect (,) seprated i/p values. check iacl_help command for example')
        else:
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[3])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[4])
            xpIaclkeyFieldList_t_Ptr.type = args[2]
            fldList = args[5].strip().split(',')
            if len(fldList) != int(args[3]):
                print('\nInvalid input, numFlds and total-flds count is not match: numFlds=%d and numOfKeyFlds(flds)=%d' % (int(args[3]), len(fldList)))
                print('Enter correct numFlds and key-flds with "iacl_type_define_key" command\n')
                return

            xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
            xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
            for fldIdx,fldEnm in enumerate(fldList):
                xpIaclkeyField_t_ptr.keyFlds = eval(fldEnm)
                byteSize = mapxpIaclTypeKeyByteMask[xpIaclkeyFieldList_t_Ptr.type][eval(fldEnm)]  
                byteLen = bin(byteSize).count('1')
                valPtr = new_uint8Arr_tp(byteLen)
                maskPtr = new_uint8Arr_tp(byteLen)
                xpIaclkeyField_t_ptr.value = valPtr
                xpIaclkeyField_t_ptr.mask = maskPtr
                xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr)

            xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr
            #print('Input Arguments are, devId=%d, keyType=%s isValid=%d, numFlds=%d, fld=%s value=%s mask=%s' % (args[0],args[1],xpIaclkeyFieldList_t_Ptr.isValid,xpIaclkeyFieldList_t_Ptr.numFlds,fldList,valueList,maskList))

            #print('Input Arguments are, devId=%d, iaclType=%d, keyType=%d' % (args[0],args[1],args[2]))
            ret = xpsIaclTypeDefineKey(args[0],args[1],args[2],xpIaclkeyFieldList_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
            del valPtr
            del maskPtr
            del xpIaclkeyField_t_arr
            delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
            delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
    #/********************************************************************************/
    # command for xpsIaclTypeWriteKey
    #/********************************************************************************/
    def do_iacl_type_write_key(self, arg):
        '''
         xpsIaclTypeWriteKey: Enter [ devId,camIndex,iaclType,numFlds,isValid,type, flds,value,mask flds,value,mask .. upto the num of key wants to be program.]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
             print('Invalid input, Enter [ devId,camIndex,iaclType,numFlds,isValid,type, flds,value,mask flds,value,mask ...append flds,value,mask for num of key wants to be program] \n')
        else:
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[3])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[4])
            xpIaclkeyFieldList_t_Ptr.type = eval(args[5])
            fldList = []
            valueList = []
            maskList = []
            fldvalMask = ''
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 3:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1'
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])

            if len(fldList) != int(args[3]):
                print('\nInvalid input, numFlds and total-flds count is not match: numFlds=%d and numOfKeyFlds(flds)=%d' % (int(args[3]), len(fldList)))
                print('Enter correct numFlds and key-flds with "iacl_type_write_key" command\n')
                return

            if len(fldList) == int(args[3]) and len(valueList) == int(args[3]) and len(maskList) == int(args[3]):
                xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                       xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                       byteSize = mapxpIaclTypeKeyByteMask[xpIaclkeyFieldList_t_Ptr.type][eval(fldList[fldIdx])]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       for valIdx in range(byteLen):
                           uint8Arr_tp_setitem(valPtr, valIdx, 0)
                           uint8Arr_tp_setitem(maskPtr, valIdx,0xff)

                       if(defFld in fldList):
                           hexFlag = 0
                           fldMask = []
                           fldVal = []
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split('.')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['255'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split('.')
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split(':')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['ff'] * len(fldVal)
                               else:
                                   if(maskList[fldIdx] == '255'):
                                       fldMask = ['ff'] * len(fldVal)
                                   else:
                                       fldMask = maskList[fldIdx].strip().split(':')
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               if maskList[fldIdx] == '0':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('00')
                               elif maskList[fldIdx] == '1':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('ff')
                               else:
                                   tmp = int(maskList[fldIdx])
                                   for valIdx in range(byteLen):
                                       tmp1 = tmp%256
                                       fldMask.append(hex(tmp1).replace('0x',''))
                                       tmp = tmp/256
                                   fldMask = fldMask[::-1]
                               hexFlag = 1
                           for valIdx in range(len(fldVal)):
                               if(hexFlag):
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16))
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1],16))
                               else:
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1]))
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1]))
                           hexFlag = 0
                       xpIaclkeyField_t_ptr.value = valPtr
                       xpIaclkeyField_t_ptr.mask = maskPtr
                       xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr)
                xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr

                #print('Input Arguments are, devId=%d, camIndex=%d isValid=%d, numFlds=%d, type=%d fld=%s value=%s mask=%s' % (args[0],args[1],xpIaclkeyFieldList_t_Ptr.isValid,xpIaclkeyFieldList_t_Ptr.numFlds,xpIaclkeyFieldList_t_Ptr.type,fldList,valueList,maskList))
                ret = xpsIaclTypeWriteKey(args[0],args[1],args[2],xpIaclkeyFieldList_t_Ptr)
                err = 0
                if ret != 0:
                    print('Return Value = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                    delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
                    delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args "
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[3]
                   print "flds = %s"% fldList
    #/********************************************************************************/
    # command for xpsIaclWriteData
    #/********************************************************************************/
    def do_iacl_type_write_data(self, arg):
        '''
         xpsIaclTypeWriteData: Enter [ devId,camIndex,iaclType,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 24
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,iaclType,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmdOrEcmpSize,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            xpsIaclData_t_Ptr = new_xpsIaclData_tp()
            xpsIaclData_t_Ptr.iAclDataType.aclData.isTerminal = int(args[3])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[4])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[5])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enDpRemark = int(args[6])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enPolicer = int(args[7])
            xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[8])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[9])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enTcRemark = int(args[10])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[11])
            xpsIaclData_t_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[12])
            xpsIaclData_t_Ptr.iAclDataType.aclData.tc = int(args[14])
            xpsIaclData_t_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[15])
            xpsIaclData_t_Ptr.iAclDataType.aclData.encapType = eval(args[16])
            xpsIaclData_t_Ptr.iAclDataType.aclData.policerId = int(args[18])
            if(xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif == 1):
                xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.eVifId = int(args[17])
                xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.ecmpSize = int(args[13])
            else:
                xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command = int(args[13])
                xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode = int(args[19])
            xpsIaclData_t_Ptr.iAclDataType.aclData.policerId = int(args[18])
            xpsIaclData_t_Ptr.iAclDataType.aclData.dp = int(args[20])
            if (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw == 1):
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[21])
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[22])
            else:
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[23])
            ret = xpsIaclTypeWriteData(args[0],args[1],args[2],xpsIaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Error returned = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsIaclData_tp(xpsIaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclTypeWriteData
    #/********************************************************************************/
    def do_iacl_type_write_data_cust(self, arg):
        '''
         xpsIaclTypeWriteData: Enter [ devId,camIndex,iaclType,swicthAclCpuReasonCodeEn,enCounter,enPolicer,enSampler,enTcRemark,enPcpRemark,enDscpRemark,enPktCmdTrap,enPktCmdLog,enPktCmdDrop,enPktCmdUpd,pktCmd,enPbr,ecmpSize,reasonCode,mirrorSessionId,tc,pcp,dscp,policerId,nextHopId,enPktCmdFwd,enRedirectToEvif,evif,setMetadata,metadata,enTagVlanId,enDpRemark,dp  ]
        '''
        args = re.split(';| ',arg)
        print(args)
        numArgsReq = 30
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,iaclType,swicthAclCpuReasonCodeEn,enCounter,enPolicer,enSampler,enTcRemark,enPcpRemark,enDscpRemark,enPktCmdTrap,enPktCmdLog,enPktCmdDrop,enPktCmdUpd,pktCmd,enPbr,ecmpSize,reasonCode,mirrorSessionId,tc,pcp,dscp,policerId,nextHopId,enPktCmdFwd,enRedirectToEvif,evif,setMetadata,metadata,entagVlanId,enDpRemark,dp ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            xpsIaclData_t_Ptr = new_xpsIaclData_tp()
            xpsIaclData_t_Ptr.iAclDataType.custAclData.swicthAclCpuReasonCodeEn = int(args[3])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enCounter = int(args[4])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enPolicer = int(args[5])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enSampler = int(args[6])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enTcRemark = int(args[7])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enPcpRemark = int(args[8])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enDscpRemark = int(args[9])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdTrap = int(args[10])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdLog = int(args[11])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdDrop = int(args[12])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdUpd = int(args[13])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.pktCmd = int(args[14])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enPbr = int(args[15])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.ecmpSize = int(args[16])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.reasonCode = eval(args[17])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.mirrorSessionId = int(args[18])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.tc = int(args[19])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.pcp = int(args[20])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.dscp = eval(args[21])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.policerId = int(args[22])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.nextHopId = int(args[23])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdFwd = int(args[24])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enRedirectToEvif = int(args[25])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.egressVif = int(args[26])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.setMetadata = int(args[27])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.metadata = int(args[28])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enTagVlanId = int(args[29])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.enDpRemark = int(args[30])
            xpsIaclData_t_Ptr.iAclDataType.custAclData.dp = int(args[31])
            ret = xpsIaclTypeWriteData(args[0],args[1],args[2],xpsIaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsIaclData_tp(xpsIaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclTypeReadData
    #/********************************************************************************/
    def do_iacl_type_read_data(self, arg):
        '''
         xpsIaclTypeReadData: Enter [ devId,camIndex,iaclType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,iaclType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            xpsIaclData_t_Ptr = new_xpsIaclData_tp()
            #print('Input Arguments are, devId=%d, camIndex=%d, iaclType=%d' % (args[0],args[1],args[2]))
            ret = xpsIaclTypeReadData(args[0],args[1],args[2],xpsIaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('****DataFields****')
                print('isTerminal = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.isTerminal))
                print('enPktCmdUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enPktCmdUpd))
                print('enRedirectToEvif = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif))
                print('enPolicer = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enPolicer))
                print('enMirrorSsnUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enMirrorSsnUpd))
                print('enTcRemark = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enTcRemark))
                print('enDpRemark = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enDpRemark))
                print('enDscpRemarkOrVrfUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd))
                print('enPcpRemarkOrStagRw = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw))
                print('qosOrVlanRw = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw))
                print('encapType = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.encapType))
                print('mirrorSessionId = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.mirrorSessionId))
                print('policerId = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.policerId))
                print('tc = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.tc))
                print('dp = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.dp))
                if(xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif == 1):
                    print('evifId = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.eVifId))
                    print('ecmpSize = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.ecmpSize))
                else:
                    print('pktCmd= %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command))
                    print('rsnCode = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode))

                if xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw == 1:
                    print('dscp = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp))
                    print('pcp = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp))
                else:
                    print('vlan = %d' % (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan))
                pass
            delete_xpsIaclData_tp(xpsIaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclTypeReadData
    #/********************************************************************************/
    def do_iacl_type_read_data_cust(self, arg):
        '''
         xpsIaclTypeReadData: Enter [ devId,camIndex,iaclType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,iaclType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            xpsIaclData_t_Ptr = new_xpsIaclData_tp()
            #print('Input Arguments are, devId=%d, camIndex=%d, iaclType=%d' % (args[0],args[1],args[2]))
            ret = xpsIaclTypeReadData(args[0],args[1],args[2],xpsIaclData_t_Ptr)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('****DataFields****')
                print('swicthAclCpuReasonCodeEn = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.swicthAclCpuReasonCodeEn))
                print('enCounter = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.enCounter))
                print('enPolicer = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.enPolicer))
                print('enSampler = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.enSampler))
                print('enTcRemark = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.enTcRemark))
                print('enPcpRemark = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.enPcpRemark))
                print('enDscpRemark = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.enDscpRemark))
                print('enPktCmdTrap = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdTrap))
                print('enPktCmdLog = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdLog))
                print('enPktCmdDrop = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdDrop))
                print('enPktCmdUpd = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdUpd))
                print('pktCmd = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.pktCmd))
                print('enPbr = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.enPbr))
                print('ecmpSize = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.ecmpSize))
                print('reasonCode = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.reasonCode))
                print('mirrorSessionId = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.mirrorSessionId))
                print('tc = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.tc))
                print('pcp = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.pcp))
                print('dscp = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.dscp))
                print('policerId = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.policerId))
                print('nextHopId = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.nextHopId))
                print('enPktCmdFwd = %d' % (xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdFwd))
                pass
            delete_xpsIaclData_tp(xpsIaclData_t_Ptr)

    #/********************************************************************************/
    # command for xpsIaclTypeWriteEntry
    #/********************************************************************************/
    def do_iacl_type_write_entry(self, arg):
        '''
         xpsIaclTypeWriteEntry: Enter [ devId,camIndex,iaclType,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan flds,value,mask flds,value,mask ...upto number of keys wants to be programe ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 28
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,iaclType,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan flds,value,mask flds,value,mask ...upto number of keys wants to be programe]')
        else:
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[3])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[4])
            xpIaclkeyFieldList_t_Ptr.type = eval(args[5])
            fldList = []
            valueList = []
            maskList = []
            fldvalMask = ''
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 3:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
            if len(fldList) == int(args[3]) and len(valueList) == int(args[3]) and len(maskList) == int(args[3]):
                xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                       xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                       byteSize = mapxpIaclTypeKeyByteMask[1][eval(fldList[fldIdx])]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       for valIdx in range(byteLen):
                           uint8Arr_tp_setitem(valPtr, valIdx, 0)
                           uint8Arr_tp_setitem(maskPtr, valIdx,0xff)

                       if(defFld in fldList):
                           hexFlag = 0
                           fldMask = []
                           fldVal = []
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split('.')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['255'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split('.')
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split(':')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['ff'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split(':')
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               if maskList[fldIdx] == '0':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('00')
                               elif maskList[fldIdx] == '1':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('ff')
                               else:
                                   tmp = int(maskList[fldIdx])
                                   for valIdx in range(byteLen):
                                       tmp1 = tmp%256
                                       fldMask.append(hex(tmp1).replace('0x',''))
                                       tmp = tmp/256
                                   fldMask = fldMask[::-1]
                               hexFlag = 1
                           for valIdx in range(len(fldVal)):
                               if(hexFlag):
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16)) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1],16))
                               else:
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1])) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1]))
                           hexFlag = 0
                       xpIaclkeyField_t_ptr.value = valPtr 
                       xpIaclkeyField_t_ptr.mask = maskPtr 
                       xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr) 
                xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr
                xpsIaclData_t_Ptr = new_xpsIaclData_tp()
                xpsIaclData_t_Ptr.iAclDataType.aclData.isTerminal = int(args[6])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[7])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[8])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enDpRemark = int(args[9])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enPolicer = int(args[10])
                xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[11])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[12])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enTcRemark = int(args[13])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[14])
                xpsIaclData_t_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[15])
                xpsIaclData_t_Ptr.iAclDataType.aclData.pktCmd = int(args[16])
                xpsIaclData_t_Ptr.iAclDataType.aclData.tc = int(args[17])
                xpsIaclData_t_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[18])
                xpsIaclData_t_Ptr.iAclDataType.aclData.encapType = int(args[19])
                xpsIaclData_t_Ptr.iAclDataType.aclData.egrVifId = int(args[20])
                xpsIaclData_t_Ptr.iAclDataType.aclData.policerId = int(args[21])
                xpsIaclData_t_Ptr.iAclDataType.aclData.rsnCode = int(args[22])
                xpsIaclData_t_Ptr.iAclDataType.aclData.dp = int(args[23])
                if (xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRw == 1):
                    xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[24])
                    xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[25])
                else:
                    xpsIaclData_t_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[26])
                ret = xpsIaclTypeWriteEntry(args[0],args[1],args[2],xpIaclkeyFieldList_t_Ptr,xpsIaclData_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpsIaclData_tp(xpsIaclData_t_Ptr)
                delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
                delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args " 
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsIaclTypeWriteEntry
    #/********************************************************************************/
    def do_iacl_type_write_entry_cust(self, arg):
        '''
         xpsIaclTypeWriteEntry: Enter [ devId,camIndex,iaclType,numFlds,isValid,type,swicthAclCpuReasonCodeEn,enCounter,enPolicer,enSampler,enTcRemark,enPcpRemark,enDscpRemark,enPktCmdTrap,enPktCmdLog,enPktCmdDrop,enPbr,ecmpSize,reasonCode,mirrorSessionId,tc,pcp,dscp,policerId,nextHopId,enPktCmdFwd  flds,value,mask flds,value,mask ...upto number of keys wants to be programe ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 27
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,iaclType,numFlds,isValid,type,swicthAclCpuReasonCodeEn,enCounter,enPolicer,enSampler,enTcRemark,enPcpRemark,enDscpRemark,enPktCmdTrap,enPktCmdLog,enPktCmdDrop,enPbr,ecmpSize,reasonCode,mirrorSessionId,tc,pcp,dscp,policerId,nextHopId,enPktCmdFwd  flds,value,mask flds,value,mask ...upto number of keys wants to be programe]')
        else:
            xpIaclkeyFieldList_t_Ptr = new_xpIaclkeyFieldList_tp()
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            xpIaclkeyFieldList_t_Ptr.numFlds = int(args[3])
            xpIaclkeyFieldList_t_Ptr.isValid = int(args[4])
            xpIaclkeyFieldList_t_Ptr.type = eval(args[5])
            fldList = []
            valueList = []
            maskList = []
            fldvalMask = ''
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 3:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
            if len(fldList) == int(args[3]) and len(valueList) == int(args[3]) and len(maskList) == int(args[3]):
                xpIaclkeyField_t_ptr = new_xpIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                       xpIaclkeyField_t_ptr.keyFlds = eval(defFld)
                       byteSize = mapxpIaclTypeKeyByteMask[xpIaclkeyFieldList_t_Ptr.type][eval(fldList[fldIdx])]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       if(not defFld in fldList):
                           for valIdx in range(byteLen):
                               uint8Arr_tp_setitem(valPtr, valIdx, 0) 
                               uint8Arr_tp_setitem(maskPtr, valIdx,0xff) 
                       else:
                           hexFlag = 0
                           fldMask = []
                           fldVal = []
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split('.')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['255'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split('.')
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].strip().split(':')
                               if(len(maskList[fldIdx]) == 1):
                                   if(maskList[fldIdx] == '0'):
                                       fldMask = ['00'] * len(fldVal)
                                   else:
                                       fldMask = ['ff'] * len(fldVal)
                               else:
                                   fldMask = maskList[fldIdx].strip().split(':')
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               if maskList[fldIdx] == '0':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('00')
                               elif maskList[fldIdx] == '1':
                                   del fldMask[:]
                                   for valIdx in range(byteLen):
                                       fldMask.append('ff')
                               else:
                                   tmp = int(maskList[fldIdx])
                                   for valIdx in range(byteLen):
                                       tmp1 = tmp%256
                                       fldMask.append(hex(tmp1).replace('0x',''))
                                       tmp = tmp/256
                                   fldMask = fldMask[::-1]
                               hexFlag = 1
                           for valIdx in range(len(fldVal)):
                               if(hexFlag):
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16)) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1],16))
                               else:
                                   uint8Arr_tp_setitem(valPtr, valIdx, int(fldVal[len(fldVal) - valIdx - 1])) 
                                   uint8Arr_tp_setitem(maskPtr, valIdx, int(fldMask[len(fldMask) - valIdx - 1]))
                           hexFlag = 0
                       xpIaclkeyField_t_ptr.value = valPtr 
                       xpIaclkeyField_t_ptr.mask = maskPtr 
                       xpIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr) 
                xpIaclkeyFieldList_t_Ptr.fldList = xpIaclkeyField_t_arr
                xpsIaclData_t_Ptr = new_xpsIaclData_tp()
                xpsIaclData_t_Ptr.iAclDataType.custAclData.swicthAclCpuReasonCodeEn = int(args[6])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.enCounter = int(args[7])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.enPolicer = int(args[8])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.enSampler = int(args[9])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.enTcRemark = int(args[10])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.enPcpRemark = int(args[11])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.enDscpRemark = int(args[12])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdTrap = int(args[13])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdLog = int(args[14])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdDrop = int(args[15])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.enPbr = int(args[16])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.ecmpSize = int(args[17])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.reasonCode = int(args[18])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.mirrorSessionId = int(args[19])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.tc = int(args[20])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.pcp = int(args[21])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.dscp = int(args[22])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.policerId = int(args[23])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.nextHopId = int(args[24])
                xpsIaclData_t_Ptr.iAclDataType.custAclData.enPktCmdFwd = int(args[25])
                ret = xpsIaclTypeWriteEntry(args[0],args[1],args[2],xpIaclkeyFieldList_t_Ptr,xpsIaclData_t_Ptr)
                err = 0
                if ret != 0:
                    print('Error returned = %d' % (ret))
                    err = 1
                if err == 0:
                    pass
                delete_xpsIaclData_tp(xpsIaclData_t_Ptr)
                delete_xpIaclkeyField_tp(xpIaclkeyField_t_ptr)
                delete_xpIaclkeyFieldList_tp(xpIaclkeyFieldList_t_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args " 
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsIaclTypeUpdateKeyField
    #/********************************************************************************/
    def do_iacl_type_update_key_field(self, arg):
        '''
         xpsIaclTypeUpdateKeyField: Enter [ devId,camIndex,iaclType,keyType,fldId,fieldVal,fieldMask]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,iaclType,keyType,fldId,fieldVal,fieldMask ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = eval(args[3])
            args[4] = eval(args[4])
            byteSize = mapxpIaclTypeKeyByteMask[args[3]][args[4]]
            byteLen = bin(byteSize).count('1')
            fieldVal_Ptr_4 = new_uint8Arr_tp(byteLen)
            fieldMask_Ptr_5 =  new_uint8Arr_tp(byteLen)
            fldVal = []
            fldMask = []
            hexFlg = 0
            if('.' in args[5]):
                fldVal = args[5].strip().split('.')
                if(len(args[6]) == 1):
                    if(args[6] == '0'):
                        fldMask = ['00'] * len(fldVal)
                    else:
                        fldMask = ['255'] * len(fldVal)
                else:
                    fldMask = args[6].strip().split('.')
            elif(':' in args[5]):
                hexFlg = 1
                fldVal = args[5].strip().split(':')
                if(len(args[6]) == 1):
                    if(args[6] == '0'):
                        fldMask = ['00'] * len(fldVal)
                    else:
                        fldMask = ['ff'] * len(fldVal)
                else:
                    fldMask = args[6].strip().split(':')
            else:
                tmpValue = format(int(args[5]), '0' + str(byteLen*2) + 'X')
                fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                if args[6] == '0':
                    del fldMask[:]
                    for valIdx in range(byteLen):
                        fldMask.append('00')
                elif args[6] == '1':
                    del fldMask[:]
                    for valIdx in range(byteLen):
                        fldMask.append('ff')
                else:
                    tmp = int(args[6])
                    for valIdx in range(byteLen):
                        tmp1 = tmp%256
                        fldMask.append(hex(tmp1).replace('0x',''))
                        tmp = tmp/256
                    fldMask = fldMask[::-1]
                hexFlg = 1
            for valIdx in range(byteLen):
                if(hexFlg):
                    uint8Arr_tp_setitem(fieldVal_Ptr_4, valIdx, int(fldVal[len(fldVal) - valIdx - 1],16))
                    uint8Arr_tp_setitem(fieldMask_Ptr_5, valIdx, int(fldMask[len(fldVal) - valIdx - 1],16))
                else:
                    uint8Arr_tp_setitem(fieldVal_Ptr_4, valIdx, int(fldVal[len(fldVal) - valIdx - 1]))
                    uint8Arr_tp_setitem(fieldMask_Ptr_5, valIdx, int(fldMask[len(fldVal) - valIdx - 1]))
            hexFlg = 0

            #print('Input Arguments are, devId=%d, camIndex=%d, iaclType=%d, fldId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsIaclTypeUpdateKeyField(args[0],args[1],args[2],args[3],args[4],fieldVal_Ptr_4,fieldMask_Ptr_5)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_uint8_tp(fieldMask_Ptr_5)
            delete_uint8_tp(fieldVal_Ptr_4)
    #/********************************************************************************/
    # command for xpsIaclTypeGetKeySize
    #/********************************************************************************/
    def do_iacl_type_get_key_size(self, arg):
        '''
         xpsIaclTypeGetKeySize: Enter [ devId,iaclType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,iaclType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            keySize_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, iaclType=%d' % (args[0],args[1]))
            ret = xpsIaclTypeGetKeySize(args[0],args[1],keySize_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('keySize = %d' % (uint32_tp_value(keySize_Ptr_2)))
                pass
            delete_uint32_tp(keySize_Ptr_2)
    #/********************************************************************************/
    # command for xpsIaclTypeGetKeyNumFlds
    #/********************************************************************************/
    def do_iacl_type_get_key_num_flds(self, arg):
        '''
         xpsIaclTypeGetKeyNumFlds: Enter [ devId,iaclType,keyType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,iaclType,keyType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            numFlds_Ptr_3 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, iaclType=%d, keyType=%d' % (args[0],args[1],args[2]))
            ret = xpsIaclTypeGetKeyNumFlds(args[0],args[1],args[2],numFlds_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numFlds = %d' % (uint32_tp_value(numFlds_Ptr_3)))
                pass
            delete_uint32_tp(numFlds_Ptr_3)
    #/********************************************************************************/
    # command for xpsIaclTypeGetEntryKeyMask
    #/********************************************************************************/
    def do_iacl_type_get_entry_key_mask(self, arg):
        '''
         xpsIaclTypeGetEntryKeyMask: Enter [ devId,camIndex,iaclType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,camIndex,iaclType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            fldData_Ptr_3 = new_xpIaclKeyMask_tp()
            #print('Input Arguments are, devId=%d, camIndex=%d, iaclType=%d' % (args[0],args[1],args[2]))
            ret = xpsIaclTypeGetEntryKeyMask(args[0],args[1],args[2],fldData_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('fldData = %d' % (xpIaclKeyMask_tp_value(fldData_Ptr_3)))
                pass
            delete_xpIaclKeyMask_tp(fldData_Ptr_3)
    #/********************************************************************************/
    # command for xpsIaclTypeGetNumOfValidEntries
    #/********************************************************************************/
    def do_iacl_type_get_num_of_valid_entries(self, arg):
        '''
         xpsIaclTypeGetNumOfValidEntries: Enter [ devId,iaclType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,iaclType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            numOfValidEntries_Ptr_2 = new_uint32_tp()
            #print('Input Arguments are, devId=%d, iaclType=%d' % (args[0],args[1]))
            ret = xpsIaclTypeGetNumOfValidEntries(args[0],args[1],numOfValidEntries_Ptr_2)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                print('numOfValidEntries = %d' % (uint32_tp_value(numOfValidEntries_Ptr_2)))
                pass
            delete_uint32_tp(numOfValidEntries_Ptr_2)
    #/********************************************************************************/
    # command for xpsIaclTypeFreeKeyFieldList
    #/********************************************************************************/
    def do_iacl_type_free_key_field_list(self, arg):
        '''
         xpsIaclTypeFreeKeyFieldList: Enter [ devId,index,iaclType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,iaclType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            fieldList_Ptr_3 = new_xpsIaclkeyFieldList_tp()
            #print('Input Arguments are, devId=%d, index=%d, iaclType=%d' % (args[0],args[1],args[2]))
            ret = xpsIaclTypeFreeKeyFieldList(args[0],args[1],args[2],fieldList_Ptr_3)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsIaclkeyFieldList_tp(fieldList_Ptr_3)
    #/********************************************************************************/
    # command for xpsIaclTypeMallocKeyFieldList
    #/********************************************************************************/
    def do_iacl_type_malloc_key_field_list(self, arg):
        '''
         xpsIaclTypeMallocKeyFieldList: Enter [ devId,index,iaclType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,iaclType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            fieldList_Ptr_3 = new_xpsIaclkeyFieldList_tp()
            fieldList_Ptr_pp = new_xpsIaclkeyFieldList_arr(1)
            xpsIaclkeyFieldList_arr_setitem(fieldList_Ptr_pp, 0,fieldList_Ptr_3)
            #print('Input Arguments are, devId=%d, index=%d, iaclType=%d' % (args[0],args[1],args[2]))
            ret = xpsIaclTypeMallocKeyFieldList(args[0],args[1],args[2],fieldList_Ptr_pp)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsIaclkeyFieldList_tp(fieldList_Ptr_3)
    #/********************************************************************************/
    # command for xpsIaclTypeReadEntry
    #/********************************************************************************/
    def do_iacl_type_read_entry(self, arg):
        '''
         xpsIaclTypeReadEntry: Enter [ devId,index,iaclType,keyType]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,iaclType,keyType ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            fieldList_Ptr_3 = new_xpsIaclkeyFieldList_tp()
            data_Ptr_4 = new_xpsIaclData_tp()
            #print('Input Arguments are, devId=%d, index=%d, iaclType=%d' % (args[0],args[1],args[2]))
            ret = xpsIaclTypeReadEntry(args[0],args[1],args[2],args[3],fieldList_Ptr_3,data_Ptr_4)
            err = 0
            if ret != 0:
                print('Return Value = %d' % (ret))
                err = 1
            if err == 0:
                pass
            delete_xpsIaclData_tp(data_Ptr_4)
            delete_xpsIaclkeyFieldList_tp(fieldList_Ptr_3)
    #/********************************************************************************/
    # command for xpsIaclTypeSetCountingEnable
    #/********************************************************************************/
    def do_iacl_type_set_counting_enable(self, arg):
        '''
         xpsIaclTypeSetCountingEnable: Enter [ devId,client,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, client=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsIaclTypeSetCountingEnable(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsIaclTypeSetPolicingEnable
    #/********************************************************************************/
    def do_iacl_type_set_policing_enable(self, arg):
        '''
         xpsIaclTypeSetPolicingEnable: Enter [ devId,client,enable ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,client,enable ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            #print('Input Arguments are, devId=%d, client=%d, enable=%d' % (args[0],args[1],args[2]))
            ret = xpsIaclTypeSetPolicingEnable(args[0],args[1],args[2])
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

