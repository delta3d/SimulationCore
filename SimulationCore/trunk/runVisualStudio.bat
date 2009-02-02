::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: 
:: runVisualStudio.bat - This file is provided to make it easier to 
:: correctly configure and compile a Delta3D application that uses 
:: advanced capabilties such as the SimViewerCore, dtAgeiaPhysX, and others.
::
:: To use this file, set the paths below to reflect your own configuration.
:: Then, run this file to start Visual Studio. ALWAYS use this file
:: to start Visual Studio to compile the various projects including Delta3D
:: itself and you will avoid numerous issues. 
::
:: Note - once Visual Studio is running, you may close the command window. 
::
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:: Simulation Core - runVisualStudio.bat - Using 'The MIT License'
:: Copyright (C) 2007-2009, Alion Science and Technology Corporation.
::
:: Permission is hereby granted, free of charge, to any person obtaining a copy
:: of this software and associated documentation files (the "Software"), to deal
:: in the Software without restriction, including without limitation the rights
:: to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
:: copies of the Software, and to permit persons to whom the Software is
:: furnished to do so, subject to the following conditions:
::
:: The above copyright notice and this permission notice shall be included in
:: all copies or substantial portions of the Software.
::
:: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
:: IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
:: FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
:: AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
:: LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
:: OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
:: THE SOFTWARE.
:: 
:: This software was developed by Alion Science and Technology Corporation
:: under circumstances in which the U. S. Government may have rights in the software.
::
:: @author Curtiss Murphy, Eddie Johnson, Allen Danklefsen
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


:: Turn off command printing
@echo off
@echo Configuring and launching Visual Studio 2005.  You may close this window
@echo   once Visual Studio is running.
@echo ... 

:: First, NULL out the current path to avoid any possible chance 
:: of conflicting dlls
set PATH=""

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Primary paths - Edit these to reflect your directory structure.
:: Note that some of these directories may not be needed by each
:: specific project. However, this configuration will support numerous 
:: projects including Delta3D, SimViewerCore, dtAgeiaPhysX, and others. 
::
:: Core repositories - Delta3D, SimCore, dtAgeiaPhysX, 
set DELTA_ROOT=C:\Curtiss\Projects\Delta3D\Delta3D
set SIM_CORE_ROOT=C:\Curtiss\Projects\Delta3D\SimulationCore
set DTPHYSX_ROOT=C:\Curtiss\Projects\Delta3D\dtAgeiaPhysX
set DRIVERDEMO_DIR=C:\Curtiss\Projects\Delta3D\SimulationCore\demos\DriverDemo
:: Primary Dependencies - Qt, PhysX, HLA RTI, Phython, Windows...
set QTDIR=C:\Qt\4.4.3
set PHYSX_ROOT=C:\Program Files\AGEIA Technologies\AGEIA PhysX SDK\v2.7.0
set RTI_HOME=C:\Curtiss\Projects\Delta3D\rti
set PYTHON_ROOT=C:\Program Files\Python25
set WIN_DIR=C:\WINDOWS\system32
:: Note - DTPHYSX vars are for the Delta3D physx integration code. Whereas,
:: PHYSX vars are for the actual PhysX libraries provided by NVidia.
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Custom project environment variables - Edit as needed
:: Some of these entries may not make sense for your particular project but 
:: are provided because some in the community use them. Feel free to 
:: remove projects that are unrelated to your specific needs

:: Custom Project paths go here.
set DCOS_ROOT=C:\Curtiss\Projects\MTS_DCOS\trunk
set DVTE_ROOT=C:\Curtiss\Projects\DVTE\DVTE_SimViewer
set DCSIM_DIR=c:\Curtiss\Projects\BBN\DCSim

:: Project Settings
set DCOS_PATH=%DCOS_ROOT%\bin;%DCOS_ROOT%\ext\bin
set DVTE_PATH=%DVTE_ROOT%\bin;%DVTE_ROOT%\ext\bin\win32
set DCSIM_PATH=%DCSIM_DIR%\bin

:: Final Custom Project path - used below
set CUSTOM_PROJECTS_PATH=%DVTE_PATH%;%DCOS_PATH%;%DRIVERDEMO_DIR%\bin;%DCSIM_PATH%;
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Delta3D Environment Variables go here - No need to edit this section
:: 
:: This path is the default install path for the Delta3D Self Extracting Installer
set DELTA_PATH=%DELTA_ROOT%\bin
set DELTA_EXT_PATH=%DELTA_ROOT%\ext\bin
:: Simulation Core
set SIM_CORE_PATH=%SIM_CORE_ROOT%\bin

:: PhysX 
:: dtAgeiaPhysX - The Delta3D library
set DTPHYSX_PATH=%DTPHYSX_ROOT%\bin
:: NVidia PhysX - assumes default install and setup path
set PHYSX_PATH=%PHYSX_ROOT%\Bin\win32

:: Qt
set QT_PATH=%QTDIR%\bin
 
:: The RTI
set RTI_PATH=%RTI_HOME%\lib\winnt_vc++-8.0

:: Python
set PYTHON_PATH=%PYTHON_ROOT%

:: PATH - The final combined path of everything.
set PATH=%DELTA_PATH%;%DELTA_EXT_PATH%;%SIM_CORE_PATH%;%DTPHYSX_PATH%;%PHYSX_PATH%;%QT_PATH%;%RTI_PATH%;%PYTHON_PATH%;%CUSTOM_PROJECTS_PATH%;%WIN_DIR%;

path
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Configure and launch Visual Studio - Do not edit
::
:: Call this to set up the SDK paths so we can find include files like "stdio.h"
call C:\"Program Files"\"Microsoft Visual Studio 8"\VC\bin\vcvars32.bat
call C:\"Program Files"\"Microsoft Visual Studio 8"\SDK\v2.0\Bin\sdkvars.bat
:: Launch the IDE
call C:\"Program Files"\"Microsoft Visual Studio 8"\Common7\IDE\devenv.exe
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



