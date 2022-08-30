::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::  build_cpss.bat
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::  CPSS build wrapper over Makefile, for Windows 32 and 64 simulation
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: ARGUMENTS    : [Can be defined in any order]
:: TARGET       : VC10 VC10_64
:: FAMILY       : DX_ALL PX_ALL DXPX
:: PRODUCT      : CPSS_ENABLER CPSS_LIB
:: UT           : UTF_NO (by default UTF is included)
:: TM           : TM_NO (by default TM is included)
:: LOG          : CPSS_LOG_NO (by default log is enabled)
:: LUA          : NOLUA -  by default LUA code is included, NOLUA removes LUA support
:: GALTIS       : NOGALTIS -  by default GALTIS code is included, NOGALTIS removes GALTIS support
:: EZ_BRINGUP   : EZ_BRINGUP_NO - by default EZ_BRINGUP code is included, EZ_BRINGUP_NO removes EZ_BRINGUP compilation/support
:: MPD          : MPD_NO - by default MPD code is included, MPD_NO removes MPD compilation/support
:: CPSS_USE_MUTEX_PROFILER: Use Mutex profiler, disabled by default
:: CPSS_TRAFFIC_API_LOCK_DISABLE: Disable lock on APIs, by default lock is enabled
:: GM_MODE      : GM_ALDRIN2 GM_FALCON GM_HAWK GM_IRONMAN
:: Additional
::  options     : NO_COPY_BIN - don't copy(or check) binary files from f:\Objects\cpss\bin to c:\temp\cpss_bin
::  options     : VERBOSE - print all compilation log
::  options     : D_OFF : disable compilation with debug information (by default compiled with debug info)
::  options     : DEP_NO - by default image will be compiled with dependency files, DEP_NO skips dependency
::  options     : CLEAN|clean - clean objects
::
::  variables                       default value
::------------------------------------------------------------------------
::  COMPILATION_ROOT                %CD%\compilation_root
::  CPSS_PATH                       %CD%
::  USER_BASE                       %CD%
::
::      EXAMPLES:
::          set COMPILATION_ROOT=c:\compilation_root
::
::          build_cpss.bat VC10 DX_ALL
::              in this case objects will be created in folder
::              cpss\compilation_root
::
::  C_SINGLE_COMPILATION -  C file path for single compilation (no defaulf value) 
::                   for example:
::                       SET C_SINGLE_COMPILATION=common\src\cpss\driver\interrupts\cpssDrvComIntEvReqQueues.c
::
::  GM_LIB - <explicit path to file> can be set prior to calling this batch file (the Makefile will use it)
::      example : 
::          set GM_LIB=c:\temp\dtemp\PPInterfaceIronmanGm_VC10\PPInterfaceIronmanGm_VC10\Debug\PPInterfaceIronmanGm_VC10.lib
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

@ ECHO OFF
SETLOCAL EnableDelayedExpansion

set STARTTIME=%TIME%
echo          ---------------
echo Start    : %STARTTIME%
echo          ---------------

:: In case this script has been called outside of cpss folder:
::  Extract caller path, and cpss path
set CALLER_PATH=%cd%
cd %0\..
set CPSS_PATH=%cd%
cd %CALLER_PATH%
echo CPSS_PATH:    %CPSS_PATH%

:: If called from outside CPSS folder, define -C CPSS_DIR and caller path for makefile
if %CALLER_PATH% == %CPSS_PATH% (
  set CALLER_PATH=
) else (
  echo CALLER_PATH: %CALLER_PATH%
  SET CPSS_DIR=-C %CPSS_PATH%
  SET CALLER_PATH=CALLER_PATH=%CALLER_PATH%
)

:: get number of CPU cores in system
set NUM_CORES=%NUMBER_OF_PROCESSORS%
echo NUM_CORES = %NUM_CORES%
::::::::::::::::::::::::::
:: RESET LOCAL VARIABLES
::::::::::::::::::::::::::
SET TARGET=
SET GOAL=appDemo.exe
SET FAMILY=
SET UT=INCLUDE_UTF=y
SET TM=INCLUDE_TM=y
SET LOG=LOG_ENABLE=y
SET EZ_BRINGUP=INCLUDE_EZ_BRINGUP=y
SET MPD=INCLUDE_MPD=y
SET EXT=
SET GM=
if not exist c:\temp\cpss_bin SET COPY_BIN=YES
SET SILENT=-s
SET GALTIS=INCLUDE_GALTIS=y
SET MTX=USE_MUTEX_PROFILER=n
SET LCK=TRAFFIC_API_LOCK_DISABLE=n
SET DBG=DEBUG_INFO=y
SET DEP=
SET CLEAN_MODE=
set BUILD_FOLDER=
:::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: DEFINE LISTS OF LEGAL COMMAND LINE PARAMETER VALUES
:::::::::::::::::::::::::::::::::::::::::::::::::::::::
SET TARGET_LIST=VC10,VC10_64
SET FAMILY_LIST=DX_ALL,PX_ALL,DXPX
SET PRODUCT_LIST=CPSS_ENABLER,CPSS_LIB
SET GM_LIST=GM_ALDRIN2,GM_FALCON,GM_HAWK,GM_IRONMAN
SET GOAL_LIST=CLEAN,clean,CPSS_LIB,CPSS_ENABLER

