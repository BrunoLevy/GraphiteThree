# User-configurable variables:
# - Path to Geogram

# Detecting Python
macro(graphite_find_Python)
   # Use version of Python compiled with:
   # ./configure --with-pydebug --without-pymalloc --prefix /opt/debugpython/
   #  --enable-shared
   #set(WHERE_ARE_PYTHON_INCLUDES /opt/debugpython/include/python3.7d/)
   #set(WHERE_IS_PYTHON_LIB /opt/debugpython/lib/libpython3.7d.so)
   #find_package(PythonLibs 3 QUIET) #deprecated, replaced with Python3
   find_package(Python3 COMPONENTS Development QUIET)
   if(Python3_Development_FOUND)
      set(PYTHON_INCLUDE_DIRS ${Python3_INCLUDE_DIRS})
      set(PYTHON_LIBRARIES ${Python3_LIBRARIES})
   else()
      message(
         STATUS
	"CMake did not find Python library, 
         using default fallbacks (edit WHERE_IS... in CMakeGUI if need be)."	
      )
      set(PYTHON_INCLUDE_DIRS ${WHERE_ARE_PYTHON_INCLUDES})
      set(PYTHON_LIBRARIES ${WHERE_IS_PYTHON_LIB})
   endif()
   if(
      NOT "${PYTHON_INCLUDE_DIRS}" STREQUAL "" AND
      NOT "${PYTHON_LIBRARIES}" STREQUAL ""
   )
      set(GRAPHITE_FOUND_PYTHON TRUE)
   endif()
endmacro()

# Missing in standard configuration (needed for Qt to
# find glu32)
# (TODO: do we still need that ? )
if(WIN32)
   set(
      CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} 
      "C:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x64"
    )
endif()

# Path to geogram
#################

if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/geogram/)
   set(
      GEOGRAM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/geogram/"
      CACHE PATH "full path to the Geogram (or Vorpaline) installation"
   )
   set(USE_BUILTIN_GEOGRAM TRUE)
else()
   set(
      GEOGRAM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../geogram/"
      CACHE PATH "full path to the Geogram (or Vorpaline) installation"
   )
   set(USE_BUILTIN_GEOGRAM FALSE)
endif()


