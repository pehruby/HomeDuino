Setlocal EnableDelayedExpansion
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
REM 05/11/2013 - Omar Francisco - Modified to use boards.xml to drive transfering program to board
REM </filehistory>
REM ********************************************************************

REM ********************************************************************
REM <documentation>
REM local-deploy98.bat deploys the binary file to the target device
REM
REM List of supported parameters
REM %1 = Solution Directory
REM %2 = Project Directory
REM To add additional parameters go to project/properties build events. 
REM The same parameters have to be copied for both release and debug configurations.
REM
REM </documenation>
REM ********************************************************************

REM Copy parameters
set P1=%1
set P2=%2

REM Change to the directory where the script file is running
cd /d %~dp0

REM COPY paramaeters and drop quotes
SET SOLUTION_DIR=%~1%
SET PROJECT_DIR=%~2%
SET LOCALBOARDS=targetboards.xml
SET XMLTOOL="%ATE_HOME%\TOOLS\XML.EXE"
set GLOBALBOARDS="%ATE_HOME%\boards.xml"
set NAMESPACE="http://schemas.microsoft.com/developer/msbuild/2003"
set TEMPDATA=tempdata.txt

REM calling sequence
call :SETUP
call :DEBUG
call :MAIN
goto :END

:SETUP
REM ********************************************************************
REM script initialization

REM Allowed values "ON"|"OFF"
set DEBUG_FLAG="OFF"

if NOT EXIST %LOCALBOARDS% (
  echo Local board definition "%LOCALBOARDS%" was not FOUND - Script will be aboarted
  set ABORT_SCRIPT="TRUE"
  goto :END
)

if NOT EXIST %GLOBALBOARDS% (
  echo Global board definition "%GLOBALBOARDS%" was not FOUND - Script will be aboarted
  set ABORT_SCRIPT="TRUE"
  goto :END
)

if NOT EXIST %XMLTOOL% (
  echo XML tool "%XMLTOOL%" was not FOUND - Script will be aboarted
  set ABORT_SCRIPT="TRUE"
  goto :END
)

REM Get target board 
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:CurrentBoard/x:Name" %LOCALBOARDS% > %TEMPDATA%
set /p TARGET_BOARD=< %TEMPDATA%

REM Get target programmer
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:CurrentBoard/x:Programmer" %LOCALBOARDS% > %TEMPDATA%
set /p TARGET_PROGRAMMER=< %TEMPDATA%

REM Get local misc parameters for program transfer
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:CurrentBoard/x:MiscParams" %LOCALBOARDS% > %TEMPDATA%
set /p LOCAL_MISC_PARAMS=< %TEMPDATA%

REM Get local misc parameters for program transfer
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:CurrentBoard/x:OverrideVerbosity" %LOCALBOARDS% > %TEMPDATA%
set /p OVERRIDE_VERBOSITY=< %TEMPDATA%

REM Get local misc parameters for library directory
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:CurrentBoard/x:FrequencyDirectory" %LOCALBOARDS% > %TEMPDATA%
set /p LIB_DIR_FROM_FREQUENCY=< %TEMPDATA%


REM Get Board Data Port - Used for LEONARDO board
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:BoardList[@Include='%TARGET_BOARD%']/x:DataPort" %GLOBALBOARDS% > tempdata.txt
set /p BOARD_DATA_COMPORT=< tempdata.txt


REM Get Target communication port
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:Programmer[@Include='%TARGET_PROGRAMMER%']/x:ComPort" %GLOBALBOARDS% > tempdata.txt
set /p PROGRAMMER_COMPORT=< tempdata.txt

REM Get communication port speed
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:Programmer[@Include='%TARGET_PROGRAMMER%']/x:ComPortSpeed" %GLOBALBOARDS% > tempdata.txt
set /p PROGRAMMER_COMPORT_SPEED=< tempdata.txt

REM Get communication port speed
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:Programmer[@Include='%TARGET_PROGRAMMER%']/x:ProgTransfer" %GLOBALBOARDS% > tempdata.txt
set /p PROGRAM_TRANSFER=< tempdata.txt

REM Get communication protocol
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:Programmer[@Include='%TARGET_PROGRAMMER%']/x:Protocol" %GLOBALBOARDS% > tempdata.txt
set /p PROGRAMMER_PROTOCOL=< tempdata.txt

