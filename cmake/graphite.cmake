
##############################################################################

if(${CMAKE_PROJECT_NAME} STREQUAL "Graphite")
  set(GRAPHITE_PLUGIN FALSE)
else()
  set(GRAPHITE_PLUGIN TRUE)
endif()

if(NOT GRAPHITE_PLUGIN)
   set(GRAPHITE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
   include(${GRAPHITE_SOURCE_DIR}/cmake/graphite_config.cmake)
endif()

# CMakeOptions is included after (so that it is 
# possible to override user-editable variables in
# it instead of using CMakeGUI)

if(EXISTS ${GRAPHITE_SOURCE_DIR}/CMakeOptions.txt)
   message(STATUS "Using options file: ${GRAPHITE_SOURCE_DIR}/CMakeOptions.txt")
   include(${GRAPHITE_SOURCE_DIR}/CMakeOptions.txt)
endif()

include(${GEOGRAM_SOURCE_DIR}/cmake/geogram.cmake)

##############################################################################
# GOMGEN_EXE: full path to the gomgen executable
if(WIN32)
   set(GOMGEN_EXE ${GRAPHITE_SOURCE_DIR}/${RELATIVE_BIN_DIR}/gomgen.exe)
else()
   set(GOMGEN_EXE ${GRAPHITE_SOURCE_DIR}/${RELATIVE_BIN_DIR}/gomgen)   
endif()   
   
##############################################################################

# Usage: gomgen(library_name)
# Starts the gomgen code generator for the specified library (i.e. generates
# GOM meta-information for all classes declared as 'gom_class').

macro(gomgen __lib)

# Get the current include path, and format it so that it can be
# specified to gomgen  
  get_property(
    INCLUDE_PATH DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY INCLUDE_DIRECTORIES
  )
  set(GOMGEN_INCLUDES "")
  foreach(INCLUDE_DIR IN LISTS INCLUDE_PATH)
    list_append(GOMGEN_INCLUDES "-I${INCLUDE_DIR}")
  endforeach()

# We make the gom generated file dependent on all the header files
# This adds too many dependencies (thus launches gomgen too often),
# but too often is better than not often enough !!
# (I'd like to find a means of sending the output of gomgen -deps in there
#  but it seems too complicated...)

  file(GLOB_RECURSE GOMGEN_DEPS "*.h")

# The arguments passed to gomgen.  
# Note: to save preprocessor output to gomgenerated.cpp.I,
#  add the -E flag (sometiles useful for debugging)

  set(
    GOMGEN_ARGS -ogomgenerated_${__lib}.cpp  
    -i${CMAKE_CURRENT_SOURCE_DIR}/../${__lib} ${GOMGEN_INCLUDES}
  )
  
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gomgenerated_${__lib}.cpp
    DEPENDS ${GOMGEN_EXE} ${GOMGEN_DEPS}
    COMMAND ${GOMGEN_EXE}
    ARGS    ${GOMGEN_ARGS}
  )
  
  set(SOURCES ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/gomgenerated_${__lib}.cpp)

  # Under Windows, I do not manage to update the dependencies for gomgenerated,
  # therefore I create an additional target to launch it manually (not
  # very satisfactory but at least it makes it possible to start working...)
  if(WIN32)
    add_custom_target(
      run_gomgen_for_${__lib}
      COMMAND ${GOMGEN_EXE} ${GOMGEN_ARGS}
    )
	set_target_properties(
	  run_gomgen_for_${__lib} PROPERTIES
	  FOLDER "GRAPHITE/GomGen"
	)
  endif()
  
endmacro()

##############################################################################

# Usage: copy_geogram_DLLs_for(target)
#   where target denotes a build target (usually the graphite executable)
# Under Windows: copies the geogram DLLs used by Graphite in the binaries
#  directory (else it cannot find them, why ? I don't know...)
# Under Linux: does nothing

macro(copy_geogram_DLLs_for __target)

  if(WIN32 AND NOT USE_BUILTIN_GEOGRAM)

    add_custom_command(
      TARGET ${__target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${GEOGRAM_SOURCE_DIR}/${RELATIVE_BIN_DIR}/geogram.dll 
          ${CMAKE_SOURCE_DIR}/${RELATIVE_BIN_DIR}/geogram.dll
    )

    add_custom_command(
      TARGET ${__target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${GEOGRAM_SOURCE_DIR}/${RELATIVE_BIN_DIR}/geogram_gfx.dll 
         ${CMAKE_SOURCE_DIR}/${RELATIVE_BIN_DIR}/geogram_gfx.dll
    )

    add_custom_command(
      TARGET ${__target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${GEOGRAM_SOURCE_DIR}/${RELATIVE_BIN_DIR}/geogram_glfw3.dll
         ${CMAKE_SOURCE_DIR}/${RELATIVE_BIN_DIR}/geogram_glfw3.dll
    )

    if(GEOGRAM_WITH_VORPALINE)
      add_custom_command(
        TARGET ${__target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${GEOGRAM_SOURCE_DIR}/${RELATIVE_BIN_DIR}/vorpalib.dll 
            ${CMAKE_SOURCE_DIR}/${RELATIVE_BIN_DIR}/vorpalib.dll
      )
    endif()

  endif()

endmacro()

##############################################################################

include_directories(${GRAPHITE_SOURCE_DIR}/src/lib)
include_directories(${GRAPHITE_SOURCE_DIR}/src/lib/third_party)
include_directories(${GRAPHITE_SOURCE_DIR}/plugins)
link_directories(${GRAPHITE_SOURCE_DIR}/${RELATIVE_LIB_DIR})

##############################################################################
