#!/bin/sh

# This file for Linux users, 
# launches CMake and creates configuration for
# Release and Debug modes.

if [ -d $PWD/../../../../Vorpaline/trunk/ ]
then
    GEOGRAM_PATH=$PWD/../../../../Vorpaline/trunk/
else
    GEOGRAM_PATH=$PWD/../../../geogram/
fi
    
echo
echo ============= Checking for CMake ============
echo

if (cmake --version); then
    echo "Found CMake"
    echo
else
    echo "Error: CMake not found, please install it (see http://www.cmake.org/)"
    exit 1
fi

# Check the current OS

os="$1"
if [ -z "$os" ]; then
    os=`uname -a`
    case "$os" in
        Linux*x86_64*)
            os=Linux64-gcc-dynamic
            ;;
        Linux*amd64*)
            os=Linux64-gcc-dynamic
            ;;
        Linux*i586*|Linux*i686*)
            os=Linux32-gcc-dynamic
            ;;
        *)
            echo "Error: OS not supported: $os"
            exit 1
            ;;
    esac
fi

#  Import plaform specific environment

if [ ! -f $GEOGRAM_PATH/cmake/platforms/$os/setvars.sh ]
then
    echo $os: no such platform
    exit 1
fi

. $GEOGRAM_PATH/cmake/platforms/$os/setvars.sh || exit 1

# Generate the Makefiles

for config in Release Debug; do
   platform=$os-$config
   echo
   echo ============= Creating makefiles for $platform ============
   echo

   build_dir=build/$platform
   mkdir -p $build_dir
   (cd $build_dir;
    cmake \
        -DCMAKE_BUILD_TYPE:STRING=$config \
        -DVORPALINE_PLATFORM:STRING=$os \
    $cmake_options ../../)
done

echo
echo ============== Plugin build configured ==================
echo

cat << EOF
To build the plugin:
  - go to build/$os-Release or build/$os-Debug
  - run 'make' or 'cmake --build .'

Note: local configuration can be specified in CMakeOptions.txt
(see CMakeOptions.txt.sample for an example)
You'll need to re-run configure.sh if you create or modify CMakeOptions.txt

EOF

