@echo off
setlocal EnableDelayedExpansion

REM Increment Build Counter
set BUILD_COUNTER=deploy-fail-counter.txt
set BUILD_LOG_FILE=buildlog.txt
set START_COMPILE_COUNTER=startcompilecounter.txt

REM Create buildcounter file if it does not exist
if NOT EXIST %BUILD_COUNTER% (
  echo 0> %BUILD_COUNTER%
)

set /p COUNTER= <%BUILD_COUNTER%
set /a COUNTER+=1
echo !COUNTER!> %BUILD_COUNTER%


set /p COMPILE_ATTEMPTS=<%START_COMPILE_COUNTER%
set /a COUNTER=%COMPILE_ATTEMPTS%-1
echo %COUNTER%,%DATE:~-4%%DATE:~4,2%%DATE:~7,2%-%TIME:~0,2%:%TIME:~3,2%:%TIME:~6,2%,FAIL_DEPLOY>>%BUILD_LOG_FILE%

:END