::::::::::::::::::::::::::::::::::::::::::::::
:: set local variable RUN_COMPILE_CMD allowing
:: to run CMD for CPSS compiling
::::::::::::::::::::::::::::::::::::::::::::::
title cpss_build

:::::::::::::::::::::::::::::::::::::
:: check command line "help" option
:::::::::::::::::::::::::::::::::::::
IF "%1" == "help" (
   CALL:HELP_FUNCTION
   GOTO END
)


SET CPSS_TOOLS_PATH=%CPSS_PATH%\tools
SET FILE_ZSH_EXE=%CPSS_TOOLS_PATH%\bin\zsh.exe

::::::::::::::::::::::::::::::::
:: check comand line arguments
::::::::::::::::::::::::::::::::

:ARGS_START
if -%1-==-- goto ARGS_END
    SET FOUND=
    FOR %%j IN (%TARGET_LIST%) DO (
        IF %1 == %%j (
            SET FOUND=TRUE
            if %1 == VC10_64 (
                SET TARGET=TARGET=win64
                call "c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\amd64\vcvars64.bat"
            )
            if %1 == VC10 (
                SET TARGET=TARGET=win32
                call "c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat"
            )
        )
    )
    FOR %%j IN (%FAMILY_LIST%) DO (
        IF %1 == %%j (
            if %1 == DX_ALL (SET FAMILY=FAMILY=DX)
            if %1 == PX_ALL (
                SET FAMILY=FAMILY=PX
                set TM=INCLUDE_TM=n
                SET EZ_BRINGUP=INCLUDE_EZ_BRINGUP=n
                SET MPD=INCLUDE_MPD=n
            )
            if %1 == DXPX (SET FAMILY=FAMILY=DXPX)
            SET FOUND=TRUE
        )
    )
    FOR %%j IN (%PRODUCT_LIST%) DO (
        IF %1 == %%j (
           SET PRODUCT=%%j
           SET FOUND=TRUE
        )
    )
    IF %1 == UTF_NO (
        SET UT=INCLUDE_UTF=n
        SET FOUND=TRUE
    )
    IF %1 == TM_NO (
        SET TM=INCLUDE_TM=n
        SET FOUND=TRUE
    )
    IF %1 == CPSS_LOG_NO (
        SET LOG=LOG_ENABLE=n
        SET FOUND=TRUE
    )
    IF %1 == NOLUA (
        SET LUA_MODE=CMD_LUA_CLI=n
        SET FOUND=TRUE
    )
    IF %1 == EZ_BRINGUP_NO (
        SET EZ_BRINGUP=INCLUDE_EZ_BRINGUP=n
        SET FOUND=TRUE
    )
    IF %1 == MPD_NO (
        SET MPD=INCLUDE_MPD=n
        SET FOUND=TRUE
    )
    IF %1 == NOGALTIS (
        SET GALTIS=INCLUDE_GALTIS=n
        SET FOUND=TRUE
    )
    FOR %%j IN (%GM_LIST%) DO (
        IF %1 == %%j (
           SET GM=%%j
           SET FOUND=TRUE
        )
    )
    IF %1 == NO_COPY_BIN (
        SET COPY_BIN=
        SET FOUND=TRUE
    )
    IF %1 == CPSS_TRAFFIC_API_LOCK_DISABLE (
        SET LCK=TRAFFIC_API_LOCK_DISABLE=y
        SET FOUND=TRUE
    )
    IF %1 == CPSS_USE_MUTEX_PROFILER (
        SET MTX=USE_MUTEX_PROFILER=y
        SET FOUND=TRUE
    )
    IF %1 == VERBOSE (
        SET SILENT=
        SET FOUND=TRUE
    )
    FOR %%j IN (%DEBUG_INFO_LIST%) DO (
        IF %1 == D_OFF (
           SET DBG=DEBUG_INFO=n
           SET FOUND=TRUE
        )
    )
    IF %1 == DEP_NO (
        SET DEP=DEPENDENCY=DEP_NO
        SET FOUND=TRUE
    )
    IF %1 == C005 (
        SET EXT=EXT_MODULE=C005
        SET FOUND=TRUE
    )
    FOR %%j IN (%GOAL_LIST%) DO (
        IF %1 == %%j (
            SET FOUND=TRUE
            IF /I %1 == CLEAN (
                SET GOAL=clean
            )
            IF %1 == CPSS_LIB (
                SET GOAL=cpsslibs
            )
            IF %1 == CPSS_ENABLER (
                ::Alredy set as default goal
                ::SET GOAL=appDemo.exe
            )
        )
    )
    IF NOT DEFINED FOUND (
       echo WRONG TOKEN IN COMMAND LINE - %1
       GOTO:eof
    )
    
    shift
    goto ARGS_START
