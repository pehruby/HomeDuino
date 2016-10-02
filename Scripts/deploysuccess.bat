REM ------------------------------------------------------------------
REM deploy applications to avr microcontrollers
REM Omar Franciscio - 08/25/2012
REM Script Generated On 2012-09-29 10-05-AM
REM Parameters
REM %1 Solution Directory
REM %2 Project Directory
REM %3 avrdude path
REM %4 avrdude config
REM %5 ATPROGRAM path
REM %6 library path
REM %7 header path
REM ------------------------------------------------------------------


set FILE_PATTERN=deploy-success*.bat
REM - Exectue all batch files matching filepattern in descending order
FOR /F "tokens=*" %%G IN ('dir /b %FILE_PATTERN% /O-N') DO (
	call %%G %1 %2 %3 %4 %5 %6 %7 
	rem echo %%G  %1 %2 %3 %4 %5 %6 %7
)
