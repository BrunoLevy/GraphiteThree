
# Instructions for compiling Geogram

Geogram is tested under Linux (32 and 64 bits), Windows (32 and 64 bits) and MacOS/X. You will need CMake (version >= 2.8.11). There is no other dependancy (everything that you need is shipped with Geogram). It also works with Android (ARMv7 processors). Follow the Linux, MacOS/X, Windows or Android instructions below. In addition, Emscripten (C++ to Javascript transpiler) is also supported. Specific instructions for each platform are detailed below.

Contributors / Maintainers:

*   Linux platform: Bruno Levy
*   Windows platform: Nicolas Ray
*   MacOS/X platform: Samuel Hornus, Laurent Alonso, Fabien Bosquet
*   Emcripten platform: Bruno Levy

## Linux and MacOS/X

### Default options

    sh -f configure.sh
    cd build/Linux64-gcc-dynamic-Release (Linux) or build/Darwin-clang-dynamic-Release (MacOS/X)
    make

This will generate dynamic libraries and dynamically linked executables. If you want statically linked executables, see below.

### Installing (optional)

Still from the same directory:


    sudo make install


This will install Geogram libraries, header files and binaries in /usr/local/Geogram. Under recent versions of Mac/OSX (El Capitan), you need to create /usr/local before. See [this link](http://digitizor.com/fix-homebrew-permissions-osx-el-capitan/).

If you want to change the installation directory, create a file CMakeOptions.txt in the source tree (same directory as the main CMakeFile.txt) with CMAKE_INSTALL_PREFIX set to where you want to install Geogram. Example (or see CMakeOptions.txt.sample):

    (CMAKE_INSTALL_PREFIX /home/myself/installroot/Geogram)

Once Geogram is installed, you can compile the examples in src/tutorial as follows:

    cd src/tutorial/hello_geogram
    cmake .
    make
    ./hello_geogram


If Geogram was not install in its default location (/usr/local/Geogram), you will need before to set the environment variable GEOGRAM_INSTALL_ROOT:

    export GEOGRAM_INSTALL_ROOT=/home/myself/installroot/Geogram)
    cd src/tutorial/hello_geogram
    cmake .
    make
    ./hello_geogram


### Fine tuning

Alternatively, you can use:

    sh -f configure.sh <platform_name>


For instance, for generating statically linked executable, use:

    sh -f configure.sh Linux64-gcc


To obtain the list of supported platforms, use:

    sh -f configure.sh --help-platforms


## Windows

### Default options

*   start CMake (cmake-gui)
*   Where is the source code: <geogram_path>
*   Where to build the binaries: <geogram_path>/build/windows
*   Push the 'configure' button
*   Choose the compiler (32 or 64 bits)
*   (and ignore the warning about Doxygen)
*   Push the 'generate' button
*   Open <geogram_path>/build/windows/Geogram.sln in Visual studio
*   Select the 'Release' or 'Debug' (default) configuration (pulldown menu)
*   Build the solution (F7)

Visual Studio 2015 notes: when you install Visual Studio 2015, it does not automatically install the C++ compiler. To install it, manually create a C++ project in Visual C++, this will download the compiler and Windows Desktop development tools (3Gb !!).

### Installing

*   run Visual C++ as administrator (you need that to be allowed to write in "C:\Program Files")
*   build the INSTALL target

### Fine tuning

*   start CMake (cmake-gui)
*   Where is the source code: <geogram_path>
*   Where to build the binaries: <geogram_path>/build/windows
*   Click the 'Add Entry' button
*   Create a new variable of type 'String', with name 'VORPALINE_PLATFORM' and <platform_name> where <platform_name> is the name of one of the subdirectories in <geogram_path>/cmake/platforms

For instance, for generating shared objects and dynamically linked executables, use: 'VORPALINE_PLATFORM' = 'Win-vs-dynamic-generic'

*   Continue as in the previous paragraph ('Configure', 'Generate' etc...)

## More tuning (for both Linux and Windows)

Some parameters can be fine-tuned by renaming <geogram_path>/CMakeOptions.txt.sample as CMakeOptions.txt, modifying it and re-running CMake:

*   GEOGRAM_WITH_TETGEN: set to TRUE to activate compilation of Hang Si's TETGEN constrained [Delaunay](classGEO_1_1Delaunay.html "Abstract interface for Delaunay triangulation in Nd. ") library. If you do that, you should note that TETGEN is licensed under the [GNU Affero General Public License](http://www.gnu.org/licenses/agpl-3.0.en.html), which is different from the [three-clauses BSD license](http://en.wikipedia.org/wiki/BSD_licenses) used by GEOGRAM
*   GEOGRAM_WITH_GRAPHICS: set to FALSE to deactivate compilation of all graphics-related libs and programs (geogram_gfx, vorpaview, medit)
*   GEOGRAM_WITH_FPG: set to TRUE to activate compilation of Meyer and Pion's predicate filter generator (required if you plan to write new [PCK](namespaceGEO_1_1PCK.html "PCK (Predicate Construction Kit) implements a set of geometric predicates. PCK uses arithmetic filter...") predicates).
*   CMAKE_INSTALL_PREFIX: where to install Geogram (when building the INSTALL target)
*   CPACK_GENERATOR: can be used to generate a Debian package