:ARGS_END

SET PATH=c:\temp\cpss_bin\;%PATH%

IF not -%GM%- == -- (
	SET GM=GM=%GM:*_=%
	)

IF -%COPY_BIN%- == -YES- (
    @del /f /s /q c:\temp\cpss_bin
    @mkdir c:\temp\cpss_bin
    @if exist %CPSS_PATH%\tools\bin (xcopy /q %CPSS_PATH%\tools\bin c:\temp\cpss_bin) else (@xcopy /q \\fileril103\dev\objects\cpss\bin\cpss_bin\* c:\temp\cpss_bin)
)

if DEFINED COMPILATION_ROOT (
    set BUILD_FOLDER=BUILD_FOLDER=%COMPILATION_ROOT%
)

echo gmake2 %CPSS_DIR% %TARGET% %FAMILY% %UT% %LUA% %TM% %GALTIS% %MTX% %LCK% %LOG% %EZ_BRINGUP% %MPD% %GM% %DBG% %DEP% %GOAL% %SILENT% %EXT% -j %NUM_CORES% %BUILD_FOLDER% %CALLER_PATH%
gmake2 %CPSS_DIR% %TARGET% %FAMILY% %UT% %LUA% %TM% %GALTIS% %MTX% %LCK% %LOG% %EZ_BRINGUP% %MPD% %GM% %DBG% %DEP% %GOAL% %SILENT% %EXT% -j %NUM_CORES% %BUILD_FOLDER% %CALLER_PATH%
IF %ERRORLEVEL% == 0 SET success=YES
GOTO:END

:HELP_FUNCTION
ECHO ------------------------------------------------------------------
ECHO COMMAND LINE FORMAT:

ECHO "build_cpss.bat < PP family > < target > < Options >"
ECHO ------------------------------------------------------------------
ECHO tagets: VC10, VC10_64
ECHO PP families: DX_ALL, PX_ALL, DXPX
ECHO ------------------------------------------------------------------
ECHO OPTIONS:
ECHO     UTF_NO        - UT code not included
ECHO     TM_NO         - TM code not included
ECHO     CPSS_LOG_NO   - CPSS API log is disabled
ECHO     NOLUA         - Lua code not included
ECHO     NOGALTIS      - Galtis code not included
ECHO     %GM_LIST%
ECHO     EZ_BRINGUP_NO - EZ_BRINGUP code not required
ECHO     MPD_NO        - MPD code not required
ECHO     VERBOSE       - enable make verbosity
ECHO     CPSS_USE_MUTEX_PROFILER - Use Mutex profiler
ECHO     CPSS_TRAFFIC_API_LOCK_DISABLE - Disable lock on APIs
ECHO     NO_COPY_BIN   - skip copy binary tools
ECHO     D_OFF         - disable debug info
ECHO     DEP_NO        - build without dependency files
ECHO     CLEAN         - clean objects
ECHO ------------------------------------------------------------------
goto :eof
:END

:: Calculate duration

set ENDTIME=%TIME%
:: Change formatting for the start and end times
for /F "tokens=1-4 delims=:.," %%a in ("%STARTTIME%") do (
set /A "start=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)

for /F "tokens=1-4 delims=:.," %%a in ("%ENDTIME%") do (
set /A "end=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)

:: Calculate the elapsed time by subtracting values
set /A elapsed=end-start

:: Format the results for output
set /A hh=elapsed/(60*60*100), rest=elapsed%%(60*60*100), mm=rest/(60*100), rest%%=60*100, ss=rest/100, cc=rest%%100
if %hh% lss 10 set hh=0%hh%
if %mm% lss 10 set mm=0%mm%
if %ss% lss 10 set ss=0%ss%
if %cc% lss 10 set cc=0%cc%

set DURATION=%hh%:%mm%:%ss%,%cc%

echo          ---build_cpss.bat------------
echo Start    : %STARTTIME%
echo Finish   : %ENDTIME%
echo          ---------------
echo Duration : %DURATION% 

@echo ***************************
if -%success%- == -YES- (
@echo ****    SUCCESS        ****
) else (
@echo ****       FAIL        ****
)
@echo ***************************
 
:EOF

