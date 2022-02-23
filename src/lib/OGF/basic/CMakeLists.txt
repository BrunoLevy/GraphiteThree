
aux_source_directories(SOURCES "Source Files"                .)
aux_source_directories(SOURCES "Source Files\\common"   common)
aux_source_directories(SOURCES "Source Files\\os"       os)
aux_source_directories(SOURCES "Source Files\\modules"  modules)
aux_source_directories(SOURCES "Source Files\\math"     math)
add_library(basic ${SOURCES})
target_link_libraries(basic geogram)

if(UNIX)
  target_link_libraries(basic pthread dl m)
elseif(WIN32)
  target_link_libraries(basic psapi)
endif()

#   Under windows, copy geogram DLLs into binaries directory
# (we do that here instead of in graphite's main executable
#  build because we only need it for gomgen...)
copy_geogram_DLLs_for(basic)

set_target_properties(
    basic PROPERTIES
    FOLDER "GRAPHITE"
)


