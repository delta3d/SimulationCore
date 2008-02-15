@echo off

:: Import environment variables for Visual Studio.

call vcvars32.bat

:: Tell qmake to build the Makefile which will be used to generate the sources.
call %QTDIR%\bin\qmake -makefile stealthViewer.pro

:: Now, since we will be actually building the application using Visual Studio,
:: we only use the Makefile to generate the MOC sources.  Since these are added
:: to the VC++ project, they will get included in the final build.

if "%1" == "debug" call nmake -f Makefile.debug compiler_uic_make_all mocables
if "%1" == "release" call nmake -f Makefile.release compiler_uic_make_all mocables