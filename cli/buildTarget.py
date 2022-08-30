#!/usr/lib/python
# buildTarget.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#********************************************************************************/

import os
import sys

def getBuildTargetDir():
    xp_root = os.getenv('XP_ROOT', "")

    if not xp_root:
        buildTargetDir, filename = os.path.split(os.path.abspath(__file__))
        buildTargetDir += '/../'
    else:
        buildTargetDir = xp_root
    buildTargetDir += "/dist/"

    return buildTargetDir


buildTargetDir = getBuildTargetDir()
sys.path.append(buildTargetDir)
if(os.path.isfile(buildTargetDir + "/saiShell.py")):
	from saiShell import *
	print '*' * 80
	print "xpShell running with saiShell_py Target"
	print '*' * 80
elif(os.path.isfile(buildTargetDir + "/sdk.py")):
	from sdk import *
	print '*' * 80
	print "xpShell running with sdk_py Target"
	print '*' * 80
	#xpDeviceMgr.instance().printVersion()
	print '*' * 80
elif(os.path.isfile(buildTargetDir + "/xpsApp.py")):
	from xpsApp import *
	print '*' * 80
	print "xpShell running with xpsApp_py Target"
	print '*' * 80
	xpDeviceMgr.instance().printVersion()
	print '*' * 80
elif(os.path.isfile(buildTargetDir + "/ksdk.py")):
	from ksdk import *
	print '*' * 80
	print "xpShell running with ksdk_py Target"
	print '*' * 80
	xpDeviceMgr.instance().printVersion()
	print '*' * 80
elif(os.path.isfile(buildTargetDir + "/saiShell.py")):
	from saiShell import *
	print '*' * 80
	print "xpShell running with saiShell_py Target"
	print '*' * 80
elif(os.path.isfile(buildTargetDir + "/openXps.py")):
	from openXps import *
	print '*' * 80
	print "xpShell running with openXps_py Target"
	print '*' * 80
	xpDeviceMgr.instance().printVersion()
	print '*' * 80
elif(os.path.isfile(buildTargetDir + "/ksaiShell.py")):
	from ksaiShell import *
	print '*' * 80
	print "xpShell running with ksaiShell_py Target"
	print '*' * 80
else:
	print '*' * 80
	print 'None of the target build, either build xpsApp_py or sdk_py or saiShell_py.'
	print '*' * 80

from displayTables import *


