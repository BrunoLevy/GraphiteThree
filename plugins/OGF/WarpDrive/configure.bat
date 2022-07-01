echo OFF
SETLOCAL ENABLEDELAYEDEXPANSION

REM ---------------------------------------------------------------- 
REM Check for options: [ --build_name_suffix suffix ]
REM ----------------------------------------------------------------
set buildNameSuffix=""
if "%1" ==  "--build_name_suffix" (
   set buildNameSuffix=%2
   SHIFT & SHIFT
)

ECHO Configuring Graphite plugin

ECHO ----------------------------------------------------------------
ECHO Detecting Visual C++ 

REM ---------------------------------------------------------------- 
REM Find latest installed Visual C++
REM ----------------------------------------------------------------

SET "MSVCVER=0"
FOR /F "tokens=1-5 delims=\. usebackq" %%A in (
    `REG QUERY "HKLM\SOFTWARE\Microsoft\VisualStudio" 2^> nul`
) DO (
    SET /a F=%%E
	if !F! GTR !MSVCVER! (SET "MSVCVER=!F!")
)

REM ----------------------------------------------------------------
REM Trying to detect Visual C++ 2017:
REM Damnit ! VS2017 does not create a key in HKLM\SOFTWARE\Microsfoft\VisualStudio
REM ... but there is another (cryptic) location where it leavese something...
REM ----------------------------------------------------------------

FOR /F "tokens=1-5 delims=\. usebackq" %%A in (
   `REG QUERY "HKLM\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\SxS\VS7" 2^> nul`
) DO (
    SET /a F=%%A
    if !F! GTR !MSVCVER! (SET "MSVCVER=!F!")
)

ECHO MSVC version = %MSVCVER%

rem *****************************************************************************

rem Creating directories if they are missing
if NOT EXIST build (
	echo creating build directory...
	md build
)
if NOT EXIST build\Windows%buildNameSuffix% (
	echo creating build\Windows%buildNameSuffix% directory...
	md build\Windows%buildNameSuffix%
)
chdir build\Windows%buildNameSuffix%\

echo ***** Creating project *****

cmake -G "Visual Studio %MSVCVER% Win64" ..\..\

ECHO ----------------------------------------------------------------
