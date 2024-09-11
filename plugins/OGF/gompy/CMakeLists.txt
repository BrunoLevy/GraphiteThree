##############################################################################
# CMake file for gompy
##############################################################################

cmake_minimum_required(VERSION 3.5)

graphite_find_python()

if(${GRAPHITE_FOUND_PYTHON})

   message(STATUS "Found Python headers and libs, building gompy")
   message(STATUS "   PYTHON_INCLUDE_DIRS=${PYTHON_INCLUDE_DIRS}")
   message(STATUS "   PYTHON_LIBRARIES   =${PYTHON_LIBRARIES}")

   project(gompy)

   if("${GEOGRAM_SOURCE_DIR}" STREQUAL "")
      if(EXISTS ${CMAKE_SOURCE_DIR}/../../../../Vorpaline/trunk/)
         set(
            GEOGRAM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../../../../Vorpaline/trunk"
            CACHE PATH "full path to the Geogram (or Vorpaline) installation"
         )
       else()
         set(
            GEOGRAM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../../../geogram/"
            CACHE PATH "full path to the Geogram (or Vorpaline) installation"
         )
       endif()
   endif()

   if("${GRAPHITE_SOURCE_DIR}" STREQUAL "")
     set(
       GRAPHITE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../../../GraphiteThree/"
       CACHE PATH "full path to the Graphite installation"
     )
   endif()

   include(${GRAPHITE_SOURCE_DIR}/cmake/graphite.cmake)

   if(EXISTS ${CMAKE_SOURCE_DIR}/CMakeOptions.txt)
     message(STATUS "Using options file: ${CMAKE_SOURCE_DIR}/CMakeOptions.txt")
     include(${CMAKE_SOURCE_DIR}/CMakeOptions.txt)
   endif()


   #############################################################################

   include_directories(${CMAKE_SOURCE_DIR}/../../)
   include_directories("${PYTHON_INCLUDE_DIRS}")

   # DLLs/shared objects are generated in Graphite binary directory

   set(
      CMAKE_RUNTIME_OUTPUT_DIRECTORY
      ${GRAPHITE_SOURCE_DIR}/${RELATIVE_BIN_OUTPUT_DIR}
   )

   set(
      CMAKE_LIBRARY_OUTPUT_DIRECTORY
      ${GRAPHITE_SOURCE_DIR}/${RELATIVE_LIB_OUTPUT_DIR}
   )

   #############################################################################

   aux_source_directories(SOURCES "Source Files\\common" common)
   aux_source_directories(SOURCES "Source Files\\interpreter" interpreter)
   gomgen(gompy)

   add_library(gompy ${SOURCES})

   target_link_libraries(gompy scene_graph skin gom_gom gom)
   target_link_libraries(gompy renderer basic)
   target_link_libraries(gompy geogram_gfx geogram)
   target_link_libraries(gompy ${PYTHON_LIBRARIES})

   set_target_properties(
      gompy PROPERTIES
      FOLDER "GRAPHITE/Plugins"
   )

else()
   message(STATUS "Did not find Python headers or libs, not building gompy")
endif()
