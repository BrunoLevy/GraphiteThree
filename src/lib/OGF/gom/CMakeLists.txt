
aux_source_directories(SOURCES "Source Files"              .)
aux_source_directories(SOURCES "Source Files\\common"      common)
aux_source_directories(SOURCES "Source Files\\types"       types)
aux_source_directories(SOURCES "Source Files\\services"    services)
aux_source_directories(SOURCES "Source Files\\reflection"  reflection)
aux_source_directories(SOURCES "Source Files\\codegen"     codegen)
aux_source_directories(SOURCES "Source Files\\interpreter" interpreter)
aux_source_directories(SOURCES "Source Files\\lua"         lua)

add_library(gom ${SOURCES})
target_link_libraries(gom basic)

set_target_properties(
    gom PROPERTIES
    FOLDER "GRAPHITE"
)
