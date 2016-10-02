@echo off
setlocal EnableDelayedExpansion

REM Increment Build Counter
set BUILD_COUNTER=deploy-success-counter.txt
set BUILD_LOG_FILE=buildlog.txt
set START_COMPILE_COUNTER=startcompilecounter.txt

REM Create buildcounter file if it does not exist
if NOT EXIST %BUILD_COUNTER% (
  echo 0> %BUILD_COUNTER%
)

set /p COUNTER= <%BUILD_COUNTER%
set /a COUNTER+=1
echo !COUNTER!> %BUILD_COUNTER%



set DESTINATION=..\Utility\BuildNumber.cpp
::: Capture Date
for /f "tokens=2-4 delims=/ " %%a in ('date /T') do set year=%%c
for /f "tokens=2-4 delims=/ " %%a in ('date /T') do set month=%%a
for /f "tokens=2-4 delims=/ " %%a in ('date /T') do set day=%%b
set TODAY=%year%-%month%-%day%

::: Capture time
for /f "tokens=1 delims=: " %%h in ('time /T') do set hour=%%h
for /f "tokens=2 delims=: " %%m in ('time /T') do set minutes=%%m
for /f "tokens=3 delims=: " %%a in ('time /T') do set ampm=%%a
set NOW=%hour%-%minutes%-%ampm%

::: Build time stamp
set TIME_STAMP=%TODAY% %NOW%



::: Generate buildnumber.cpp
echo #ifndef BUILDINFO_H_ > %DESTINATION%
echo #include "buildinfo.h" >> %DESTINATION%
echo #endif >> %DESTINATION%
echo. >>%DESTINATION%
echo /************************************************************ >> %DESTINATION%
echo DO NOT MODIFY >> %DESTINATION%
echo Automatically Generated On %TIME_STAMP% by %~n0.bat >> %DESTINATION%
echo *************************************************************/ >> %DESTINATION%
echo. >>%DESTINATION%
echo #if defined(BUILDINFO_RAM) >> %DESTINATION%
echo static const uint16_t BUILD_NUMBER = !COUNTER!; >> %DESTINATION%
echo #elif defined(BUILDINFO_EEMEM) >> %DESTINATION%
echo static const uint16_t BUILD_NUMBER EEMEM = !COUNTER!; >> %DESTINATION%
echo #elif defined(BUILDINFO_PROGMEM) >> %DESTINATION%
echo static const uint16_t BUILD_NUMBER PROGMEM = !COUNTER!; >> %DESTINATION%
echo #endif >> %DESTINATION%
echo. >>%DESTINATION%
echo. >>%DESTINATION%
echo uint16_t GetBuildNumber() >> %DESTINATION%
echo { >> %DESTINATION%
echo     uint16_t val; >> %DESTINATION%
echo #if defined(BUILDINFO_RAM) >> %DESTINATION%
echo     val = BUILD_NUMBER; >> %DESTINATION%
echo #elif defined(BUILDINFO_EEMEM) >> %DESTINATION%
echo     val = eeprom_read_word( ^&BUILD_NUMBER ); >> %DESTINATION%
echo #elif defined(BUILDINFO_PROGMEM) >> %DESTINATION%
echo     val = pgm_read_word(^&BUILD_NUMBER); >> %DESTINATION%
echo #endif >> %DESTINATION%
echo     return val; >> %DESTINATION%
echo } >> %DESTINATION%

echo Build number increased to !COUNTER!


set /p COMPILE_ATTEMPTS=<%START_COMPILE_COUNTER%
set /a COUNTER=%COMPILE_ATTEMPTS%-1
echo %COUNTER%,%DATE:~-4%%DATE:~4,2%%DATE:~7,2%-%TIME:~0,2%:%TIME:~3,2%:%TIME:~6,2%,SUCCESS_DEPLOY>>%BUILD_LOG_FILE%

:END