REM Get EEPROM transfer support
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:Programmer[@Include='%TARGET_PROGRAMMER%']/x:SupportEEPROMTransfer" %GLOBALBOARDS% > tempdata.txt
set /p PROGRAMMER_EEPROM_SUPPORT=< tempdata.txt

REM Get transfer verbosity
IF "%OVERRIDE_VERBOSITY%" == "" ( 
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:Programmer[@Include='%TARGET_PROGRAMMER%']/x:Verbosity" %GLOBALBOARDS% > tempdata.txt
set /p PROGRAM_TRANSFER_VERBOSITY=< tempdata.txt
) else (
echo OVERRIDING program transfer verbosity witn %OVERRIDE_VERBOSITY%
set PROGRAM_TRANSFER_VERBOSITY=%OVERRIDE_VERBOSITY%
)

REM Get miscelaneous parameters for program transfer
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:Programmer[@Include='%TARGET_PROGRAMMER%']/x:MiscParams" %GLOBALBOARDS% > tempdata.txt
set /p PROGRAM_TRANSFER_PARAMS=< tempdata.txt

REM Get communication protocol
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:ProgTransfer[@Include='%PROGRAM_TRANSFER%']/x:Path" %GLOBALBOARDS% > tempdata.txt
set /p PROGRAM_TRANSFER_PATH=< tempdata.txt

REM Get communication protocol
%XMLTOOL% sel -N x=%NAMESPACE% -t -v "/x:Project/x:ItemGroup/x:ProgTransfer[@Include='%PROGRAM_TRANSFER%']/x:Config" %GLOBALBOARDS% > tempdata.txt
set /p PROGRAM_TRANSFER_CONFIG=< tempdata.txt


REM File containing the name of the visual studio project - created during preprocessing
set PROJECT_FILE="%PROJECT_DIR%scripts\project.txt"

REM File containing the name of the program that was compiled - created during preprocessing
set OUTPUT_FILE="%PROJECT_DIR%scripts\output.txt"

REM File containing the name of the targeted AVR device - created during preprocessing
set DEVICE_FILE="%PROJECT_DIR%scripts\device.txt"

REM Visual Studio configuration (Release/Debug) - created during preprocessing
set CONFIG_FILE="%PROJECT_DIR%scripts\config.txt"

REM Get the project name, binary file, device and configuration - there is no error checking
set /p PROJECT_NAME= <%PROJECT_FILE%
set /p BINARY_FILE= <%OUTPUT_FILE%
set BINARY_FILE=%BINARY_FILE:"=%
set /p AVRPROCESSOR= <%DEVICE_FILE%
set /p CONFIGURATION= <%CONFIG_FILE%

REM File keeping track of how many sucessfull builds have been done to the project
set BUILD_COUNTER_FILE=endcompilecounter.txt
set BUILD_COUNTER=0
if exist %BUILD_COUNTER_FILE% ( set /p BUILD_COUNTER= <%BUILD_COUNTER_FILE% )

REM Header path for Arduino board
set ARDUINO_LIBS_HEADER_DIR=%ATE_HOME%\Boards\%TARGET_BOARD%\Headers

REM Full path of binary file
SET ARDUINO_LIBS_DIR=%ATE_HOME%\Boards\%TARGET_BOARD%\Libs\%LIB_DIR_FROM_FREQUENCY%

REM Get root output file name by droping the .elf from the extension
SET ROOT_OUTPUT_FILE=%BINARY_FILE%
SET ROOT_OUTPUT_FILE=%ROOT_OUTPUT_FILE:.elf=%

REM Get hex file name and path
SET HEX_BINARY_FILE=%ROOT_OUTPUT_FILE%.hex
SET HEX_BINARY_FILE_PATH="%PROJECT_DIR%%CONFIGURATION%\%HEX_BINARY_FILE%"