## Android

What follow are the instructions for installing the NDK on a Linux box, cross-compiling Geogram and testing the binaries on the target Android platform.

### Preparing the NDK

*   Download the [Android NDK](https://developer.android.com/ndk/downloads/index.html)
*   Unpack the archive:

        chmod 755 android-ndk-rxxy-linux-x86_64.bin
        ./android-ndk-rxxy-linux-x86_64.bin

where 'rxxy' denotes the NDK version and release (e.g., r10e)

*   Create a standalone toolchain (cross-compiler)

        mkdir toolchain

        ./android-ndk-rxxy/build/tools/make-standalone-toolchain.sh
        --platform=android-zz --toolchain=arm-linux-androideabi-4.9
        --system=linux-x86_64 --install-dir=`pwd`/toolchain
        --stl=libc++


    where 'zz' denotes the target Android version (e.g., 19 for KitKat, 20 for Lollipop etc...)
*   Add the binaries directory of the toolchain in the path

        export PATH=`pwd`/toolchain/bin/:$PATH

(the line above can be added to your .bashrc)

### Cross-compiling Geogram

    cd geogram
    ./configure.sh Android-armv7-gcc
    cd build/Android-armv7-gcc-Release/
    make

Note: for now, only 32 bits ARM with FPU is supported. ARM64 can be quite easily supported (but this requires implementing a couple of ASM functions in [basic/atomics.h](atomics_8h.html "Functions for atomic operations. ")), will be implemented when I'll update my phone.

### Using the generated binaries on the phone

You can install [TerminalIDE](https://play.google.com/store/apps/details?id=com.spartacusrex.spartacuside&hl=fr) on the phone, then copy the binaries and some data files in TerminalIDE's working directory (/data/data/com.spartacusrex.spartacuside/files). It can be also used through [ADB](http://developer.android.com/tools/help/adb.html).

### What about performances ?

On my PC (Intel Core i7-4900MQ, 2.8 GHz), compute_delaunay with 850,000 points takes 1.4 seconds in parallel mode (and 4.3 seconds in sequential mode).

On my phone (HTC One M7, Snapdragon 600, 4 cores, 1.7 GHz), compute_delaunay with 850,000 points takes 15 seconds in parallel mode (and 35 seconds in sequential mode).

The phone has approximately 1/10th of the horsepower of the computer.

### What is the point ?

A portable, small, easy to compile and efficient geometry processing library can be useful for many projects, including 3D reconstruction with a mobile phone [here](http://cvg.ethz.ch/mobile/), reconstructions of buildings using drones, ...

## Emscripten

[Emscripten](http://kripken.github.io/emscripten-site/) is a C++ to Javascript "transpiler". It can create webpages with embedded OpenGL-enabled programs that can be executed in any modern webbrower, without needing any plugin. Some examples of web-compiled geogram programs are available [here](http://www.loria.fr/~levy/GLUP) and [here](http://www.loria.fr/~levy/GEOGRAM).

To transform a geogram C++ program into a web appliction, you first need to install a version of Emscripten for your system, follow instructions on [Emscripten website](http://kripken.github.io/emscripten-site/). The EMSCRIPTEN environment variable should point to the installation.

### Compiling geogram for Emscripten


    sh -f configure.sh Emscripten-clang
    cd build/Emscripten-clang-Release
    make


### Creating a webpage around a program

    cd build/Emscripten-clang-Release/bin
    <geogram_root>/tools/gen_emscripten_html.sh test_glup.js

If you need to embed datafiles:

    <geogram_root>/tools/gen_emscripten_html.sh vorpaview.js morph.tet6

All the datafiles specified on the command line after the js executable will be aggregated in an archive that is preloaded by the webpage (they are then accessible through a "pseudo file system" that interprets file system calls like open(), read(), close()...).

The so-generated programs are executable by any modern webbrowser (it uses HTML5/Javascript/WebGL). Performance is approximatively half the speed of a native single-threaded application, which is reasonable for doing small demos / teaching / educational programs...

## Generating the documentation

A (hopefully reasonably) up-to-date version of the documentation is available [here](http://alice.loria.fr/software/geogram/doc/html/). You may want to generate the documentation from your own version, or change some options of documentation generation.

### Linux and MacOS/X

You will need "doxygen" to generate the documentation.

    sh -f configure.sh
    cd build/Linux64-gcc-dynamic-Release (or build/Darwin-clang-dynamic-Release on MacOS/X)
    make doc-devkit-full

This generates the documentation in doc/devkit-full/html

Aternatively, one can also:

    make doc-devkit-internal // this generates also the documentation of the implementation private classes declared in the .cpp source files as well as class collaboration graphs;
    make doc-devkit-internal-light // the same, without class collaboration graphs.

To see all the possible targets, use "make help".
