@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

REM ---------------------------------------------------------------- 
REM Check for options: [ --build_name_suffix suffix ]
REM ----------------------------------------------------------------
set buildNameSuffix=""
if "%1" ==  "--build_name_suffix" (
   set buildNameSuffix=%2
   SHIFT & SHIFT
)

ECHO Configuring Graphite build

ECHO ----------------------------------------------------------------
ECHO Detecting Python (Python is optional)

SET WHERE_IS_PYTHON=NOIDEA

REM Try Python distribution first
FOR /D %%A IN (C:\Python3*) DO (
   SET WHERE_IS_PYTHON=%%A
)

REM Then Anaconda (preferred). If both are found, then
REM Anaconda is selected.
FOR /D %%A IN (C:\anaconda* %HOMEDRIVE%%HOMEPATH%\Anaconda* C:\ProgramData\Anaconda*) DO (
   SET WHERE_IS_PYTHON=%%A
)

SET WHERE_ARE_PYTHON_INCLUDES=NOIDEA
if exist %WHERE_IS_PYTHON%\include (
   SET WHERE_ARE_PYTHON_INCLUDES=%WHERE_IS_PYTHON%\include
)

SET WHERE_IS_PYTHON_LIB=NOIDEA
FOR %%A IN (%WHERE_IS_PYTHON%\Libs\python*.lib) DO (
   SET WHERE_IS_PYTHON_LIB=%%A
)

ECHO WHERE_ARE_PYTHON_INCLUDES=%WHERE_ARE_PYTHON_INCLUDES%
ECHO WHERE_IS_PYTHON_LIB=%WHERE_IS_PYTHON_LIB%

ECHO ----------------------------------------------------------------

if not exist "geogram\CMakeOptions.txt" (
   copy "geogram\CMakeOptions.txt.graphite" "geogram\CMakeOptions.txt"
)

REM ----------------------------------------------------------------
REM Create build directory and run cmake
REM ----------------------------------------------------------------

ECHO Starting CMake...
ECHO  (NOTE: it may complain about missing VULKAN, you can safely ignore)

if not exist "build\Windows%buildNameSuffix%" (
   mkdir "build\Windows%buildNameSuffix%"
)

cd build\Windows%buildNameSuffix%

"%ProgramFiles%\cmake\bin\cmake.exe" ..\.. ^
 -DVORPALINE_PLATFORM:STRING=Win-vs-dynamic-generic ^
 -DWHERE_ARE_PYTHON_INCLUDES="%WHERE_ARE_PYTHON_INCLUDES%" ^
 -DWHERE_IS_PYTHON_LIB="%WHERE_IS_PYTHON_LIB%"


REM -----------------------------------------------------------------
REM Wait for user keypress to keep DOS box open
REM -----------------------------------------------------------------

ECHO ----------------------------------------------------------------

if exist "Graphite.sln" (
   ECHO Graphite build is configured
   ECHO Visual Studio solution is in GraphiteThree\build\Windows%buildNameSuffix%\Graphite.sln
) else (
   ECHO ERROR: could not generate Visual Studio solution
   ECHO Maybe you have an old CMake / Visual Studio ...
   echo ... Try old_configure.bat
)

ECHO ----------------------------------------------------------------

REM set /p DUMMY=Hit ENTER to continue...