REM Strategy to Resolve availabilit of EEPROM_FILE
REM The eeprom file will always exist.  When its content is ":00000001FF" the EEPROM file 
REM does not need to be deployed.
set INCLUDE_EEPROM_FILE="FALSE"
SET EEPROM_FILE=%ROOT_OUTPUT_FILE%.eep
SET EEPROM_FILE_PATH="%PROJECT_DIR%%CONFIGURATION%\%EEPROM_FILE%"
if exist %EEPROM_FILE_PATH% (set /p EEPROM_CONTENT= <%EEPROM_FILE_PATH%)
if "%EEPROM_CONTENT%" == "" goto :NO_EEPROM_FILE
if "%EEPROM_CONTENT%" == ":00000001FF" goto :NO_EEPROM_FILE
set INCLUDE_EEPROM_FILE="TRUE"
:NO_EEPROM_FILE

echo ----------------------------------------------------------
echo Deploying %PROJECT_NAME% - Build %BUILD_COUNTER% 

goto :END
REM ********************************************************************


:DEBUG
REM ********************************************************************
if %DEBUG_FLAG%=="ON" (
echo ****************************************
echo DEBUG MODE ON - File:%~n0%~x0
echo ****************************************
echo Solution Directory: "%SOLUTION_DIR%"
echo Project Directory: "%PROJECT_DIR%"
echo Target Board: %TARGET_BOARD%
echo Target Programmer: %TARGET_PROGRAMMER%
echo Com Port: %PROGRAMMER_COMPORT%
echo Com Port Speed: %PROGRAMMER_COMPORT_SPEED%
echo Allow EEPROM transfer:%PROGRAMMER_EEPROM_SUPPORT%
echo Prog Transfer: %PROGRAM_TRANSFER%
echo Transfer Protocol: %PROGRAMMER_PROTOCOL%
echo Prog Transfer Path: "%PROGRAM_TRANSFER_PATH%"
echo Prog Transfer Config: "%PROGRAM_TRANSFER_CONFIG%"
echo Prog Trasfer Verbosity: %PROGRAM_TRANSFER_VERBOSITY%
echo Misc Parameters: %PROGRAM_TRANSFER_PARAMS%
echo Local Misc Parameters: %LOCAL_MISC_PARAMS%
echo Template Express Home: "%ATE_HOME%"
echo Library Path: "%ARDUINO_LIBS_DIR%"
echo Header Path: "%ARDUINO_LIBS_HEADER_DIR%"
echo Configuration: %CONFIGURATION%
echo EEPROM File: %EEPROM_FILE_PATH%
echo EEPROM File Found: %INCLUDE_EEPROM_FILE%
echo Target Device: %AVRPROCESSOR%
echo Project Name:%PROJECT_NAME%
echo Output File: %BINARY_FILE%
echo Full Hex File Path: %HEX_BINARY_FILE_PATH%
echo.
echo.

if exist %PROJECT_FILE% (echo Input Project File: %PROJECT_FILE% [CHECKED]) else (echo Input Project File: %PROJECT_FILE% [NOT FOUND])
if exist %OUTPUT_FILE% (echo Input Output File: %OUTPUT_FILE% [CHECKED] ) else (echo Input Output File: %OUTPUT_FILE% [NOT FOUND])
if exist %DEVICE_FILE% (echo Input Device File: %DEVICE_FILE% [CHECKED]) else (echo Input Device File: %DEVICE_FILE% [NOT FOUND])
if exist %CONFIG_FILE% (echo Input Config File: %CONFIG_FILE% [CHECKED]) else (echo Input Config File: %CONFIG_FILE% [NOT FOUND])
if exist "%SOLUTION_DIR%" (echo Solution Directory:%SOLUTION_DIR% [CHECKED]) else (echo Solution Directory:%SOLUTION_DIR% [NOT FOUND])
if exist "%PROJECT_DIR%" (echo Project Directory:%PROJECT_DIR% [CHECKED]) else (echo Project Directory:%PROJECT_DIR% [NOT FOUND])

if exist "%PROGRAM_TRANSFER_PATH%" (echo Prog Transfer Path:"%PROGRAM_TRANSFER_PATH%" [CHECKED]) else (echo Program Transfer Path:"%PROGRAM_TRANSFER_PATH%" [NOT FOUND])
if exist "%PROGRAM_TRANSFER_CONFIG%" (echo Prog Transfer Path:"%PROGRAM_TRANSFER_CONFIG%" [CHECKED]) else (echo Program Transfer Path:"%PROGRAM_TRANSFER_CONFIG%" [NOT FOUND])
if exist %HEX_BINARY_FILE_PATH% (echo Binary File: %HEX_BINARY_FILE_PATH% [CHECKED]) else (echo Binary File: %HEX_BINARY_FILE_PATH% [NOT FOUND])
if exist %EEPROM_FILE_PATH% (echo EEPROM File: %EEPROM_FILE_PATH% [CHECKED]) else (echo EEPROM File: %EEPROM_FILE_PATH% [NOT FOUND])
if exist "%ARDUINO_LIBS_HEADER_DIR%" (echo Arduino Library Header Path:%ARDUINO_LIBS_HEADER_DIR% [CHECKED]) else (echo Arduino Library Header Path:%ARDUINO_LIBS_HEADER_DIR% [NOT FOUND])
if exist "%ARDUINO_LIBS_DIR%" (echo Arduino Library Path:%ARDUINO_LIBS_DIR% [CHECKED]) else (echo Arduino Library Path:%ARDUINO_LIBS_DIR% [NOT FOUND])

if NOT EXIST "%PROJECT_FILE%" (
  echo CANNOT Find project directory "%PROJECT_FILE%" - Aborting script
  goto :END
)

if NOT EXIST "%OUTPUT_FILE%" (
  echo CANNOT Find output file "%OUTPUT_FILE%" - Aborting script
  goto :END
)

if NOT EXIST "%DEVICE_FILE%" (
  echo CANNOT Find device file "%DEVICE_FILE%" - Aborting script
  goto :END
)


if NOT EXIST "%CONFIG_FILE%" (
  echo CANNOT Find config file "%CONFIG_FILE%" - Aborting script
  goto :END
)

if NOT EXIST "%SOLUTION_DIR%" (
  echo CANNOT Find solution directory "%SOLUTION_DIR%" - Aborting script
  goto :END
)

if NOT EXIST "%PROJECT_DIR%" (
  echo CANNOT Find project directory %PROJECT_DIR% - Aborting script
  goto :END
)

if NOT EXIST "%ARDUINO_LIBS_DIR%" (
  echo [WARNING] CANNOT Find project directory "%ARDUINO_LIBS_DIR%"
  rem goto :END
)

if NOT EXIST "%ARDUINO_LIBS_HEADER_DIR%" (
  echo [WARNING] CANNOT Find header directory "%ARDUINO_LIBS_HEADER_DIR%"
  rem goto :END
)

if NOT EXIST "%PROGRAM_TRANSFER_PATH%" (
  echo CANNOT Find Transfer Program "%PROGRAM_TRANSFER_PATH%" - Aborting script
  goto :END
)

if NOT EXIST "%HEX_BINARY_FILE_PATH%" (
  echo CANNOT Find Binary File path "%HEX_BINARY_FILE_PATH%" - Aborting script
  goto :END
)
echo ****************************************
)
goto :END
REM ********************************************************************

