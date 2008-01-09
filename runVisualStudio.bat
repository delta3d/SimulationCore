:: 
:: runVisualStudio.bat - This file is provided to make it easier to 
:: correctly configure and compile a Delta3D application that uses 
:: advanced capabilties such as the SimViewerCore, DVTE, and dtAgeiaPhysX.
::
:: To use this file, set the paths below to reflect your own configuration.
:: Then, run this file to start Visual Studio 2005. ALWAYS use this file
:: to start Visual Studio to compile the various projects including Delta3D
:: itself and you will avoid numerous issues. 
::
:: Note - once Visual Studio is running, you may close the command window. 
::
:: Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
:: 
::   Alion Science and Technology Corporation
::   5365 Robin Hood Road
::   Norfolk, VA 23513
::   (757) 857-5670, www.alionscience.com
::
:: This software was developed by Alion Science and Technology Corporation
:: under circumstances in which the U. S. Government may have rights in the software.
:: 
::


:: Turn off command printing
@echo off
@echo Configuring and launching Visual Studio 2005.  You may close this window
@echo   once Visual Studio is running.
@echo ...  

:: First, NULL out the current path to avoid any possible chance 
:: of conflicting dlls
set PATH=""

:: Primary paths. Change these to match your own directories
:: Note that some of these directories may not be needed by each
:: specific project. However, this configuration will support numerous 
:: projects including Delta3D, SimViewerCore, MTS BS21, dtAgeiaPhysX, 
:: DVTE, and others. 
::
:: Please set ALL variables as accurately as possible
:: and ALWAYS run this .bat file to start Visual Studio.  
set DELTA_ROOT=C:\Curtiss\Projects\Delta3D\Delta3D
set SIM_CORE_ROOT=C:\Curtiss\Projects\DVTE\SimulationCore
set DCOS_ROOT=C:\Curtiss\Projects\MTS_DCOS\trunk
set AGEIA_ROOT=C:\Program Files\AGEIA Technologies\AGEIA PhysX SDK\v2.7.0
set DT_AGEIA_ROOT=C:\Curtiss\Projects\DVTE\dtAgeiaPhysX
set RTI_HOME=C:\Curtiss\Projects\DVTE\rti
set QTDIR=C:\Qt\4.3.0
set DVTE_ROOT=C:\Curtiss\Projects\DVTE\DVTE_SimViewer
set PYTHON_ROOT=C:\Program Files\Python25
set WIN_DIR=C:\WINDOWS\system32


:: Delta3D
:: This path is the default install path for the Delta3D Self Extracting Installer
set DELTA_INC=%DELTA_ROOT%\inc;%DELTA_ROOT%\ext\inc;%DELTA_ROOT%\ext\inc\CEGUI
set DELTA_LIB=%DELTA_ROOT%\lib;%DELTA_ROOT%\ext\lib
set DELTA_DATA=%DELTA_ROOT%\data
set DELTA_PATH=%DELTA_ROOT%\bin
set DELTA_EXT_PATH=%DELTA_ROOT%\ext\bin

:: Simulation Core
set SIM_CORE_INC=%SIM_CORE_ROOT%\include
set SIM_CORE_LIB=%SIM_CORE_ROOT%\lib
set SIM_CORE_PATH=%SIM_CORE_ROOT%\bin

:: dtAgeiaPhysX
set DT_AGEIA_INC=%DT_AGEIA_ROOT%\include
set DT_AGEIA_LIB=%DT_AGEIA_ROOT%\lib
set DT_AGEIA_PATH=%DT_AGEIA_ROOT%\bin

:: Humvee App
set DVTE_INC=%DVTE_ROOT%\include;%DVTE_ROOT%\ext\include\win32
set DVTE_LIB=%DVTE_ROOT%\lib;%DVTE_ROOT%\ext\lib\win32
set DVTE_PATH=%DVTE_ROOT%\bin
set DVTE_EXT_PATH=%DVTE_ROOT%\ext\bin\win32

set EXTERNAL_PATH=%DVTE_ROOT%\ext\bin\win32

:: Includes for osgEphemiris moved to SimCore
set EXTERNAL_INC=%SIM_CORE_ROOT%\ext\include\win32;%DVTE_ROOT%\ext\include\win32
set EXTERNAL_LIB=%DVTE_ROOT%\ext\lib\win32

:: Ageia
:: This is the default Ageia install path
set AGEIA_INC=%AGEIA_ROOT%\SDKs\Cooking\include;%AGEIA_ROOT%\SDKs\Foundation\include;%AGEIA_ROOT%\SDKs\NxCharacter\include;%AGEIA_ROOT%\SDKs\NxExtensions\include;%AGEIA_ROOT%\SDKs\Physics\include;%AGEIA_ROOT%\SDKs\PhysXLoader\include;%DT_AGEIA_INC%;
set AGEIA_LIB=%AGEIA_ROOT%\SDKs\lib\win32;%DT_AGEIA_LIB%;
set AGEIA_PATH=%AGEIA_ROOT%\Bin\win32

:: Qt
set QT_PATH=%QTDIR%\bin

:: The RTI
set RTI_PATH=%RTI_HOME%\lib\winnt_vc++-8.0

:: Python (for the Stealth Viewer)
:: This is the default Python installation directory
set PYTHON_PATH=%PYTHON_ROOT%

:: PATH
set PATH=%DELTA_PATH%;%DELTA_EXT_PATH%;%SIM_CORE_PATH%;%DT_AGEIA_PATH%;%DVTE_PATH%;%DVTE_EXT_PATH%;%AGEIA_PATH%;%QT_PATH%;%RTI_PATH%;%PYTHON_PATH%;%WIN_DIR%

path

:: Configure and launch Visual Studio

:: Call this to set up the SDK paths so we can find include files like "stdio.h"
call C:\"Program Files"\"Microsoft Visual Studio 8"\VC\bin\vcvars32.bat
call C:\"Program Files"\"Microsoft Visual Studio 8"\SDK\v2.0\Bin\sdkvars.bat

:: Launch the IDE
call C:\"Program Files"\"Microsoft Visual Studio 8"\Common7\IDE\devenv.exe
::call GameStartd.exe HMMWVGroundSim --enableLogging 0 --enablePlayback 0 --fedMappingFileResource "Federations/DVTE-VISIT/VisitMapping.xml" --federationExecutionName jojo --fedFileName Federations/v3_dvte:v3_dvte.fed --startX 50000 --startY 30000 --startZ 600 --abletoSwitchWeapons 0 --simulationRole 0 --projectPath "ProjectAssets" --mapName "29 Palms Large" 

