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
REM 02/13/2013 - Omar Francisco - Created
REM </filehistory>
REM ********************************************************************
SETLOCAL EnableDelayedExpansion

REM ********************************************************************
REM <documentation>
REM post-build98.bat Increments the build counter keeping track of number of build attempts.  This counter goes up
REM independently of the build result.
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
set DEBUG_FLAG="OFF"
set BUILD_COUNTER=endcompilecounter.txt

goto :END
REM ********************************************************************

:DEBUG
REM ********************************************************************
if %DEBUG_FLAG%=="ON" (
echo ****************************************
echo DEBUG MODE ON - File:%~n0%~x0
echo ****************************************
echo Build Counter File: %BUILD_COUNTER%
echo Project File: %PROJECT_FILE%
echo Output File: %OUTPUT_FILE%
echo Device File: %DEVICE_FILE%
echo Config File: %CONFIG_FILE%
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
REM Create buildcounter file if it does not exist
if NOT EXIST %BUILD_COUNTER% (
echo 0> %BUILD_COUNTER%
)

set /p COUNTER= <%BUILD_COUNTER%
set /a COUNTER+=1
echo !COUNTER!> %BUILD_COUNTER%
rem echo Sucessfull Build !COUNTER!
goto :END
REM ********************************************************************



:END