:MAIN
REM ********************************************************************

rem ABORT if something failed during setup
if "%ABORT_SCRIPT%"=="TRUE" goto :END

rem echo Programmer: %TARGET_PROGRAMMER% 
rem echo Protocol: %PROGRAMMER_PROTOCOL%
rem echo Port: %PROGRAMMER_COMPORT% 
rem echo EEPROM transfer:%PROGRAMMER_EEPROM_SUPPORT%
rem echo EEPROM File Found: %INCLUDE_EEPROM_FILE%
rem echo EEPROM File: %EEPROM_FILE_PATH%
rem echo Full Hex File Path: %HEX_BINARY_FILE_PATH%

REM branch to one of the supported program transfers
if "%PROGRAM_TRANSFER%"=="avrdude" goto :PROGRAM_WITH_AVRDUDE
if "%PROGRAM_TRANSFER%"=="atprogram" goto :PROGRAM_WITH_ATPROGRAM 
if "%PROGRAM_TRANSFER%"=="filecopy" goto :PROGRAM_WITH_FILE_COPY 
echo [ERROR] Invalid program transfer detected [%PROGRAMMER%] - Aborting Script
goto :END

:PROGRAM_WITH_AVRDUDE

REM This is required for the LEONARDO baord to reset the board and have the COM port change.
REM After the port is opened at 1200 BPS we need to give the board a couple of seconds to
REM switch to a new COM port to allow the bootloader to receive the program.  The board
REM will wait for about 8 seconds for the code transmition.  Play with the -n # parameter
REM to increase/decrease the amount of time between reseting the board and transmitting the
REM program if you find the COM port is not available.  There MUST be a space between n and #.
REM See Automatic (Software) Reset and Bootloader Initiation at 
REM http://arduino.cc/en/Main/arduinoBoardLeonardo
IF "%TARGET_BOARD%"=="Leonardo" (
echo Reseting Leonardo Board "%BOARD_DATA_COMPORT%"
MODE %BOARD_DATA_COMPORT%:1200,N,8,1,P
ping localhost -n 4  > nul
)

