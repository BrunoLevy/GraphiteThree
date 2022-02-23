
# ========================================================================
set(APP_NAME graphite)

# ========================================================================

set(MODMGR_APPEND_LIBPATH "")

# This is to avoid "DLL nightmares" on Windows: declare
# the path where Python was found (if any) so that graphite
# executable can add it to the PATH env. variable and the
# gompy plugin finds the Python DLL.
if(WIN32)
  graphite_find_python()
  if(${GRAPHITE_FOUND_PYTHON})
      set(MODMGR_APPEND_LIBPATH ${PYTHON_LIBRARIES})
  endif()
  # Flip slashes
  string(REPLACE "\\" "/" MODMGR_APPEND_LIBPATH "${MODMGR_APPEND_LIBPATH}")
  # Strip name of lib
  string(REGEX REPLACE "/[^/]+$" "" MODMGR_APPEND_LIBPATH "${MODMGR_APPEND_LIBPATH}")
  # Go one subdirectory upper
  set(MODMGR_APPEND_LIBPATH "${MODMGR_APPEND_LIBPATH}/..")  
endif()

add_definitions(-DMODMGR_APPEND_LIBPATH="${MODMGR_APPEND_LIBPATH}")

# ========================================================================

aux_source_directories(SOURCES "" .)
add_executable(${APP_NAME} ${SOURCES})
target_link_libraries(${APP_NAME} scene_graph)

set_target_properties(
   ${APP_NAME} PROPERTIES
   FOLDER "GRAPHITE/Programs"
)

