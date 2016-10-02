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
REM 02/23/2013 - Omar Francisco - Created
REM </filehistory>
REM ********************************************************************

REM ********************************************************************
REM <documentation>
REM pre-build98.bat Generates the file builddate.cpp with the current timestamp to be included in the
REM AVR code.
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
REM Change to the directory where the script file is running
set DEBUG_FLAG="OFF"
cd /d %~dp0

REM All paths must be relative to the script directory
set DESTINATION=..\Utility\BuildDate.cpp

set VARNAME=BUILD_DATE
set FUNCTION_NAME=GetBuildDate

REM Capture Date
for /f "tokens=2-4 delims=/ " %%a in ('date /T') do set year=%%c
for /f "tokens=2-4 delims=/ " %%a in ('date /T') do set month=%%a
for /f "tokens=2-4 delims=/ " %%a in ('date /T') do set day=%%b
set TODAY=%year%-%month%-%day%

REM Capture time
for /f "tokens=1 delims=: " %%h in ('time /T') do set hour=%%h
for /f "tokens=2 delims=: " %%m in ('time /T') do set minutes=%%m
for /f "tokens=3 delims=: " %%a in ('time /T') do set ampm=%%a
set NOW=%hour%-%minutes%-%ampm%

REM Build time stamp
set TIME_STAMP=%TODAY% %NOW%
goto :END
REM ********************************************************************


:DEBUG
REM ********************************************************************
if %DEBUG_FLAG%=="ON" (
echo ****************************************
echo DEBUG MODE ON - File:%~n0%~x0
echo ****************************************
echo Time Stamp: %TIME_STAMP%
echo MSBuildProjectName: %P1%
echo OutputFileName.OutputFileExtension: %P2%
echo avrdevice: %P3%
echo Configuration: %P4%
echo MSBuildProjectFile: %P5%
echo SolutionDir: %P6%
echo MSBuildProjectDirectory: %P7%
echo Future1: %P8%
echo Future2: %P9%
echo Current Directory: %CD%
echo ****************************************
)
goto :END
REM ********************************************************************


:MAIN
REM ********************************************************************
REM Generate builddate.cpp
echo #ifndef BUILDINFO_H_ > %DESTINATION%
echo #include "buildinfo.h" >> %DESTINATION%
echo #endif >> %DESTINATION%
echo. >>%DESTINATION%
echo /************************************************************  >> %DESTINATION%
echo DO NOT MODIFY >> %DESTINATION%
echo Automatically Generated On %TIME_STAMP% by %~n0 >> %DESTINATION%
echo *************************************************************/ >> %DESTINATION%
echo. >>%DESTINATION%
echo #if defined(BUILDINFO_RAM) >> %DESTINATION%
echo static const char* %VARNAME% = "%TIME_STAMP%"; >> %DESTINATION%
echo #elif defined(BUILDINFO_EEMEM) >> %DESTINATION%
echo static const char %VARNAME%[HeaderMsgSize] EEMEM = "%TIME_STAMP%"; >> %DESTINATION%
echo #elif defined(BUILDINFO_PROGMEM) >> %DESTINATION%
echo static const char %VARNAME%[] PROGMEM = "%TIME_STAMP%"; >> %DESTINATION%
echo #endif >> %DESTINATION%
echo. >>%DESTINATION%
echo /* Return the header message */  >> %DESTINATION%
echo void %FUNCTION_NAME%( void *buffer, size_t bufferSize )  >> %DESTINATION%
echo {   >> %DESTINATION%
echo #if defined(BUILDINFO_RAM) >> %DESTINATION%
echo 	memcpy( buffer, %VARNAME%, bufferSize ); >> %DESTINATION%
echo #elif defined(BUILDINFO_EEMEM) >> %DESTINATION%
echo    eeprom_read_block(buffer,%VARNAME%,bufferSize); >> %DESTINATION%
echo #elif defined(BUILDINFO_PROGMEM) >> %DESTINATION%
echo    memcpy_P(buffer,%VARNAME%,bufferSize); >> %DESTINATION%
echo #endif    >> %DESTINATION%
echo }   >> %DESTINATION%
goto :END
REM ********************************************************************

:END