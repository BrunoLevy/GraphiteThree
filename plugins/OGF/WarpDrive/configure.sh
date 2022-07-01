#!/bin/sh

# This file for Linux users, 
# launches CMake and creates configuration for
# Release and Debug modes.

if [ -z "$GEOGRAM_PATH" ] ; then
    GEOGRAM_PATH=$PWD/../../../geogram/
fi
if [ -z "$GRAPHITE_PATH" ] ; then
    GRAPHITE_PATH=$PWD/../../../GraphiteThree/
fi

# args
build_name_suffix=
while [ -n "$1" ]; do
    case "$1" in
        --build_name_suffix=*)
            build_name_suffix=`echo "$1" | sed 's/--build_name_suffix=\(.*\)$/\1/'`
            shift
            ;; 
        *)
            break;
            ;;
    esac
done

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
        Darwin*)
            os=Darwin-clang-dynamic
            ;;
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


   build_dir=build/$platform$build_name_suffix
   mkdir -p $build_dir
   (cd $build_dir;
    cmake \
        -DCMAKE_BUILD_TYPE:STRING=$config \
        -DVORPALINE_PLATFORM:STRING=$os \
        -DGEOGRAM_SOURCE_DIR:STRING=$GEOGRAM_PATH \
        -DGRAPHITE_SOURCE_DIR:STRING=$GRAPHITE_PATH \
    $cmake_options ../../)
done

echo
echo ============== Plugin build configured ==================
echo

cat << EOF
To build graphite:
  - go to build/$os-Release$build_name_suffix or build/$os-Debug$build_name_suffix
  - run 'make' or 'cmake --build .'

Note: local configuration can be specified in CMakeOptions.txt
(see CMakeOptions.txt.sample for an example)
You'll need to re-run configure.sh if you create or modify CMakeOptions.txt

EOF

