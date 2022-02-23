# Instructions for compiling Graphite

## I. Windows

### 1. Prerequisites

- Microsoft Visual C++ (tested with ver. 2012, 2013, 2015, 2017, works probably with any version >= 2008)

    If you do not have a version of Visual C++ already, you can download Visual Studio Community from: \n
    https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx

- Cmake (ver >= 2.8.11)

    Download from: https://cmake.org/download/

    Use the Win32-x86 installer 

    (yes, you can use a 32 bits version of CMake to compile Graphite/// in 64 bits, see C.4)

- Note: Python and Qt are no longer needed/used

### 2. Configuration

Double click on "configure.bat", this will automatically detect and use the
most recent visual C++ installed on the machine. This will generate a
single all-in-one Visual C++ solution with both Graphite and Geogram.

### 3. Compiling Graphite

- Open graphite3-vvv-rrr/GraphiteThree/build/Windows/Graphite.sln
- Default configuration is Debug -> choose Release (pulldown menu top right)
- BUILD menu -> build solution
- In Solution Explorer, right click on 'graphite' project,
   set it as startup project
- start Graphite with <Ctrl><F5>
- Alternatively, you can directly start the executable by clicking on it,
it is in graphite3-vvv-rrr/GraphiteThree/build/Windows/Release/bin/graphite.exe
(or you can create a shortcut on the desktop)

## II. Linux

### 1. Prerequisites
   - g++
   - Cmake (ver >= 2.8.11)
   - Note: Qt and Python are no longer needed

### 2.a. Using the all-in-one generation script

    cd graphite3-vvv-rrr/
    ./make_it.sh

If everything goes well, you can start Graphite/// with:
    bin/graphite

Note: makeit.sh needs to be started from the directory that contains it
("./make_it.sh"), do not use "graphite3-vvv-rrr/makeit.sh", it will not 
work (you need to 'cd' there before launching "make_it.sh"). This will 
compile both geogram and Graphite.

### 2.b. Manual compilation

Here are the instructions to compile Geogram and Graphite (you may want
to do that if you play with the CMakeLists):

    cd graphite3-vvv-rrr/GraphiteThree/
    ./configure.sh
    cd build/Linux64-gcc-dynamic-Release
    make -j8

Generated binary is in `Graphite3-vvv-rrr/GraphiteThree/build/Linux64-gcc-dynamic-Release/bin/graphite`

### 3. Tuning Compilation Parameters

If a file named CMakeOptions.txt is found in Graphite3-vvv-rrr/geogram/
or Graphite3-vvv-rrr/GraphiteThree/, then it is included by CMake. You
can use this mechanism to specify some variables and override system
settings.

# Generating the documentation

A (hopefully reasonably) up-to-date version of the documentation
is available [here](http://alice.loria.fr/software/graphite/doc/html/).
You may want to generate the documentation from your own version, or
change some options of documentation generation.

## Linux 

You will need "doxygen" to generate the documentation.

    sh -f configure.sh
    cd build/Linux64-gcc-Release
    make doc

This generates the documentation in doc/doc/html

# Standalone binary version for Windows


To create a standalone binary version (that can be zipped and
given to somebody else, or that can be carried on a USB key and executed
everywhere), there is an automatic script. It is in:

    lib/make_windows_dist.lua

Start it using GEL/execute file. Enter the version tag in the dialog
box (just use to name the archive, chose whatever you want). This will
generate a standalone binary distribution of Graphite in GRAPHITEDIST/GraphiteThree
as well as a zip archive.

Note: using the standalone Graphite may require to install Visual C++
redistribuable package on the machine where it will be executed.

What it does:

**Warning:** obsolete as Qt is no longer a dependency

It copies the following files:

- create a subdirectory "QtPlugins" in GraphiteThree/build/Windows/bin/Release/QtPlugins
- copy the "platforms" subdirectory found in the plugins/ subdirectory of the Qt distribution

When Graphite detects the presence of the QtPlugins subdirectory in its file
hierarchy, it redefines the path used by Qt to this subdirectory. Thus Graphite
can run anywhere. It does not need any installation process, just start graphite.exe from the
bin/ subdirectory.
