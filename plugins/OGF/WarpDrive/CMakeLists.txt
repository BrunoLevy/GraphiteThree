##############################################################################
# CMake file for WarpDrive
##############################################################################

cmake_minimum_required(VERSION 3.5...3.30)

project(WarpDrive)

if("$ENV{GEOGRAM_SOURCE_DIR}" STREQUAL "")
  set(
     GEOGRAM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../geogram/"
     CACHE PATH "full path to the Geogram (or Vorpaline) installation"
  )
else()
  set(
    GEOGRAM_SOURCE_DIR "$ENV{GEOGRAM_SOURCE_DIR}"
    CACHE PATH "full path to the Geogram (or Vorpaline) installation"
  )
endif()

if(NOT "$ENV{GRAPHITE_SOURCE_DIR}" STREQUAL "")
  set(
     GRAPHITE_SOURCE_DIR "$ENV{GRAPHITE_SOURCE_DIR}"
     CACHE PATH "full path to the Graphite installation"
  )
  include_directories(${CMAKE_SOURCE_DIR}/../../)
else()
   set(
     GRAPHITE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../"
     CACHE PATH "full path to the Graphite installation"
   )
endif()

include(${GRAPHITE_SOURCE_DIR}/cmake/graphite.cmake)

if(EXISTS ${CMAKE_SOURCE_DIR}/CMakeOptions.txt)
   message(STATUS "Using options file: ${CMAKE_SOURCE_DIR}/CMakeOptions.txt")
   include(${CMAKE_SOURCE_DIR}/CMakeOptions.txt)
endif()


##############################################################################

include_directories(${CMAKE_SOURCE_DIR}/../../)

# DLLs/shared objects are generated in Graphite binary directory

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GRAPHITE_SOURCE_DIR}/${RELATIVE_BIN_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GRAPHITE_SOURCE_DIR}/${RELATIVE_LIB_OUTPUT_DIR})

##############################################################################

aux_source_directories(SOURCES "Source Files\\common" common)
aux_source_directories(SOURCES "Source Files\\algo" algo)
aux_source_directories(SOURCES "Source Files\\commands" commands)
aux_source_directories(SOURCES "Source Files\\shaders" shaders)
aux_source_directories(SOURCES "Source Files\\IO" IO)
aux_source_directories(SOURCES "Source Files\\interfaces" interfaces)
aux_source_directories(SOURCES "Source Files\\third_party\\muparser" third_party/muparser)

gomgen(WarpDrive)

add_library(WarpDrive ${SOURCES})

target_link_libraries(WarpDrive mesh_gfx)

if(GEOGRAM_WITH_VORPALINE)
  target_link_libraries(WarpDrive vorpalib)
endif()

target_link_libraries(WarpDrive exploragram geogram_gfx geogram)

set_target_properties(
   WarpDrive PROPERTIES
   FOLDER "GRAPHITE/Plugins"
)