set PROGRAM_CMD="%PROGRAM_TRANSFER_PATH%" -C"%PROGRAM_TRANSFER_CONFIG%" %PROGRAM_TRANSFER_VERBOSITY% -p%AVRPROCESSOR% -c%PROGRAMMER_PROTOCOL% -P%PROGRAMMER_COMPORT% %PROGRAM_TRANSFER_PARAMS% %LOCAL_MISC_PARAMS% -Uflash:w:%HEX_BINARY_FILE_PATH%:i
if %INCLUDE_EEPROM_FILE%=="TRUE" (
	if "%PROGRAMMER_EEPROM_SUPPORT%"=="false" (
		echo.
		echo [ERROR] ABORTING deployment - The programmer %TARGET_PROGRAMMER% does not support deployment of EEPROM files
		echo.
		goto :END )
	set PROGRAM_CMD=!PROGRAM_CMD! -Ueeprom:w:%EEPROM_FILE_PATH%:i	)

echo.
echo %PROGRAM_CMD%
%PROGRAM_CMD%
if %ERRORLEVEL%==1 goto :DEPLOYMENT_FAILED
echo.
goto :DEPLOYMENT_COMPLETE
goto :END

:PROGRAM_WITH_ATPROGRAM 


set PROGRAM_CMD="%PROGRAM_TRANSFER_PATH%" %PROGRAM_TRANSFER_VERBOSITY% -t %PROGRAMMER_COMPORT%  -i %PROGRAMMER_PROTOCOL% -d %AVRPROCESSOR% %PROGRAM_TRANSFER_PARAMS% %LOCAL_MISC_PARAMS% chiperase program -fl -f %HEX_BINARY_FILE_PATH% --format hex --verify
echo.
echo %PROGRAM_CMD%
%PROGRAM_CMD%
if %ERRORLEVEL%==1 goto :DEPLOYMENT_FAILED
echo.

set PROGRAM_CMD="%PROGRAM_TRANSFER_PATH%" %PROGRAM_TRANSFER_VERBOSITY% -t %PROGRAMMER_COMPORT% -i %PROGRAMMER_PROTOCOL% -d %AVRPROCESSOR% program -ee -f %EEPROM_FILE_PATH% --format hex --verify
if %INCLUDE_EEPROM_FILE%=="TRUE" (
	if "%PROGRAMMER_EEPROM_SUPPORT%"=="false" (
		echo.
		echo [ERROR] ABORTING deployment - The programmer %TARGET_PROGRAMMER% does not support deployment of EEPROM files
		echo.
		goto :END )

	echo Delay [5 seconds]
	REM Delay writing the eeprom - if done too quick avrdude fails to open the usb port.
	ping localhost -n 5 >NUL 
	echo.
	echo %PROGRAM_CMD%
	%PROGRAM_CMD%
	if %ERRORLEVEL%==1 goto :DEPLOYMENT_FAILED
	echo.
 	)
goto :DEPLOYMENT_COMPLETE
goto :END

:PROGRAM_WITH_FILE_COPY 
goto :DEPLOYMENT_COMPLETE
goto :END

:DEPLOYMENT_FAILED
echo Deployment Failed
call deployfail.bat %1 %2 %3 %4 %5 %6 %7 %8 %9
goto :END


:DEPLOYMENT_COMPLETE
echo Deployment Complete
call deploysuccess.bat %1 %2 %3 %4 %5 %6 %7 
goto :END
REM ********************************************************************



:END























