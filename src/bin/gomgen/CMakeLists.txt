set(APP_NAME gomgen)

if(WIN32)
#  Insert flags here if there are some MSVC warnings to deactivate.
else()
   add_definitions(-Wno-unused -Wno-sign-compare -Wno-strict-aliasing)
   if(
       "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
       "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"
   )
      add_definitions(-Wno-suggest-override)            # swig
      add_definitions(-Wno-suggest-destructor-override) # swig
   endif()
endif()


aux_source_directories(SOURCES "" .)
add_executable(${APP_NAME} ${SOURCES})
target_link_libraries(${APP_NAME} gom basic geogram swig)
set_target_properties(
   ${APP_NAME} PROPERTIES
   FOLDER "GRAPHITE/Programs"
)
