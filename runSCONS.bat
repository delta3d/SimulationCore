:: Turn off command printing
@echo off

:: First, NULL out the current path to avoid any possible chance 
:: of conflicting dlls
set PATH=""

:: Create some useful environment variables
:: Please note that your directories will probably differ from these
:: If so, please feel free to edit these paths as appropriate

:: Delta3D
:: This path is the default install path for the Delta3D Self Extracting Installer
if not defined DELTA_ROOT set DELTA_ROOT=C:\Documents and Settings\johnson\Desktop\dvte_branches\delta3d
if not defined DELTA_INC set DELTA_INC=%DELTA_ROOT%\inc;%DELTA_ROOT%\ext\inc;%DELTA_ROOT%\ext\inc\CEGUI
if not defined DELTA_LIB set DELTA_LIB=%DELTA_ROOT%\lib;%DELTA_ROOT%\ext\lib
if not defined DELTA_DATA set DELTA_DATE=%DELTA_ROOT%\data

set DELTA_PATH=%DELTA_ROOT%\bin
set DELTA_EXT_PATH=%DELTA_ROOT%\ext\bin

:: Simulation Core
if not defined SIMCORE_ROOT set SIMCORE_ROOT=C:\Documents and Settings\johnson\Desktop\dvte_branches\SimulationCore
if not defined SIMCORE_INC set SIMCORE_INC=%SIM_CORE_ROOT%\include
if not defined SIMCORE_LIB set SIMCORE_LIB=%SIM_CORE_ROOT%\lib

set SIM_CORE_PATH=%SIM_CORE_ROOT%\bin

:: dtAgeiaPhysX
if not defined DT_AGEIA_ROOT set DT_AGEIA_ROOT=C:\Documents and Settings\johnson\Desktop\dvte_branches\dtAgeiaPhysX
if not defined DT_AGEIA_INC set DT_AGEIA_INC=%DT_AGEIA_ROOT%\include
if not defined DT_AGEIA_LIB set DT_AGEIA_LIB=%DT_AGEIA_ROOT%\lib

set DT_AGEIA_PATH=%DT_AGEIA_ROOT%\bin

:: Humvee App
if not defined DVTE_ROOT set DVTE_ROOT=C:\Documents and Settings\johnson\Desktop\dvte_branches\dvte

set DVTE_PATH=%DVTE_ROOT%\bin
set DVTE_EXT_PATH=%DVTE_ROOT%\ext\bin\win32

:: Ageia
:: This is the default Ageia install path
if not defined AGEIA_ROOT set AGEIA_ROOT=C:\Program Files\AGEIA Technologies\AGEIA PhysX SDK\v2.7.0
if not defined AGEIA_INC set AGEIA_INC=%AGEIA_ROOT%\SDKs\Cooking\include;%AGEIA_ROOT%\SDKs\Foundation\include;%AGEIA_ROOT%\SDKs\NxCharacter\include;%AGEIA_ROOT%\SDKs\NxExtensions\include;%AGEIA_ROOT%\SDKs\Physics\include;%AGEIA_ROOT%\SDKs\PhysXLoader\include;%DT_AGEIA_INC%;
if not defined AGEIA_LIB set AGEIA_LIB=%AGEIA_ROOT%\SDKs\lib\win32;%DT_AGEIA_LIB%;

set AGEIA_PATH=%AGEIA_ROOT%\Bin\win32

set INCLUDE=%INCLUDE%;%DVTE_ROOT%\include;%DVTE_ROOT%\ext\include\win32;%DT_AGEIA_INC%
set LIB=%LIB%;%DVTE_ROOT%\lib;%DVTE_ROOT%\ext\lib\win32;%AGEIA_LIB%

:: Qt
if not defined QTDIR set QTDIR=C:\Qt\4.3.0

set QT_PATH=%QTDIR%\bin

:: The RTI
if not defined RTI_HOME set RTI_HOME=C:\Curtiss\Projects\DVTE\rti

set RTI_PATH=%RTI_HOME%\lib\winnt_vc++-8.0

:: Python (for the Stealth Viewer)
:: This is the default Python installation directory
if not defined PYTHON_ROOT set PYTHON_ROOT=C:\Program Files\Python25

set PYTHON_PATH=%PYTHON_ROOT%

:: Basic Windows dirs and exes
set WIN_DIR=C:\WINDOWS\system32

:: PATH
set PATH=%DELTA_PATH%;%DELTA_EXT_PATH%;%SIMCORE_PATH%;%DT_AGEIA_PATH%;%DVTE_PATH%;%DVTE_EXT_PATH%;%AGEIA_PATH%;%QT_PATH%;%RTI_PATH%;%PYTHON_PATH%;%WIN_DIR%

path

:: Configure and launch SCONS
if "%2%" == "clean" goto clean

call scons mode="%1%" ageia=1 -j3
goto end

:clean
call scons mode="%1%" -c
goto end

:end

