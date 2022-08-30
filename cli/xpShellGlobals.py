# xpShellGlobals.py
#
#/*******************************************************************************
#* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
#* subject to the limited use license agreement by and between Marvell and you, *
#* your employer or other entity on behalf of whom you act. In the absence of   *
#* such license agreement the following file is subject to Marvell`s standard   *
#* Limited Use License Agreement.                                               *
#********************************************************************************/

def globalInit():

    global cmdRetVal
    global userDefVar
    global varDict
    global sessionFlag
    global xpConfigFileRd
    global xpConfigFileWr
    global sessionId
    global adminUser
    global rootUser
    global adminFlag
    global retVal
    global noPrompt
    global disableLog

    # Initialize global variables
    cmdRetVal = 0
    userDefVar = ''
    varDict = {}
    sessionFlag = 0
    xpConfigFileRd = ''
    xpConfigFileWr = ''
    sessionId = 0
    adminUser = 'admin'
    rootUser = 'root'
    adminFlag = 0
    retVal = 0
    noPrompt = 0
    disableLog = 0
