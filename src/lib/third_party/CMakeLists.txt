
vor_reset_warning_level()

# third-party libraries are generated as static libraries,
# and linked to the Graphite library that uses them.
set(BUILD_SHARED_LIBS FALSE)

if(WIN32)
#  Insert flags here if there are some MSVC warnings to deactivate.
else()
   add_definitions(-Wno-unused -Wno-sign-compare -Wno-strict-aliasing)
   if(
       "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
       "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"
   )
      add_definitions(-Wno-deprecated-declarations)     # swig uses sprintf
      add_definitions(-Wno-suggest-override)            # swig
      add_definitions(-Wno-suggest-destructor-override) # swig
   endif()
endif()

add_subdirectory(swig)
