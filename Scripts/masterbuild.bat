@echo off 
cls 
REM /************************************************************ 
REM DO NOT MODIFY 
REM Automatically Generated On 2013-05-16 07-06-AM by pre-build97.bat 
REM *************************************************************/ 

REM Create lock file to prevent script from regenerating this file 
copy /Y masterbuild.bat masterbuild.lock 

:RUN_MSBUILD
REM Configure Atmel Studio 
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

REM Configure Deployment Script 
set DEPLOY_SCRIPT=%USERPROFILE%\Documents\Atmel Studio\deploy.bat 
msbuild ..\build.xml /verbosity:m /consoleloggerparameters:PerformanceSummary /p:DeployScript="%DEPLOY_SCRIPT%" /p:AtmelStudio=%ATMEL_STUDIO% /p:ProjectName="TemplateSketch.cppproj" /p:ListOfSuites="[]" /p:TargetSolutionDir=C:\Projects\Arduino-Installer\ArduinoLibrariesSource\TemplateSketch\ /p:TargetProjectDir=C:\Projects\Arduino-Installer\ArduinoLibrariesSource\TemplateSketch\TemplateSketch\
del /f masterbuild.lock 
