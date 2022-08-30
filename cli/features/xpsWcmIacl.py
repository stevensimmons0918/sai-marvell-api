#!/usr/bin/env python
#  xpsWcmIacl.py
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
# The class object for xpsWcmIacl operations
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

class xpsWcmIaclObj(Cmd):

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
    # command for xpsWcmIaclInit
    #/********************************************************************************/
    def do_wcm_iacl_init(self, arg):
        '''
         xpsWcmIaclInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsWcmIaclInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('WCM Iacl init done!')
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclInitScope
    #/********************************************************************************/
    def do_wcm_iacl_init_scope(self, arg):
        '''
         xpsWcmIaclInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsWcmIaclInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('WCM Iacl init done, Scope-Id = %d' %(args[0]))
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclDeInit
    #/********************************************************************************/
    def do_wcm_iacl_de_init(self, arg):
        '''
         xpsWcmIaclDeInit: Enter [ void ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 0
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ void ]')
        else:
            #print('Input Arguments are : Not required' % ())
            ret = xpsWcmIaclDeInit()
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('WCM Iacl de-init done!')
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclDeInitScope
    #/********************************************************************************/
    def do_wcm_iacl_de_init_scope(self, arg):
        '''
         xpsWcmIaclDeInitScope: Enter [ scopeId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ scopeId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, scopeId=%d' % (args[0]))
            ret = xpsWcmIaclDeInitScope(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('WCM Iacl de-init done, Scope-Id = %d' %(args[0]))
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclAddDevice
    #/********************************************************************************/
    def do_wcm_iacl_add_device(self, arg):
        '''
         xpsWcmIaclAddDevice: Enter [ devId,initType ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,initType ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            #print('Input Arguments are, devId=%d, initType=%d' % (args[0],args[1]))
            ret = xpsWcmIaclAddDevice(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('WCM Iacl add device done!')
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclRemoveDevice
    #/********************************************************************************/
    def do_wcm_iacl_remove_device(self, arg):
        '''
         xpsWcmIaclRemoveDevice: Enter [ devId ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 1
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId ]')
        else:
            args[0] = int(args[0])
            #print('Input Arguments are, devId=%d' % (args[0]))
            ret = xpsWcmIaclRemoveDevice(args[0])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclCreateTable
    #/********************************************************************************/
    def do_wcm_iacl_create_table(self, arg):
        '''
         xpsWcmIaclCreateTable: Enter [ devId,numTables,wcmTblType,keySize,valueSize,noOfIaclTblTypes,iaclTblType ]
         example : wcm_iacl_create_table 0 2 WCM_TBL_1,WCM_TBL_2 256,256 64,64 2,1 XP_IACL0,XP_IACL1,XP_IACL2
        '''
        args = re.split(';| ',arg)
        numArgsReq = 7
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,numTables,wcmTblType,keySize,valueSize,noOfIaclTblTypes,iaclTblType ]\n <NOTE:- Arguments wcmTblType,keySize,valueSize,noOfIaclTblTypes and iaclTblType expect (, [comma]) seperated values>')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            wcmTblTypeList = args[2].split(',')
            defKeySize = args[3].split(',')
            valueSize = args[4].split(',')
            noOfIaclTblTypesList = args[5].split(',')
            tblTypeList = args[6].split(',')
            if(not(len(wcmTblTypeList) == len(defKeySize) == len(valueSize) == len(noOfIaclTblTypesList))):
                print('Invalid comma separated arguments\n')
                return
            tmp = list(map(int, noOfIaclTblTypesList))
            if(not(sum(tmp) == len(tblTypeList))):
                print('Mismatch in noOfIaclTblTypes and iaclTblType argument\n')
                return
            #args[5] = int(args[5])
            keySize = [] 
            tblTypeIdx = 0
            xpWcmIaclTableProfile_t_Ptr = xpWcmIaclTableProfile_t()
            xpWcmIaclTableProfile_t_Ptr.numTables = args[1]
            arrWcmIaclTableInfo = new_xpWcmIaclTableInfo_arr(args[1])
            for idx in range(args[1]):
                tableProfile = xpWcmIaclTableInfo_t() 
                if(int(defKeySize[idx]) <= 64): 
                    keySize.append(65)
                elif(int(defKeySize[idx]) > 64 and int(defKeySize[idx]) <= 128):
                    keySize.append(130)
                elif(int(defKeySize[idx]) > 128 and int(defKeySize[idx]) <= 192):
                    keySize.append(195)
                else:
                    keySize.append(256)
                tableProfile.wcmTbl = eval(wcmTblTypeList[idx])
                #tableProfile.tblType = eval(tblTypeList[idx])
                tableProfile.keySize = keySize[idx]
                tableProfile.valueSize = int(valueSize[idx])
                tableProfile.numTypes = int(noOfIaclTblTypesList[idx])
                for x in range(tableProfile.numTypes):
                    #tableProfile.tblType[x] = eval(tblTypeList[tblTypeIdx])
                    xpIaclType_arr_setitem(tableProfile.tblType, x , eval(tblTypeList[tblTypeIdx]))
                    tblTypeIdx = tblTypeIdx + 1
                xpWcmIaclTableInfo_arr_setitem(arrWcmIaclTableInfo, idx, tableProfile)
            xpWcmIaclTableProfile_t_Ptr.tableProfile = arrWcmIaclTableInfo
            ret = xpsWcmIaclCreateTable(args[0],xpWcmIaclTableProfile_t_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('WCM Iacl create table done!')
                pass

    #/********************************************************************************/
    # command for xpsWcmIaclDefineKey 
    #/********************************************************************************/
    def do_wcm_iacl_define_key(self, arg):
        '''
         xpsWcmIaclDefineKey: Enter [ devId,keyType,isValid,numFlds,iaclTypes,flds ]
         Note : fields numFlds,iaclTypes and flds expecting comma(,) separated values
         Example: wcm_iacl_define_key 0 XP_IACL_V4_TYPE 1 3,3,3 XP_IACL0,XP_IACL1,XP_IACL2 XP_IACL_KEY_TYPE_V4,XP_IACL_MAC_DA,XP_IACL_MAC_SA XP_IACL_KEY_TYPE_V4,XP_IACL_MAC_DA,XP_IACL_MAC_SA XP_IACL_KEY_TYPE_V4,XP_IACL_MAC_DA,XP_IACL_MAC_SA
        '''
        args = re.split(';| ',arg)
        numArgsReq = 6
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,keyType,isValid,numFlds,iaclTypes,flds ]\nNote : fields numFlds,iaclTypes and flds expecting comma(,) separated values')
            print('Example: wcm_iacl_define_key 0 XP_IACL_V4_TYPE 1 3,3,3 XP_IACL0,XP_IACL1,XP_IACL2 XP_IACL_KEY_TYPE_V4,XP_IACL_MAC_DA,XP_IACL_MAC_SA XP_IACL_KEY_TYPE_V4,XP_IACL_MAC_DA,XP_IACL_MAC_SA XP_IACL_KEY_TYPE_V4,XP_IACL_MAC_DA,XP_IACL_MAC_SA')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            fldData_Ptr = new_xpWcmIaclkeyFieldList_tp()
            memZero(fldData_Ptr, sizeof_xpWcmIaclkeyFieldList)
            fldData_Ptr.isValid = int(args[2])
            #fldData_Ptr.numFlds = args[3].strip().split(',')
            #keyTypeList = args[2].strip().split(',')
            numFldsList = args[3].strip().split(',')
            fldData_Ptr.type = args[1]
            iaclTypeList = args[4].strip().split(',')
            if(len(numFldsList) != len(iaclTypeList)):
                print 'numFlds and iaclTypes values should be in sync'
                delete_xpWcmIaclkeyFieldList_tp(fldData_Ptr)
                return
            indexCnt = 5
            fldDict = {}
            for iaclType in iaclTypeList:
                try:
                    if(eval(iaclType) >= XP_IACL_TOTAL_TYPE):
                        print 'Invalid iaclType'
                        delete_xpWcmIaclkeyFieldList_tp(fldData_Ptr)
                        return
                    fldDict[iaclType] = args[indexCnt].strip().split(',')
                    indexCnt = indexCnt + 1
                except:
                    print 'No fields define for iaclType : %s' %(iaclType)
                    delete_xpWcmIaclkeyFieldList_tp(fldData_Ptr)
                    return
            #fldList = args[5].strip().split(',')
            for iaclType in iaclTypeList:
                if((self.validate_keyfields_sequence(fldDict[iaclType])) != 0):
                    delete_xpWcmIaclkeyFieldList_tp(fldData_Ptr)
                    return
            wcmIaclkeyFieldList_arr = new_xpWcmIaclkeyFieldList_arr(XP_IACL_TOTAL_TYPE)
            memZero(wcmIaclkeyFieldList_arr, sizeof_xpWcmIaclkeyFieldList * XP_IACL_TOTAL_TYPE)
            wcmIaclKeyFormat_Ptr = new_xpWcmIaclKeyFormat_tp()
            memZero(wcmIaclKeyFormat_Ptr, sizeof_xpsWcmIaclKeyFormat)
            for iaclType in iaclTypeList:
                fldData_Ptr.numFlds = int(numFldsList[eval(iaclType)])
                #fldData_Ptr.type = eval(keyTypeList[eval(iaclType)])
                xpIaclkeyField_t_ptr = new_xpWcmIaclkeyField_tp()
                memZero(xpIaclkeyField_t_ptr, sizeof_xpWcmIaclkeyField)
                xpIaclkeyField_t_arr = new_xpWcmIaclkeyField_arr(len(fldDict[iaclType]))
                memZero(xpIaclkeyField_t_arr, sizeof_xpWcmIaclkeyField * len(fldDict[iaclType]))
                for fldIdx,fldEnm in enumerate(fldDict[iaclType]):
                    if(fldData_Ptr.type == 0):
                        xpIaclkeyField_t_ptr.fld.v4Fld = eval(fldEnm)
                        byteSize =mapxpIaclV4KeyByteMask[eval(fldEnm)]
                        if(xpIaclkeyField_t_ptr.fld.v4Fld == XP_IACL_L4_SRC_PORT):
                            xpIaclkeyField_t_ptr.transType = XP_RANGE_A_TRANS
                        elif (xpIaclkeyField_t_ptr.fld.v4Fld == XP_IACL_L4_DEST_PORT):
                            xpIaclkeyField_t_ptr.transType = XP_RANGE_B_TRANS
                        elif (xpIaclkeyField_t_ptr.fld.v4Fld == XP_IACL_SIP_V4):
                            xpIaclkeyField_t_ptr.transType = XP_PREFIX_A_TRANS
                        elif (xpIaclkeyField_t_ptr.fld.v4Fld == XP_IACL_DIP_V4):
                            xpIaclkeyField_t_ptr.transType = XP_PREFIX_B_TRANS
                        else:
                            xpIaclkeyField_t_ptr.transType = XP_TRANS_NONE
                    else:
                        xpIaclkeyField_t_ptr.fld.v6Fld = eval(fldEnm)
                        byteSize =mapxpIaclV6KeyByteMask[eval(fldEnm)]
                        if(xpIaclkeyField_t_ptr.fld.v6Fld == XP_IACL_L4_V6_SRC_PORT):
                            xpIaclkeyField_t_ptr.transType = XP_RANGE_A_TRANS
                        elif (xpIaclkeyField_t_ptr.fld.v6Fld ==  XP_IACL_L4_V6_DEST_PORT):
                            xpIaclkeyField_t_ptr.transType = XP_RANGE_B_TRANS
                        elif (xpIaclkeyField_t_ptr.fld.v6Fld == XP_IACL_SIP_V6):
                            xpIaclkeyField_t_ptr.transType = XP_PREFIX_A_TRANS
                        elif (xpIaclkeyField_t_ptr.fld.v6Fld == XP_IACL_DIP_V6):
                            xpIaclkeyField_t_ptr.transType = XP_PREFIX_B_TRANS
                        else:
                            xpIaclkeyField_t_ptr.transType = XP_TRANS_NONE
                    byteLen = bin(byteSize).count('1')
                    valPtr = new_uint8Arr_tp(byteLen)
                    maskPtr = new_uint8Arr_tp(byteLen)
                    xpIaclkeyField_t_ptr.value = valPtr
                    xpIaclkeyField_t_ptr.mask = maskPtr

                    xpWcmIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr)
                fldData_Ptr.fldList = xpIaclkeyField_t_arr
                xpWcmIaclkeyFieldList_arr_setitem(wcmIaclKeyFormat_Ptr.iaclFldList,eval(iaclType),fldData_Ptr)
            ret = xpsWcmIaclDefineKey(args[0],args[1],wcmIaclKeyFormat_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('WCM Iacl define key done!')
            del valPtr 
            del maskPtr
            delete_xpWcmIaclKeyFormat_tp(wcmIaclKeyFormat_Ptr)
            delete_xpWcmIaclkeyFieldList_tp(fldData_Ptr)
        
    #/********************************************************************************/
    # command for xpsWcmIaclAddPaclEntry
    #/********************************************************************************/
    def do_wcm_iacl_add_pacl_entry(self, arg):
        '''
         xpsWcmIaclAddPaclEntry: Enter [ devId,priority,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan, {flds,value,mask,wcmInstrType,rangeInUpperBound}, {flds,value,mask,wcmInstrType,rangeInUpperBound} ...upto number of keys wants to be programe ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 27
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,priority,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan, {flds,value,mask,wcmInstrType,rangeInUpperBound}, {flds,value,mask,wcmInstrType,rangeInUpperBound} ...upto number of keys wants to be programe \
                    e.g.: \n \
                    wcm_iacl_add_pacl_entry 0 0 3 1 XP_IACL_V4_TYPE 0 1 0 1 0 1 0 0 0 0 0 0 0 0 0 0 19 0 0 0 852 XP_IACL_KEY_TYPE_V4,0,0,XP_WCM_INSTR_MATCH,100 XP_IACL_MAC_DA,0:0:0:0:0:0,0,XP_WCM_INSTR_MATCH,100 XP_IACL_MAC_SA,0:0:0:0:0:0,0,XP_WCM_INSTR_MATCH,100]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            keyData_Ptr = new_xpWcmIaclkeyFieldList_tp()
            keyData_Ptr.numFlds = int(args[2])
            keyData_Ptr.isValid = int(args[3])
            keyData_Ptr.type = eval(args[4])
            fldList = [] 
            valueList = [] 
            maskList = [] 
            wcmInstrTypeList = [] 
            rangeInUpperBoundList = [] 
            fldvalMask = '' 
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 5:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
                    wcmInstrTypeList.append(tmpFldList[3])
                    rangeInUpperBoundList.append(tmpFldList[4])

            if len(fldList) == int(args[2]) and len(valueList) == int(args[2]) and len(maskList) == int(args[2]) and len(wcmInstrTypeList) == int(args[2]) and len(rangeInUpperBoundList) == int(args[2]):
                xpIaclkeyField_t_ptr = new_xpWcmIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpWcmIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                        if(eval(args[4]) == 0):
                            xpIaclkeyField_t_ptr.fld.v4Fld = eval(defFld)
                            byteSize = mapxpIaclV4KeyByteMask[eval(fldList[fldIdx])]
                            if(eval(defFld) == XP_IACL_L4_SRC_PORT):
                                xpIaclkeyField_t_ptr.transType = XP_RANGE_A_TRANS
                            elif (eval(defFld) == XP_IACL_L4_DEST_PORT):
                                xpIaclkeyField_t_ptr.transType = XP_RANGE_B_TRANS
                            elif (eval(defFld) == XP_IACL_SIP_V4):
                                xpIaclkeyField_t_ptr.transType = XP_PREFIX_A_TRANS
                            elif (eval(defFld) == XP_IACL_DIP_V4):
                                xpIaclkeyField_t_ptr.transType = XP_PREFIX_B_TRANS
                            else:
                                xpIaclkeyField_t_ptr.transType = XP_TRANS_NONE
                        else:
                            xpIaclkeyField_t_ptr.fld.v6Fld = eval(defFld)
                            byteSize = mapxpIaclV6KeyByteMask[eval(fldList[fldIdx])]
                            if(eval(defFld) == XP_IACL_L4_V6_SRC_PORT):
                                xpIaclkeyField_t_ptr.transType = XP_RANGE_A_TRANS
                            elif (eval(defFld) ==  XP_IACL_L4_V6_DEST_PORT):
                                xpIaclkeyField_t_ptr.transType = XP_RANGE_B_TRANS
                            elif (eval(defFld) == XP_IACL_SIP_V6):
                                xpIaclkeyField_t_ptr.transType = XP_PREFIX_A_TRANS
                            elif (eval(defFld) == XP_IACL_DIP_V6):
                                xpIaclkeyField_t_ptr.transType = XP_PREFIX_B_TRANS
                            else:
                                xpIaclkeyField_t_ptr.transType = XP_TRANS_NONE
                        byteLen = bin(byteSize).count('1')
                        valPtr = new_uint8Arr_tp(byteLen)
                        maskPtr = new_uint8Arr_tp(byteLen)
                        xpIaclkeyField_t_ptr.instr.type = eval(wcmInstrTypeList[fldIdx])
                        xpIaclkeyField_t_ptr.instr.rangeInstrUpperBound = int(rangeInUpperBoundList[fldIdx])
                        if(not defFld in fldList):
                            for valIdx in range(byteLen):
                                uint8Arr_tp_setitem(valPtr, valIdx, 0)
                                uint8Arr_tp_setitem(maskPtr, valIdx,0xff)
                        else:
                            hexFlag = 0
                            fldMask = []
                            fldVal = []
                            if('.' in  valueList[fldIdx]):
                                fldVal = valueList[fldIdx].split('.')
                                if(len(maskList[fldIdx]) == 1):
                                    if(maskList[fldIdx] == '0'):
                                        fldMask = ['00'] * len(fldVal)
                                    else:
                                        fldMask = ['255'] * len(fldVal)
                                else:
                                    fldMask = maskList[fldIdx].strip().split('.')
                            elif(':' in  valueList[fldIdx]):
                                fldVal = valueList[fldIdx].split(':')
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
                                #print '>>', maskList
                                if(maskList[fldIdx] == '0'):
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
                            #print '>>>>>',fldVal, fldMask
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
                        xpWcmIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr)
                keyData_Ptr.fldList = xpIaclkeyField_t_arr
                ctrlData_Ptr = new_xpIaclData_tp()
                ctrlData_Ptr.iAclDataType.aclData.isTerminal = int(args[5])
                ctrlData_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[6])
                ctrlData_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[7])
                ctrlData_Ptr.iAclDataType.aclData.enDpRemark = int(args[8])
                ctrlData_Ptr.iAclDataType.aclData.enPolicer = int(args[9])
                ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[10])
                ctrlData_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[11])
                ctrlData_Ptr.iAclDataType.aclData.enTcRemark = int(args[12])
                ctrlData_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[13])
                ctrlData_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[14])
                ctrlData_Ptr.iAclDataType.aclData.tc = int(args[16])
                ctrlData_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[17])
                ctrlData_Ptr.iAclDataType.aclData.encapType = int(args[18])
                ctrlData_Ptr.iAclDataType.aclData.policerId = int(args[20])
                ctrlData_Ptr.iAclDataType.aclData.dp = int(args[22])
                if(ctrlData_Ptr.iAclDataType.aclData.enRedirectToEvif == 1):
                    ctrlData_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.eVifId = int(args[19])
                    ctrlData_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.ecmpSize = int(args[15])
                else:
                    ctrlData_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode = int(args[21])
                    ctrlData_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command = int(args[15])
                if (ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRw == 0):
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[23])
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[24])
                else:
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[25])
                indexList_Ptr = new_xpsHashIndexList_tp()
                #print('Input Arguments are, devId=%d, priority=%d' % (args[0],args[1]))
                ret = xpsWcmIaclAddPaclEntry(args[0],args[1],keyData_Ptr,ctrlData_Ptr,indexList_Ptr)
                err = 0
                if ret != 0:
                    print('Return Value = %d' % (ret))
                    err = 1
                if err == 0:
                    print('Size of Index List = %d' % (indexList_Ptr.size))
                    print('HashIndexList = '),
                    for i in range(0, indexList_Ptr.size):
                        print indexList_Ptr.index[i],',',
                    print('')
                    pass
                delete_xpsHashIndexList_tp(indexList_Ptr)
                delete_xpIaclData_tp(ctrlData_Ptr)
                delete_xpWcmIaclkeyFieldList_tp(keyData_Ptr)
            else:
                if fldvalMask ==  '-1':
                    print "Invalid input args "
                else:
                    print "numFlds and given number of flds not matched"
                    print "numFlds = %s"% args[2]
                    print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsWcmIaclRemovePaclEntry
    #/********************************************************************************/
    def do_wcm_iacl_remove_pacl_entry(self, arg):
        '''
         xpsWcmIaclRemovePaclEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsWcmIaclRemovePaclEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsWcmIaclAddBaclEntry
    #/********************************************************************************/
    def do_wcm_iacl_add_bacl_entry(self, arg):
        '''
         xpsWcmIaclAddBaclEntry: Enter [ devId,priority,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan, {flds,value,mask,wcmInstrType,rangeInUpperBound}, {flds,value,mask,wcmInstrType,rangeInUpperBound} ...upto number of keys wants to be programe ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 27
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,priority,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan, {flds,value,mask,wcmInstrType,rangeInUpperBound}, {flds,value,mask,wcmInstrType,rangeInUpperBound} ...upto number of keys wants to be programe \
                    e.g.: \n \
                    wcm_iacl_add_bacl_entry 0 0 3 1 XP_IACL_V4_TYPE 0 1 0 1 0 1 0 0 0 0 0 0 0 0 0 0 19 0 0 0 852 XP_IACL_KEY_TYPE_V4,0,0,XP_WCM_INSTR_MATCH,100 XP_IACL_MAC_DA,0:0:0:0:0:0,0,XP_WCM_INSTR_MATCH,100 XP_IACL_MAC_SA,0:0:0:0:0:0,0,XP_WCM_INSTR_MATCH,100')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            keyData_Ptr = new_xpWcmIaclkeyFieldList_tp()
            keyData_Ptr.numFlds = int(args[2])
            keyData_Ptr.isValid = int(args[3])
            keyData_Ptr.type = eval(args[4])
            fldList = [] 
            valueList = [] 
            maskList = [] 
            wcmInstrTypeList = [] 
            rangeInUpperBoundList = [] 
            fldvalMask = '' 
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 5:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
                    wcmInstrTypeList.append(tmpFldList[3])
                    rangeInUpperBoundList.append(tmpFldList[4])

            if len(fldList) == int(args[2]) and len(valueList) == int(args[2]) and len(maskList) == int(args[2]) and len(wcmInstrTypeList) == int(args[2]) and len(rangeInUpperBoundList) == int(args[2]):
                xpIaclkeyField_t_ptr = new_xpWcmIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpWcmIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                    if(eval(args[4]) == 0):
                        xpIaclkeyField_t_ptr.fld.v4Fld = eval(defFld)
                        byteSize = mapxpIaclV4KeyByteMask[eval(fldList[fldIdx])]
                        if(eval(defFld) == XP_IACL_L4_SRC_PORT):
                            xpIaclkeyField_t_ptr.transType = XP_RANGE_A_TRANS
                        elif (eval(defFld) == XP_IACL_L4_DEST_PORT):
                            xpIaclkeyField_t_ptr.transType = XP_RANGE_B_TRANS
                        elif (eval(defFld) == XP_IACL_SIP_V4):
                            xpIaclkeyField_t_ptr.transType = XP_PREFIX_A_TRANS
                        elif (eval(defFld) == XP_IACL_DIP_V4):
                            xpIaclkeyField_t_ptr.transType = XP_PREFIX_B_TRANS
                        else:
                            xpIaclkeyField_t_ptr.transType = XP_TRANS_NONE
                    else:
                        xpIaclkeyField_t_ptr.fld.v6Fld = eval(defFld)
                        byteSize = mapxpIaclV6KeyByteMask[eval(fldList[fldIdx])]
                        if(eval(defFld) == XP_IACL_L4_V6_SRC_PORT):
                            xpIaclkeyField_t_ptr.transType = XP_RANGE_A_TRANS
                        elif (eval(defFld) ==  XP_IACL_L4_V6_DEST_PORT):
                            xpIaclkeyField_t_ptr.transType = XP_RANGE_B_TRANS
                        elif (eval(defFld) == XP_IACL_SIP_V6):
                            xpIaclkeyField_t_ptr.transType = XP_PREFIX_A_TRANS
                        elif (eval(defFld) == XP_IACL_DIP_V6):
                            xpIaclkeyField_t_ptr.transType = XP_PREFIX_B_TRANS
                        else:
                            xpIaclkeyField_t_ptr.transType = XP_TRANS_NONE
                    byteLen = bin(byteSize).count('1')
                    valPtr = new_uint8Arr_tp(byteLen)
                    maskPtr = new_uint8Arr_tp(byteLen)
                    xpIaclkeyField_t_ptr.instr.type = eval(wcmInstrTypeList[fldIdx])
                    xpIaclkeyField_t_ptr.instr.rangeInstrUpperBound = int(rangeInUpperBoundList[fldIdx])
                    if(not defFld in fldList):
                        for valIdx in range(byteLen):
                            uint8Arr_tp_setitem(valPtr, valIdx, 0)
                            uint8Arr_tp_setitem(maskPtr, valIdx,0xff)
                    else:
                        hexFlag = 0
                        fldMask = []
                        fldVal = []
                        if('.' in  valueList[fldIdx]):
                            fldVal = valueList[fldIdx].split('.')
                            if(len(maskList[fldIdx]) == 1):
                                if(maskList[fldIdx] == '0'):
                                    fldMask = ['00'] * len(fldVal)
                                else:
                                    fldMask = ['255'] * len(fldVal)
                            else:
                                fldMask = maskList[fldIdx].strip().split('.')
                        elif(':' in  valueList[fldIdx]):
                            fldVal = valueList[fldIdx].split(':')
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
                            #print '>>', maskList
                            if(maskList[fldIdx] == '0'):
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
                    xpWcmIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr)
                keyData_Ptr.fldList = xpIaclkeyField_t_arr
                ctrlData_Ptr = new_xpIaclData_tp()
                ctrlData_Ptr.iAclDataType.aclData.isTerminal = int(args[5])
                ctrlData_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[6])
                ctrlData_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[7])
                ctrlData_Ptr.iAclDataType.aclData.enDpRemark = int(args[8])
                ctrlData_Ptr.iAclDataType.aclData.enPolicer = int(args[9])
                ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[10])
                ctrlData_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[11])
                ctrlData_Ptr.iAclDataType.aclData.enTcRemark = int(args[12])
                ctrlData_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[13])
                ctrlData_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[14])
                ctrlData_Ptr.iAclDataType.aclData.tc = int(args[16])
                ctrlData_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[17])
                ctrlData_Ptr.iAclDataType.aclData.encapType = int(args[18])
                ctrlData_Ptr.iAclDataType.aclData.policerId = int(args[20])
                if(ctrlData_Ptr.iAclDataType.aclData.enRedirectToEvif == 1):
                    ctrlData_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.egrVifId = int(args[19])
                    ctrlData_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.ecmpSize = int(args[15])
                else:
                    ctrlData_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode = int(args[21])
                    ctrlData_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command = int(args[15])
                ctrlData_Ptr.iAclDataType.aclData.dp = int(args[22])
                if (ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRw == 0):
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[23])
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[24])
                else:
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[25])

                indexList_Ptr = new_xpsHashIndexList_tp()
                #print('Input Arguments are, devId=%d, priority=%d' % (args[0],args[1]))
                ret = xpsWcmIaclAddBaclEntry(args[0],args[1],keyData_Ptr,ctrlData_Ptr,indexList_Ptr)
                err = 0
                if ret != 0:
                    print('Return Value = %d' % (ret))
                    err = 1
                if err == 0:
                    print('Size of Index List = %d' % (indexList_Ptr.size))
                    print('HashIndexList = '),
                    for i in range(0, indexList_Ptr.size):
                        print indexList_Ptr.index[i],',',
                    print('')
                    pass
                delete_xpsHashIndexList_tp(indexList_Ptr)
                delete_xpIaclData_tp(ctrlData_Ptr)
                delete_xpWcmIaclkeyFieldList_tp(keyData_Ptr)
            else:
                if fldvalMask ==  '-1':
                    print "Invalid input args "
                else:
                    print "numFlds and given number of flds not matched"
                    print "numFlds = %s"% args[2]
                    print "flds = %s"% fldList
    #/********************************************************************************/
    # command for xpsWcmIaclRemoveBaclEntry
    #/********************************************************************************/
    def do_wcm_iacl_remove_bacl_entry(self, arg):
        '''
         xpsWcmIaclRemoveBaclEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsWcmIaclRemoveBaclEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass

    #/********************************************************************************/
    # command for xpsWcmIaclAddRaclEntry
    #/********************************************************************************/
    def do_wcm_iacl_add_racl_entry(self, arg):
        '''
         xpsWcmIaclAddRaclEntry: Enter [ devId,priority,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan, {flds,value,mask,wcmInstrType,rangeInUpperBound}, {flds,value,mask,wcmInstrType,rangeInUpperBound} ...upto number of keys wants to be programe ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 27
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,priority,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan, {flds,value,mask,wcmInstrType,rangeInUpperBound}, {flds,value,mask,wcmInstrType,rangeInUpperBound} ...upto number of keys wants to be programe \
                    e.g.: \n \
                    wcm_iacl_add_racl_entry 0 0 3 1 XP_IACL_V4_TYPE 0 1 0 1 0 1 0 0 0 0 0 0 0 0 0 0 19 0 0 0 852 XP_IACL_KEY_TYPE_V4,0,0,XP_WCM_INSTR_MATCH,100 XP_IACL_MAC_DA,0:0:0:0:0:0,0,XP_WCM_INSTR_MATCH,100 XP_IACL_MAC_SA,0:0:0:0:0:0,0,XP_WCM_INSTR_MATCH,100]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            keyData_Ptr = new_xpWcmIaclkeyFieldList_tp()
            keyData_Ptr.numFlds = int(args[2])
            keyData_Ptr.isValid = int(args[3])
            keyData_Ptr.type = eval(args[4])
            fldList = [] 
            valueList = [] 
            maskList = [] 
            wcmInstrTypeList = [] 
            rangeInUpperBoundList = [] 
            fldvalMask = '' 
            for idx in range(numArgsReq - 1, len(args)):
                tmpFldList = args[idx].split(',')
                if not len(tmpFldList) == 5:
                    print "Given input args Incorrect for %s fld,value,mask "% args[idx]
                    fldvalMask = '-1' 
                else:
                    fldList.append(tmpFldList[0])
                    valueList.append(tmpFldList[1])
                    maskList.append(tmpFldList[2])
                    wcmInstrTypeList.append(tmpFldList[3])
                    rangeInUpperBoundList.append(tmpFldList[4])

            if len(fldList) == int(args[2]) and len(valueList) == int(args[2]) and len(maskList) == int(args[2]) and len(wcmInstrTypeList) == int(args[2]) and len(rangeInUpperBoundList) == int(args[2]):
                xpIaclkeyField_t_ptr = new_xpWcmIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpWcmIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                    if(eval(args[4]) == 0):
                        xpIaclkeyField_t_ptr.fld.v4Fld = eval(defFld)
                        byteSize = mapxpIaclV4KeyByteMask[eval(fldList[fldIdx])]
                        if(eval(defFld) == XP_IACL_L4_SRC_PORT):
                            xpIaclkeyField_t_ptr.transType = XP_RANGE_A_TRANS
                        elif (eval(defFld) == XP_IACL_L4_DEST_PORT):
                            xpIaclkeyField_t_ptr.transType = XP_RANGE_B_TRANS
                        elif (eval(defFld) == XP_IACL_SIP_V4):
                            xpIaclkeyField_t_ptr.transType = XP_PREFIX_A_TRANS
                        elif (eval(defFld) == XP_IACL_DIP_V4):
                            xpIaclkeyField_t_ptr.transType = XP_PREFIX_B_TRANS
                        else:
                            xpIaclkeyField_t_ptr.transType = XP_TRANS_NONE
                    else:
                        xpIaclkeyField_t_ptr.fld.v6Fld = eval(defFld)
                        byteSize = mapxpIaclV6KeyByteMask[eval(fldList[fldIdx])]
                        if(eval(defFld) == XP_IACL_L4_V6_SRC_PORT):
                            xpIaclkeyField_t_ptr.transType = XP_RANGE_A_TRANS
                        elif (eval(defFld) ==  XP_IACL_L4_V6_DEST_PORT):
                            xpIaclkeyField_t_ptr.transType = XP_RANGE_B_TRANS
                        elif (eval(defFld) == XP_IACL_SIP_V6):
                            xpIaclkeyField_t_ptr.transType = XP_PREFIX_A_TRANS
                        elif (eval(defFld) == XP_IACL_DIP_V6):
                            xpIaclkeyField_t_ptr.transType = XP_PREFIX_B_TRANS
                        else:
                            xpIaclkeyField_t_ptr.transType = XP_TRANS_NONE
                    byteLen = bin(byteSize).count('1')
                    valPtr = new_uint8Arr_tp(byteLen)
                    maskPtr = new_uint8Arr_tp(byteLen)
                    xpIaclkeyField_t_ptr.instr.type = eval(wcmInstrTypeList[fldIdx])
                    xpIaclkeyField_t_ptr.instr.rangeInstrUpperBound = int(rangeInUpperBoundList[fldIdx])
                    if(not defFld in fldList):
                        for valIdx in range(byteLen):
                            uint8Arr_tp_setitem(valPtr, valIdx, 0)
                            uint8Arr_tp_setitem(maskPtr, valIdx,0xff)
                    else:
                        hexFlag = 0
                        fldMask = []
                        fldVal = []
                        if('.' in  valueList[fldIdx]):
                            fldVal = valueList[fldIdx].split('.')
                            if(len(maskList[fldIdx]) == 1):
                                if(maskList[fldIdx] == '0'):
                                    fldMask = ['00'] * len(fldVal)
                                else:
                                    fldMask = ['255'] * len(fldVal)
                            else:
                                fldMask = maskList[fldIdx].strip().split('.')
                        elif(':' in  valueList[fldIdx]):
                            fldVal = valueList[fldIdx].split(':')
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
                            #print '>>', maskList
                            if(maskList[fldIdx] == '0'):
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
                    xpWcmIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr)
                keyData_Ptr.fldList = xpIaclkeyField_t_arr
                ctrlData_Ptr = new_xpIaclData_tp()
                ctrlData_Ptr.iAclDataType.aclData.isTerminal = int(args[5])
                ctrlData_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[6])
                ctrlData_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[7])
                ctrlData_Ptr.iAclDataType.aclData.enDpRemark = int(args[8])
                ctrlData_Ptr.iAclDataType.aclData.enPolicer = int(args[9])
                ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[10])
                ctrlData_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[11])
                ctrlData_Ptr.iAclDataType.aclData.enTcRemark = int(args[12])
                ctrlData_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[13])
                ctrlData_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[14])
                ctrlData_Ptr.iAclDataType.aclData.tc = int(args[16])
                ctrlData_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[17])
                ctrlData_Ptr.iAclDataType.aclData.encapType = int(args[18])
                ctrlData_Ptr.iAclDataType.aclData.policerId = int(args[20])
                if(ctrlData_Ptr.iAclDataType.aclData.enRedirectToEvif == 1):
                    ctrlData_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.egrVifId = int(args[19])
                    ctrlData_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.ecmpSize = int(args[15])
                else:
                    ctrlData_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode = int(args[21])
                    ctrlData_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command = int(args[15])
                ctrlData_Ptr.iAclDataType.aclData.dp = int(args[22])
                if (ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRw == 0):
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[23])
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[24])
                else:
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[25])

                indexList_Ptr = new_xpsHashIndexList_tp()
                #print('Input Arguments are, devId=%d, priority=%d' % (args[0],args[1]))
                ret = xpsWcmIaclAddRaclEntry(args[0],args[1],keyData_Ptr,ctrlData_Ptr,indexList_Ptr)
                err = 0
                if ret != 0:
                    print('Return Value = %d' % (ret))
                    err = 1
                if err == 0:
                    print('Size of Index List = %d' % (indexList_Ptr.size))
                    print('HashIndexList = '),
                    for i in range(0, indexList_Ptr.size):
                        print indexList_Ptr.index[i],',',
                    print('')
                    pass
                delete_xpsHashIndexList_tp(indexList_Ptr)
                delete_xpIaclData_tp(ctrlData_Ptr)
                delete_xpWcmIaclkeyFieldList_tp(keyData_Ptr)
            else:
                if fldvalMask ==  '-1':
                    print "Invalid input args "
                else:
                    print "numFlds and given number of flds not matched"
                    print "numFlds = %s"% args[2]
                    print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsWcmIaclRemoveRaclEntry
    #/********************************************************************************/
    def do_wcm_iacl_remove_racl_entry(self, arg):
        '''
         xpsWcmIaclRemoveRaclEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsWcmIaclRemoveRaclEntry(args[0],args[1])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass


    #/********************************************************************************/
    # command for xpsWcmIaclWritePaclOverflowEntry
    #/********************************************************************************/
    def do_wcm_iacl_write_pacl_overflow_entry(self, arg):
        '''
         xpsWcmIaclWritePaclOverflowEntry: Enter [ devId,overflowIndex,priority,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan, {flds,value,mask}, {flds,value,mask} ...upto number of keys wants to be programe ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 28
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,overflowIndex,priority,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan, {flds,value,mask}, {flds,value,mask} ...upto number of keys wants to be programe \
                    e.g.: \n \
                    wcm_iacl_write_pacl_overflow_entry 0 52 0 3 1 XP_IACL_V4_TYPE 0 1 0 1 0 1 0 0 0 0 0 0 0 0 0 0 19 0 0 0 852 XP_IACL_KEY_TYPE_V4,0,0 XP_IACL_MAC_DA,0:0:0:0:0:0,0 XP_IACL_MAC_SA,0:0:0:0:0:0,0]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            keyData_Ptr = new_xpWcmIaclkeyFieldList_tp()
            keyData_Ptr.numFlds = int(args[3])
            keyData_Ptr.isValid = int(args[4])
            keyData_Ptr.type = eval(args[5])
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
                xpIaclkeyField_t_ptr = new_xpWcmIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpWcmIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                       if(eval(args[5]) == 0):
                           xpIaclkeyField_t_ptr.fld.v4Fld = eval(defFld)
                           byteSize = mapxpIaclV4KeyByteMask[eval(fldList[fldIdx])]
                       else:
                           xpIaclkeyField_t_ptr.fld.v6Fld = eval(defFld)
                           byteSize = mapxpIaclV6KeyByteMask[eval(fldList[fldIdx])]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       if(not defFld in fldList):
                           for valIdx in range(byteLen):
                               uint8Arr_tp_setitem(valPtr, valIdx, 0)
                               uint8Arr_tp_setitem(maskPtr, valIdx,0xff)
                       else:
                           hexFlag = 0
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].split('.')
                               if(maskList[fldIdx] == '0'):
                                   fldMask = ['00'] * len(fldVal)
                               else:
                                   fldMask = ['255'] * len(fldVal)
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].split(':')
                               if(maskList[fldIdx] == '0'):
                                   fldMask = ['00'] * len(fldVal)
                               else:
                                   fldMask = ['ff'] * len(fldVal)
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               tmpMask = str('1' * ((byteLen*8)- len(bin(int(valueList[fldIdx])).lstrip('0b'))) + str(maskList[fldIdx] * len(bin(int(valueList[fldIdx])).lstrip('0b'))))
                               tmpMask = hex(int(tmpMask, 2))
                               tmpMask = format(int(tmpMask,16), '0' + str(byteLen*2) + 'x')
                               fldMask = [tmpMask[i:i+2] for i in range(0, len(tmpMask), 2)]
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
                       xpWcmIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr)
                keyData_Ptr.fldList = xpIaclkeyField_t_arr
                ctrlData_Ptr = new_xpIaclData_tp()
                ctrlData_Ptr.iAclDataType.aclData.isTerminal = int(args[6])
                ctrlData_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[7])
                ctrlData_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[8])
                ctrlData_Ptr.iAclDataType.aclData.enDpRemark = int(args[9])
                ctrlData_Ptr.iAclDataType.aclData.enPolicer = int(args[10])
                ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[11])
                ctrlData_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[12])
                ctrlData_Ptr.iAclDataType.aclData.enTcRemark = int(args[13])
                ctrlData_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[14])
                ctrlData_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[15])
                ctrlData_Ptr.iAclDataType.aclData.tc = int(args[17])
                ctrlData_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[18])
                ctrlData_Ptr.iAclDataType.aclData.encapType = int(args[19])
                ctrlData_Ptr.iAclDataType.aclData.policerId = int(args[21])
                if(ctrlData_Ptr.iAclDataType.aclData.enRedirectToEvif == 1):
                    ctrlData_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.egrVifId = int(args[20])
                    ctrlData_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.ecmpSize = int(args[16])
                else:
                    ctrlData_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode = int(args[22])
                    ctrlData_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command = int(args[16])
                ctrlData_Ptr.iAclDataType.aclData.dp = int(args[23])
                if (ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRw == 0):
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[24])
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[25])
                else:
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[26])

                indexList_Ptr = new_xpsHashIndexList_tp()
                #print('Input Arguments are, devId=%d, priority=%d' % (args[0],args[1]))
                ret = xpsWcmIaclWritePaclOverflowEntry(args[0],args[1],args[2],keyData_Ptr,ctrlData_Ptr)
                if ret != 0:
                    print('Failed to set wcm Iacl rule for Pacl, index - %d' %(args[1]))
                    print('Return Value = %d' % (ret))
                else:
                    print('Wcm Iacl rule for Pacl set successfully, index - %d' %(args[1]))
                    pass
                delete_xpIaclData_tp(ctrlData_Ptr)
                delete_xpWcmIaclkeyFieldList_tp(keyData_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args "
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsWcmIaclWriteBaclOverflowEntry
    #/********************************************************************************/
    def do_wcm_iacl_write_bacl_overflow_entry(self, arg):
        '''
         xpsWcmIaclWriteBaclOverflowEntry: Enter [ devId,overflowIndex,priority,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan, {flds,value,mask}, {flds,value,mask} ...upto number of keys wants to be programe ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 28
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,overflowIndex,priority,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan, {flds,value,mask}, {flds,value,mask} ...upto number of keys wants to be programe \
                    e.g.: \n \
                    wcm_iacl_write_pacl_overflow_entry 0 52 0 3 1 XP_IACL_V4_TYPE 0 1 0 1 0 1 0 0 0 0 0 0 0 0 0 0 19 0 0 0 852 XP_IACL_KEY_TYPE_V4,0,0 XP_IACL_MAC_DA,0:0:0:0:0:0,0 XP_IACL_MAC_SA,0:0:0:0:0:0,0]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            keyData_Ptr = new_xpWcmIaclkeyFieldList_tp()
            keyData_Ptr.numFlds = int(args[3])
            keyData_Ptr.isValid = int(args[4])
            keyData_Ptr.type = eval(args[5])
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
                xpIaclkeyField_t_ptr = new_xpWcmIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpWcmIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                       if(eval(args[5]) == 0):
                           xpIaclkeyField_t_ptr.fld.v4Fld = eval(defFld)
                           byteSize = mapxpIaclV4KeyByteMask[eval(fldList[fldIdx])]
                       else:
                           xpIaclkeyField_t_ptr.fld.v6Fld = eval(defFld)
                           byteSize = mapxpIaclV6KeyByteMask[eval(fldList[fldIdx])]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       if(not defFld in fldList):
                           for valIdx in range(byteLen):
                               uint8Arr_tp_setitem(valPtr, valIdx, 0)
                               uint8Arr_tp_setitem(maskPtr, valIdx,0xff)
                       else:
                           hexFlag = 0
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].split('.')
                               if(maskList[fldIdx] == '0'):
                                   fldMask = ['00'] * len(fldVal)
                               else:
                                   fldMask = ['255'] * len(fldVal)
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].split(':')
                               if(maskList[fldIdx] == '0'):
                                   fldMask = ['00'] * len(fldVal)
                               else:
                                   fldMask = ['ff'] * len(fldVal)
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               tmpMask = str('1' * ((byteLen*8)- len(bin(int(valueList[fldIdx])).lstrip('0b'))) + str(maskList[fldIdx] * len(bin(int(valueList[fldIdx])).lstrip('0b'))))
                               tmpMask = hex(int(tmpMask, 2))
                               tmpMask = format(int(tmpMask,16), '0' + str(byteLen*2) + 'x')
                               fldMask = [tmpMask[i:i+2] for i in range(0, len(tmpMask), 2)]
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
                       xpWcmIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr)
                keyData_Ptr.fldList = xpIaclkeyField_t_arr
                ctrlData_Ptr = new_xpIaclData_tp()
                ctrlData_Ptr.iAclDataType.aclData.isTerminal = int(args[6])
                ctrlData_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[7])
                ctrlData_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[8])
                ctrlData_Ptr.iAclDataType.aclData.enDpRemark = int(args[9])
                ctrlData_Ptr.iAclDataType.aclData.enPolicer = int(args[10])
                ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[11])
                ctrlData_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[12])
                ctrlData_Ptr.iAclDataType.aclData.enTcRemark = int(args[13])
                ctrlData_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[14])
                ctrlData_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[15])
                ctrlData_Ptr.iAclDataType.aclData.tc = int(args[17])
                ctrlData_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[18])
                ctrlData_Ptr.iAclDataType.aclData.encapType = int(args[19])
                ctrlData_Ptr.iAclDataType.aclData.policerId = int(args[21])
                if(ctrlData_Ptr.iAclDataType.aclData.enRedirectToEvif == 1):
                    ctrlData_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.egrVifId = int(args[20])
                    ctrlData_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.ecmpSize = int(args[16])
                else:
                    ctrlData_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode = int(args[22])
                    ctrlData_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command = int(args[16])
                ctrlData_Ptr.iAclDataType.aclData.dp = int(args[23])
                if (ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRw == 0):
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[24])
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[25])
                else:
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[26])

                indexList_Ptr = new_xpsHashIndexList_tp()
                #print('Input Arguments are, devId=%d, priority=%d' % (args[0],args[1]))
                ret = xpsWcmIaclWriteBaclOverflowEntry(args[0],args[1],args[2],keyData_Ptr,ctrlData_Ptr)
                if ret != 0:
                    print('Failed to set wcm Iacl rule for Bacl, index - %d' %(args[1]))
                    print('Return Value = %d' % (ret))
                else:
                    print('Wcm Iacl rule for Bacl set successfully, index - %d' %(args[1]))
                    pass
                delete_xpIaclData_tp(ctrlData_Ptr)
                delete_xpWcmIaclkeyFieldList_tp(keyData_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args "
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsWcmIaclWriteRaclOverflowEntry
    #/********************************************************************************/
    def do_wcm_iacl_write_racl_overflow_entry(self, arg):
        '''
         xpsWcmIaclWriteRaclOverflowEntry: Enter [ devId,overflowIndex,priority,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan, {flds,value,mask}, {flds,value,mask} ...upto number of keys wants to be programe ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 28
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,overflowIndex,priority,numFlds,isValid,type,isTerminal,enPktCmdUpd,enRedirectToEvif,enDpRemark,enPolicer,qosOrVlanRw,enMirrorSsnUpd,enTcRemark,enDscpRemarkOrVrfUpd,enPcpRemarkOrStagRw,pktCmd,TC,mirrorSessionId,encapType,eVifId,policerId,rsnCode,DP,PCP,DSCP,vlan, {flds,value,mask}, {flds,value,mask} ...upto number of keys wants to be programe \
                    e.g.: \n \
                    wcm_iacl_write_racl_overflow_entry 0 52 0 3 1 XP_IACL_V4_TYPE 0 1 0 1 0 1 0 0 0 0 0 0 0 0 0 0 19 0 0 0 852 XP_IACL_KEY_TYPE_V4,0,0 XP_IACL_MAC_DA,0:0:0:0:0:0,0 XP_IACL_MAC_SA,0:0:0:0:0:0,0]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            keyData_Ptr = new_xpWcmIaclkeyFieldList_tp()
            keyData_Ptr.numFlds = int(args[3])
            keyData_Ptr.isValid = int(args[4])
            keyData_Ptr.type = eval(args[5])
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
                xpIaclkeyField_t_ptr = new_xpWcmIaclkeyField_tp()
                xpIaclkeyField_t_arr = new_xpWcmIaclkeyField_arr(len(fldList))
                for fldIdx,defFld in enumerate(fldList):
                       if(eval(args[5]) == 0):
                           xpIaclkeyField_t_ptr.fld.v4Fld = eval(defFld)
                           byteSize = mapxpIaclV4KeyByteMask[eval(fldList[fldIdx])]
                       else:
                           xpIaclkeyField_t_ptr.fld.v6Fld = eval(defFld)
                           byteSize = mapxpIaclV6KeyByteMask[eval(fldList[fldIdx])]
                       byteLen = bin(byteSize).count('1')
                       valPtr = new_uint8Arr_tp(byteLen)
                       maskPtr = new_uint8Arr_tp(byteLen)
                       if(not defFld in fldList):
                           for valIdx in range(byteLen):
                               uint8Arr_tp_setitem(valPtr, valIdx, 0)
                               uint8Arr_tp_setitem(maskPtr, valIdx,0xff)
                       else:
                           hexFlag = 0
                           if('.' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].split('.')
                               if(maskList[fldIdx] == '0'):
                                   fldMask = ['00'] * len(fldVal)
                               else:
                                   fldMask = ['255'] * len(fldVal)
                           elif(':' in  valueList[fldIdx]):
                               fldVal = valueList[fldIdx].split(':')
                               if(maskList[fldIdx] == '0'):
                                   fldMask = ['00'] * len(fldVal)
                               else:
                                   fldMask = ['ff'] * len(fldVal)
                               hexFlag = 1
                           else:
                               tmpValue = format(int(valueList[fldIdx]), '0' + str(byteLen*2) + 'X')
                               fldVal = [tmpValue[i:i+2] for i in range(0, len(tmpValue), 2)]
                               tmpMask = str('1' * ((byteLen*8)- len(bin(int(valueList[fldIdx])).lstrip('0b'))) + str(maskList[fldIdx] * len(bin(int(valueList[fldIdx])).lstrip('0b'))))
                               tmpMask = hex(int(tmpMask, 2))
                               tmpMask = format(int(tmpMask,16), '0' + str(byteLen*2) + 'x')
                               fldMask = [tmpMask[i:i+2] for i in range(0, len(tmpMask), 2)]
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
                       xpWcmIaclkeyField_arr_setitem(xpIaclkeyField_t_arr, fldIdx, xpIaclkeyField_t_ptr)
                keyData_Ptr.fldList = xpIaclkeyField_t_arr
                ctrlData_Ptr = new_xpIaclData_tp()
                ctrlData_Ptr.iAclDataType.aclData.isTerminal = int(args[6])
                ctrlData_Ptr.iAclDataType.aclData.enPktCmdUpd = int(args[7])
                ctrlData_Ptr.iAclDataType.aclData.enRedirectToEvif = int(args[8])
                ctrlData_Ptr.iAclDataType.aclData.enDpRemark = int(args[9])
                ctrlData_Ptr.iAclDataType.aclData.enPolicer = int(args[10])
                ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRw = int(args[11])
                ctrlData_Ptr.iAclDataType.aclData.enMirrorSsnUpd = int(args[12])
                ctrlData_Ptr.iAclDataType.aclData.enTcRemark = int(args[13])
                ctrlData_Ptr.iAclDataType.aclData.enDscpRemarkOrVrfUpd = int(args[14])
                ctrlData_Ptr.iAclDataType.aclData.enPcpRemarkOrStagRw = int(args[15])
                ctrlData_Ptr.iAclDataType.aclData.tc = int(args[17])
                ctrlData_Ptr.iAclDataType.aclData.mirrorSessionId = int(args[18])
                ctrlData_Ptr.iAclDataType.aclData.encapType = int(args[19])
                ctrlData_Ptr.iAclDataType.aclData.policerId = int(args[21])
                if(ctrlData_Ptr.iAclDataType.aclData.enRedirectToEvif == 1):
                    ctrlData_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.egrVifId = int(args[20])
                    ctrlData_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.ecmpSize = int(args[16])
                else:
                    ctrlData_Ptr.iAclDataType.aclData.egrVifIdOrRsnCode.rsnCode = int(args[22])
                    ctrlData_Ptr.iAclDataType.aclData.pktCmdOrEcmpSize.pktCmd.command = int(args[16])
                ctrlData_Ptr.iAclDataType.aclData.dp = int(args[23])
                if (ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRw == 0):
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.pcp = int(args[24])
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.qosData.dscp = int(args[25])
                else:
                    ctrlData_Ptr.iAclDataType.aclData.qosOrVlanRwData.vlan = int(args[26])

                indexList_Ptr = new_xpsHashIndexList_tp()
                #print('Input Arguments are, devId=%d, priority=%d' % (args[0],args[1]))
                ret = xpsWcmIaclWriteRaclOverflowEntry(args[0],args[1],args[2],keyData_Ptr,ctrlData_Ptr)
                if ret != 0:
                    print('Failed to set wcm Iacl rule for Racl, index - %d' %(args[1]))
                    print('Return Value = %d' % (ret))
                else:
                    print('Wcm Iacl rule for Racl set successfully, index - %d' %(args[1]))
                    pass
                delete_xpIaclData_tp(ctrlData_Ptr)
                delete_xpWcmIaclkeyFieldList_tp(keyData_Ptr)
            else:
                if fldvalMask ==  '-1':
                   print "Invalid input args "
                else:
                   print "numFlds and given number of flds not matched"
                   print "numFlds = %s"% args[2]
                   print "flds = %s"% fldList

    #/********************************************************************************/
    # command for xpsWcmIaclWritePaclTranslatorEntry
    #/********************************************************************************/
    def do_wcm_iacl_write_pacl_translator_entry(self, arg):
        '''
         xpsWcmIaclWritePaclTranslatorEntry: Enter [ devId,index,result ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,result ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, index=%d, result=%d' % (args[0],args[1],args[2]))
            ret = xpsWcmIaclWritePaclTranslatorEntry(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclReadPaclTranslatorEntry
    #/********************************************************************************/
    def do_wcm_iacl_read_pacl_translator_entry(self, arg):
        '''
         xpsWcmIaclReadPaclTranslatorEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            result_Ptr = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsWcmIaclReadPaclTranslatorEntry(args[0],args[1],result_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('result = %d' % (uint32_tp_value(result_Ptr)))
                pass
            delete_uint32_tp(result_Ptr)

    #/********************************************************************************/
    # command for xpsWcmIaclWriteBaclTranslatorEntry
    #/********************************************************************************/
    def do_wcm_iacl_write_bacl_translator_entry(self, arg):
        '''
         xpsWcmIaclWriteBaclTranslatorEntry: Enter [ devId,index,result ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,result ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, index=%d, result=%d' % (args[0],args[1],args[2]))
            ret = xpsWcmIaclWriteBaclTranslatorEntry(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclReadBaclTranslatorEntry
    #/********************************************************************************/
    def do_wcm_iacl_read_bacl_translator_entry(self, arg):
        '''
         xpsWcmIaclReadBaclTranslatorEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            result_Ptr = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsWcmIaclReadBaclTranslatorEntry(args[0],args[1],result_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('result = %d' % (uint32_tp_value(result_Ptr)))
                pass
            delete_uint32_tp(result_Ptr)

    #/********************************************************************************/
    # command for xpsWcmIaclWriteRaclTranslatorEntry
    #/********************************************************************************/
    def do_wcm_iacl_write_racl_translator_entry(self, arg):
        '''
         xpsWcmIaclWriteRaclTranslatorEntry: Enter [ devId,index,result ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 3
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index,result ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            args[2] = int(args[2])
            #print('Input Arguments are, devId=%d, index=%d, result=%d' % (args[0],args[1],args[2]))
            ret = xpsWcmIaclWriteRaclTranslatorEntry(args[0],args[1],args[2])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclReadRaclTranslatorEntry
    #/********************************************************************************/
    def do_wcm_iacl_read_racl_translator_entry(self, arg):
        '''
         xpsWcmIaclReadRaclTranslatorEntry: Enter [ devId,index ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 2
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,index ]')
        else:
            args[0] = int(args[0])
            args[1] = int(args[1])
            result_Ptr = new_uint32_tp()
            #print('Input Arguments are, devId=%d, index=%d' % (args[0],args[1]))
            ret = xpsWcmIaclReadRaclTranslatorEntry(args[0],args[1],result_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('result = %d' % (uint32_tp_value(result_Ptr)))
                pass
            delete_uint32_tp(result_Ptr)
    #/********************************************************************************/
    # command for xpsWcmIaclAddPaclRangeEntry
    #/********************************************************************************/
    def do_wcm_iacl_add_pacl_range_entry(self, arg):
        '''
         xpsWcmIaclAddPaclRangeEntry: Enter [ devId,rangeType,start,end,result ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,rangeType,start,end,result ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, rangeType=%d, start=%d, end=%d, result=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsWcmIaclAddPaclRangeEntry(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('added wcm Iacl range entry for Pacl')
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclRemovePaclRangeEntry
    #/********************************************************************************/
    def do_wcm_iacl_remove_pacl_range_entry(self, arg):
        '''
         xpsWcmIaclRemovePaclRangeEntry: Enter [ devId,rangeType,start,end ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,rangeType,start,end ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, rangeType=%d, start=%d, end=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsWcmIaclRemovePaclRangeEntry(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('removed wcm Iacl range entry for Pacl')
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclAddBaclRangeEntry
    #/********************************************************************************/
    def do_wcm_iacl_add_bacl_range_entry(self, arg):
        '''
         xpsWcmIaclAddBaclRangeEntry: Enter [ devId,rangeType,start,end,result ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,rangeType,start,end,result ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, rangeType=%d, start=%d, end=%d, result=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsWcmIaclAddBaclRangeEntry(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('added wcm Iacl range entry for Bacl')
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclRemoveBaclRangeEntry
    #/********************************************************************************/
    def do_wcm_iacl_remove_bacl_range_entry(self, arg):
        '''
         xpsWcmIaclRemoveBaclRangeEntry: Enter [ devId,rangeType,start,end ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,rangeType,start,end ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, rangeType=%d, start=%d, end=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsWcmIaclRemoveBaclRangeEntry(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('removed wcm Iacl range entry for Bacl')
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclAddRaclRangeEntry
    #/********************************************************************************/
    def do_wcm_iacl_add_racl_range_entry(self, arg):
        '''
         xpsWcmIaclAddRaclRangeEntry: Enter [ devId,rangeType,start,end,result ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,rangeType,start,end,result ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            args[4] = int(args[4])
            #print('Input Arguments are, devId=%d, rangeType=%d, start=%d, end=%d, result=%d' % (args[0],args[1],args[2],args[3],args[4]))
            ret = xpsWcmIaclAddRaclRangeEntry(args[0],args[1],args[2],args[3],args[4])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('Added wcm Iacl range entry for Racl')
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclRemoveRaclRangeEntry
    #/********************************************************************************/
    def do_wcm_iacl_remove_racl_range_entry(self, arg):
        '''
         xpsWcmIaclRemoveRaclRangeEntry: Enter [ devId,rangeType,start,end ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 4
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,rangeType,start,end ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = int(args[2])
            args[3] = int(args[3])
            #print('Input Arguments are, devId=%d, rangeType=%d, start=%d, end=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsWcmIaclRemoveRaclRangeEntry(args[0],args[1],args[2],args[3])
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('removed wcm Iacl range entry for Racl')
                pass
    #/********************************************************************************/
    # command for xpsWcmIaclAddPaclPrefixTranslation
    #/********************************************************************************/
    def do_wcm_iacl_add_pacl_prefix_translation(self, arg):
        '''
         xpsWcmIaclAddPaclPrefixTranslation: Enter [ devId,prefixiLookUpType,prefixType,ipMaskLen,SIP/DIP ]
         Example: wcm_iacl_add_pacl_prefix_translation 0 XP_PRELOOKUP_V4_DIP XP_PREFIX_TYPE_IPV4 32 192.168.220.57
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,prefixiLookUpType,prefixType,ipMaskLen,SIP/DIP ]')
            print('Example: wcm_iacl_add_pacl_prefix_translation 0 XP_PRELOOKUP_V4_DIP XP_PREFIX_TYPE_IPV4 32 192.168.220.57')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            xpsPrefix_Ptr = new_xpsPrefix_tp()
            xpsPrefix_Ptr.type = args[2]
            xpsPrefix_Ptr.ipMaskLen = args[3]
            dprefixId = 0
            valPtr = new_uint8Arr_tp(4)
            if(args[2] == XP_PREFIX_TYPE_IPV4):
                tmpFld = args[4].split('.')
                if(len(tmpFld) == 4):
                    for x in range(4):
                        xpsPrefix_Ptr.ipv4Addr[int(x)] = int(tmpFld[3 - int(x)])
                    dprefixId = (int(tmpFld[0]) << 24) | (int(tmpFld[1]) << 16) | (int(tmpFld[2]) << 8) | int(tmpFld[3])
                else:
                    print 'Invalid SIP/DIP value'
                    return
            else:
                print 'Currently Only Ipv4 Prefix Type supported'
                return
            #print('Input Arguments are, devId=%d, prefixLookUpType=%d, prefix=%d, prefixTransId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsWcmIaclAddPaclPrefixTranslation(args[0],args[1],xpsPrefix_Ptr,dprefixId)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('add prefix translation for Pacl done!')
                pass

    #/********************************************************************************/
    # command for xpsWcmIaclRemovePaclPrefixTranslation
    #/********************************************************************************/
    def do_wcm_iacl_remove_pacl_prefix_translation(self, arg):
        '''
         xpsWcmIaclRemovePaclPrefixTranslation: Enter [ devId,prefixLookUpType,prefixType,prefix,ipMaskLen,SIP/DIP ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,prefixLookUpType,prefixType,prefix,ipMaskLen,SIP/DIP ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            xpsPrefix_Ptr = new_xpsPrefix_tp()
            xpsPrefix_Ptr.type = args[2]
            xpsPrefix_Ptr.ipMaskLen = args[3]
            if(args[2] == XP_PREFIX_TYPE_IPV4):
                tmpFld = args[4].split('.')
                if(len(tmpFld) == 4):
                    for x in range(4):
                        xpsPrefix_Ptr.ipv4Addr[int(x)] = int(tmpFld[int(x)])
                else:
                    print 'Invalid SIP/DIP value'
                    return
            else:
                print 'Currently Only Ipv4 Prefix Type supported'
                return
            #print('Input Arguments are, devId=%d, prefixType=%d, prefix=%d' % (args[0],args[1],args[2]))
            ret = xpsWcmIaclRemovePaclPrefixTranslation(args[0],args[1],xpsPrefix_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('Remove prefix translation for Pacl done!')
                pass

    #/********************************************************************************/
    # command for xpsWcmIaclAddBaclPrefixTranslation
    #/********************************************************************************/
    def do_wcm_iacl_add_bacl_prefix_translation(self, arg):
        '''
         xpsWcmIaclAddBaclPrefixTranslation: Enter [ devId,prefixLookUpType,prefixType,ipMaskLen,SIP/DIP ]
         Example: wcm_iacl_add_bacl_prefix_translation 0 XP_PRELOOKUP_V4_DIP XP_PREFIX_TYPE_IPV4 32 192.168.220.57
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,prefixiLookUpType,prefixType,ipMaskLen,SIP/DIP ]')
            print('Example: wcm_iacl_add_bacl_prefix_translation 0 XP_PRELOOKUP_V4_DIP XP_PREFIX_TYPE_IPV4 32 192.168.220.57')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            xpsPrefix_Ptr = new_xpsPrefix_tp()
            xpsPrefix_Ptr.type = args[2]
            xpsPrefix_Ptr.ipMaskLen = args[3]
            dprefixId = 0
            if(args[2] == XP_PREFIX_TYPE_IPV4):
                tmpFld = args[4].split('.')
                if(len(tmpFld) == 4):
                    for x in range(4):
                        xpsPrefix_Ptr.ipv4Addr[int(x)] = int(tmpFld[3 - int(x)])
                    dprefixId = (int(tmpFld[0]) << 24) | (int(tmpFld[1]) << 16) | (int(tmpFld[2]) << 8) | int(tmpFld[3])
                else:
                    print 'Invalid SIP/DIP value'
                    return
            else:
                print 'Currently Only Ipv4 Prefix Type supported'
                return
            #print('Input Arguments are, devId=%d, prefixLookUpType=%d, prefix=%d, prefixTransId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsWcmIaclAddBaclPrefixTranslation(args[0],args[1],xpsPrefix_Ptr,dprefixId)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('add prefix translation for Bacl done!')
                pass

    #/********************************************************************************/
    # command for xpsWcmIaclRemoveBaclPrefixTranslation
    #/********************************************************************************/
    def do_wcm_iacl_remove_bacl_prefix_translation(self, arg):
        '''
         xpsWcmIaclRemoveBaclPrefixTranslation: Enter [ devId,prefixLookUpType,prefixType,prefix,ipMaskLen,SIP/DIP ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,prefixLookUpType,prefixType,prefix,ipMaskLen,SIP/DIP ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            xpsPrefix_Ptr = new_xpsPrefix_tp()
            xpsPrefix_Ptr.type = args[2]
            xpsPrefix_Ptr.ipMaskLen = args[3]
            if(args[2] == XP_PREFIX_TYPE_IPV4):
                tmpFld = args[4].split('.')
                if(len(tmpFld) == 4):
                    for x in range(4):
                        xpsPrefix_Ptr.ipv4Addr[int(x)] = int(tmpFld[int(x)])
                else:
                    print 'Invalid SIP/DIP value'
                    return
            else:
                print 'Currently Only Ipv4 Prefix Type supported'
                return
            #print('Input Arguments are, devId=%d, prefixType=%d, prefix=%d' % (args[0],args[1],args[2]))
            ret = xpsWcmIaclRemoveBaclPrefixTranslation(args[0],args[1],xpsPrefix_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('Remove prefix translation for Bacl done!')
                pass

    #/********************************************************************************/
    # command for xpsWcmIaclAddRaclPrefixTranslation
    #/********************************************************************************/
    def do_wcm_iacl_add_racl_prefix_translation(self, arg):
        '''
         xpsWcmIaclAddBaclPrefixTranslation: Enter [ devId,prefixiLookUpType,prefixType,ipMaskLen,SIP/DIP ]
         Example: wcm_iacl_add_racl_prefix_translation 0 XP_PRELOOKUP_V4_DIP XP_PREFIX_TYPE_IPV4 32 192.168.220.57
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,prefixiLookUpType,prefixType,ipMaskLen,SIP/DIP ]')
            print('Example: wcm_iacl_add_racl_prefix_translation 0 XP_PRELOOKUP_V4_DIP XP_PREFIX_TYPE_IPV4 32 192.168.220.57')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            xpsPrefix_Ptr = new_xpsPrefix_tp()
            xpsPrefix_Ptr.type = args[2]
            xpsPrefix_Ptr.ipMaskLen = args[3]
            dprefixId = 0
            if(args[2] == XP_PREFIX_TYPE_IPV4):
                tmpFld = args[4].split('.')
                if(len(tmpFld) == 4):
                    for x in range(4):
                        xpsPrefix_Ptr.ipv4Addr[int(x)] = int(tmpFld[3 - int(x)])
                    dprefixId = (int(tmpFld[0]) << 24) | (int(tmpFld[1]) << 16) | (int(tmpFld[2]) << 8) | int(tmpFld[3])
                else:
                    print 'Invalid SIP/DIP value'
                    return
            else:
                print 'Currently Only Ipv4 Prefix Type supported'
                return
            #print('Input Arguments are, devId=%d, prefixLookUpType=%d, prefix=%d, prefixTransId=%d' % (args[0],args[1],args[2],args[3]))
            ret = xpsWcmIaclAddRaclPrefixTranslation(args[0],args[1],xpsPrefix_Ptr,dprefixId)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('add prefix translation for Racl done!')
                pass

    #/********************************************************************************/
    # command for xpsWcmIaclRemoveRaclPrefixTranslation
    #/********************************************************************************/
    def do_wcm_iacl_remove_racl_prefix_translation(self, arg):
        '''
         xpsWcmIaclRemoveRaclPrefixTranslation: Enter [ devId,prefixLookUpType,prefixType,prefix,ipMaskLen,SIP/DIP ]
        '''
        args = re.split(';| ',arg)
        numArgsReq = 5
        if  (numArgsReq > 0 and args[0] == '') or (len(args) < numArgsReq) :
            print('Invalid input, Enter [ devId,prefixLookUpType,prefixType,prefix,ipMaskLen,SIP/DIP ]')
        else:
            args[0] = int(args[0])
            args[1] = eval(args[1])
            args[2] = eval(args[2])
            args[3] = int(args[3])
            xpsPrefix_Ptr = new_xpsPrefix_tp()
            xpsPrefix_Ptr.type = args[2]
            xpsPrefix_Ptr.ipMaskLen = args[3]
            if(args[2] == XP_PREFIX_TYPE_IPV4):
                tmpFld = args[4].split('.')
                if(len(tmpFld) == 4):
                    for x in range(4):
                        xpsPrefix_Ptr.ipv4Addr[int(x)] = int(tmpFld[int(x)])
                else:
                    print 'Invalid SIP/DIP value'
                    return
            else:
                print 'Currently Only Ipv4 Prefix Type supported'
                return
            #print('Input Arguments are, devId=%d, prefixType=%d, prefix=%d' % (args[0],args[1],args[2]))
            ret = xpsWcmIaclRemoveRaclPrefixTranslation(args[0],args[1],xpsPrefix_Ptr)
            if ret != 0:
                print('Return Value = %d' % (ret))
            else:
                print('Remove prefix translation for Racl done!')
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
