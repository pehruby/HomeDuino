@echo off 
cls 
set SCRIPT_NAME=MakeIt.bat
set SCRIPT_VERSION=1.00
set SCRIPT_DATE=20130603

rem set root folder for project file
for %%A in ("%~dp0\..") do set "ROOT_FOLDER=%%~fA\"
rem echo %ROOT_FOLDER%

IF [%1] == [] goto :MISSING_PARAM1

REM Convert first parameter to lower case
echo>%1
dir /b/l %1>lower.tmp
set /p param1=<lower.tmp
del /F lower.tmp



IF [%param1%] == [demo] (
 set TARGET_STEP=Demo
  goto :GET_PROJECT_FILE )

IF [%param1%] == [compile] (
 set TARGET_STEP=CompileCurrent
  goto :GET_PROJECT_FILE )

IF [%param1%] == [compileall] (
 set TARGET_STEP=RestoreCurrentBoard
  goto :GET_PROJECT_FILE )


IF [%param1%] == [deploy] (
 set TARGET_STEP=DeployCurrent
  goto :GET_PROJECT_FILE )

IF [%param1%] == [deployall] (
 set TARGET_STEP=DeployAll
  goto :GET_PROJECT_FILE )

IF [%param1%] == [debug] (
 set TARGET_STEP=Debug
  goto :GET_PROJECT_FILE )


:MISSING_PARAM1
cls
echo %SCRIPT_NAME%. %SCRIPT_VERSION% %SCRIPT_DATE% - Compiles sketches for one or multiple
echo boards in the script\targetboard.xml file. Parameters are case insensitive.
echo Example - MakeIt.bat BuildCurrent
echo.
echo [Demo] 
echo -- Provides a sample output of a compilation against all supported board
echo [Compile]
echo -- Build sketch against the current board in the script\targetboard.xml file
echo [CompileAll]
echo -- Build the sketch for all supported boards in the script\targetboard.xml file
echo [Deploy]
echo -- Build sketch against the current board in the script\targetboard.xml file
echo [DeployAll]
echo -- Build the sketch for all supported boards in the script\targetboard.xml file
goto :END

:GET_PROJECT_FILE
REM Target project is the first CPPPROJ found in the directory - there should be only one
for  %%x in (..\*.cppproj) do (
  rem set "PROJECT_NAME=%%x"
  set PROJECT_NAME=%%~nx%%~xx
  set LIB_NAME=%%~nx
  goto :FOUND_PROJECT
)
:FOUND_PROJECT
rem echo Root %ROOT_FOLDER%
rem echo Project %PROJECT_NAME%
rem echo Library %LIB_NAME%
REM ************************************************************************
REM !HACK! Setting this environment variable forces programs invoked by the
REM batch file to run in the same security context of the invoker. 
REM This is necessary to prevent the elevated privilege window from poping up
REM when Atmel Studio is configured to run with Elevated Privileges
REM ************************************************************************
set __COMPAT_LAYER=RunAsInvoker

REM Support 6.1 and 6.0 Atmel studio
if exist "C:\Program Files (x86)\Atmel\Atmel Studio 6.1\atmelstudio.exe" (
set ATMEL_STUDIO="C:\Program Files (x86)\Atmel\Atmel Studio 6.1\atmelstudio.exe"
GOTO :INVOKE_MSBUILD
)
if exist "C:\Program Files\Atmel\Atmel Studio 6.1\atmelstudio.exe" (
set ATMEL_STUDIO="C:\Program Files\Atmel\Atmel Studio 6.1\atmelstudio.exe" 
GOTO :INVOKE_MSBUILD
)
if exist "C:\Program Files (x86)\Atmel\Atmel Studio 6.0\atmelstudio.exe" (
set ATMEL_STUDIO="C:\Program Files (x86)\Atmel\Atmel Studio 6.0\atmelstudio.exe"
GOTO :INVOKE_MSBUILD
)
if exist "C:\Program Files\Atmel\Atmel Studio 6.0\atmelstudio.exe" (
set ATMEL_STUDIO="C:\Program Files\Atmel\Atmel Studio 6.0\atmelstudio.exe" 
GOTO :INVOKE_MSBUILD
)

:INVOKE_MSBUILD
msbuild makeit.xml /verbosity:m  /consoleloggerparameters:PerformanceSummary  /p:RootFolder=%ROOT_FOLDER% /p:AtmelStudio=%ATMEL_STUDIO% /p:ProjectName=%PROJECT_NAME% /p:LibraryName=%LIB_NAME% /p:TargetStep=%TARGET_STEP%
:END