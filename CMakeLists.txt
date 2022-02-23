##############################################################################
# Graphite/// root CMakeList
##############################################################################

# CMake 2.8.11 is required for 2 reasons:
# - it is the first version that fully supports the specification of Visual
# Studio toolsets (v110_xp).
# - it is the version that supports the command string(TIMESTAMP ...)
cmake_minimum_required(VERSION 2.8.11)

project(Graphite)
include(${CMAKE_SOURCE_DIR}/cmake/graphite.cmake)

##############################################################################

# If there is a bundled Geogram or Vorpaline distribution, compile it as well.
if(IS_DIRECTORY ${CMAKE_SOURCE_DIR}/geogram)
   add_subdirectory(geogram)
endif()

add_subdirectory(src/lib/third_party)

add_subdirectory(src/lib/OGF/basic)
add_subdirectory(src/lib/OGF/renderer)
add_subdirectory(src/lib/OGF/gom)
add_subdirectory(src/lib/OGF/gom_gom)
add_subdirectory(src/lib/OGF/skin)
add_subdirectory(src/lib/OGF/scene_graph)
add_subdirectory(src/lib/OGF/mesh)
add_subdirectory(src/lib/OGF/voxel)
add_subdirectory(src/lib/OGF/luagrob)
add_subdirectory(src/lib/OGF/devel)
add_subdirectory(src/lib/OGF/skin_imgui)
add_subdirectory(src/bin/graphite)
add_subdirectory(src/bin/gomgen)
add_subdirectory(doc)

add_subdirectory(plugins/OGF)

# Make Graphite the startup project in Visual C++
if(WIN32)
  set_property(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY
    VS_STARTUP_PROJECT graphite
  )
endif()
