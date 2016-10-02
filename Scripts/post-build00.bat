@echo off
REM ********************************************************************
REM <license>
REM Copyright (c) 2013, Omar Francisco
REM All rights reserved.
REM 
REM Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
REM 
REM Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
REM Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
REM Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
REM THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
REM </license>

REM <filehistory>
REM 08/24/2012 - Omar Francisco - Created
REM 02/27/2013 - Omar Francisco - Modified to execute pre-build99.bat to pre-build00.bat in reverse order
REM </filehistory>
REM ********************************************************************

REM ********************************************************************
REM <documentation>
REM postbuild.bat excutes batch file with the names post-buildxx.bat in reverse order.  XX is a number
REM between 99 and 00.  This scheme supports up to 100 scripts to run during the pre-build event.
REM
REM List of supported parameters
REM %1 = "$(MSBuildProjectName)" - Project name in quotes in order to support names with spaces
REM %2 = "$(OutputFileName)$(OutputFileExtension)"  - Output file name in quotes in order to support names with spaces
REM %3 = $(avrdevice) - The AVR microcontroller
REM %4 = $(Configuration) - Release | Debug
REM %5 = "$(MSBuildProjectFile)" - Project file name in quotes in order to uspport names with spaces 
REM %6 = "$(SolutionDir)" - Solution directory in quotes in order to support names with spaces
REM %7 = "$(MSBuildProjectDirectory)\" - Directory where build is located in quotes in order to support names with spaces
REM %8 = future1
REM %9 = future2
REM To add additional parameters go to project/properties build events. The same parameters have to be copied for both release and debug configurations.
REM
REM </documenation>
REM ********************************************************************

REM Copy parameters
set P1=%1
set P2=%2
set P3=%3
set P4=%4
set P5=%5
set P6=%6
set P7=%7
set P8=%8
set P9=%9

REM calling sequence
call :SETUP
call :DEBUG
call :MAIN
goto :END

:SETUP
REM ********************************************************************
REM script initialization
REM set BUILDTRACKER="C:\Projects\avrtestrunner\source\TestTracker\bin\Debug\TestTracker.exe"
set DEBUG_FLAG="OFF"

REM Change to the directory where the script file is running
cd /d %~dp0
goto :END
REM ********************************************************************


:DEBUG
REM ********************************************************************
if %DEBUG_FLAG%=="ON" (
echo ****************************************
echo DEBUG MODE ON - File:%~n0%~x0
echo ****************************************
echo Build Tracker: %BUILDTRACKER%
echo MSBuildProjectName: %P1%
echo OutputFileName.OutputFileExtension: %P2%
echo avrdevice: %P3%
echo Configuration: %P4%
echo MSBuildProjectFile: %P5%
echo SolutionDir: %P6%
echo MSBuildProjectDirectory: %P7%
echo Project Directory: %P8%
echo Future1: %P9%
echo Current Directory: %CD%
echo ****************************************
)
goto :END
REM ********************************************************************

:MAIN
REM ********************************************************************
set START_COMPILE_COUNTER=startcompilecounter.txt
set END_COMPILE_COUNTER=endcompilecounter.txt
set DEPLOY_SUCCESS_COUNTER=deploy-success-counter.txt
set DEPLOY_FAIL_COUNTER=deploy-fail-counter.txt
set BUILD_LOG_FILE=buildlog.txt

if exist %START_COMPILE_COUNTER% (set /p COMPILE_ATTEMPTS=<%START_COMPILE_COUNTER%) else (set COMPILE_ATTEMPTS=0)
if exist %END_COMPILE_COUNTER% (set /p COMPILE_SUCCESS=<%END_COMPILE_COUNTER%) else (set COMPILE_SUCCESS=0)
if exist %DEPLOY_SUCCESS_COUNTER% (set /p DEPLOYMENTS=<%DEPLOY_SUCCESS_COUNTER%) else (set DEPLOYMENTS=0)
if exist %DEPLOY_FAIL_COUNTER% (set /p DEPLOYMENTS_FAILS=<%DEPLOY_FAIL_COUNTER%) else (set DEPLOYMENTS_FAILS=0)


set /a COMPILE_FAIL=%COMPILE_ATTEMPTS%-%COMPILE_SUCCESS%

REM record log entry
set /a COUNTER=%COMPILE_ATTEMPTS%-1
echo %COUNTER%,%DATE:~-4%%DATE:~4,2%%DATE:~7,2%-%TIME:~0,2%:%TIME:~3,2%:%TIME:~6,2%,POSTBUILD>>%BUILD_LOG_FILE%

echo.
echo ******************[KEY PERFORMANCE INDICATORS]*********************
echo Compile Attempts: %COMPILE_ATTEMPTS%
echo Compile Success: %COMPILE_SUCCESS%
echo Compile Fail: %COMPILE_FAIL%
echo Device Deployments: %DEPLOYMENTS%
echo Deployment Failures: %DEPLOYMENTS_FAILS%
echo *******************************************************************




REM %BUILDTRACKER% %P1%
goto :END
REM ********************************************************************

:END

