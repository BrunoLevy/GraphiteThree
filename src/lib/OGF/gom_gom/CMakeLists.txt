
aux_source_directories(SOURCES "Source Files"              .)
aux_source_directories(SOURCES "Source Files\\common"      common)

gomgen(gom)
add_library(gom_gom ${SOURCES})
target_link_libraries(gom_gom gom)

set_target_properties(
    gom_gom PROPERTIES
    FOLDER "GRAPHITE"
)
