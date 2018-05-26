@ECHO OFF
call build.bat
set /A b = %__MY_BUILD%+1
echo set __MY_BUILD=%b% > Build.bat
echo #define __MY_BUILD %b% > Build.h
set DATESTAMP=%DATE:~4,2%/%DATE:~7,2%/%DATE:~10,4%
set TIMESTAMP=%TIME:~0,2%:%TIME:~3,2%:%TIME:~6,2%
echo Build number now %b% on %DATESTAMP% at %TIMESTAMP% 
