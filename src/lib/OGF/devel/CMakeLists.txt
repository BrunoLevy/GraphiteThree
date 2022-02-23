
aux_source_directories(SOURCES "Source Files"                      .)
aux_source_directories(SOURCES "Source Files\\common"              common)
aux_source_directories(SOURCES "Source Files\\types"               types)
aux_source_directories(SOURCES "Source Files\\commands"            commands)

gomgen(devel)
add_library(devel ${SOURCES})

target_link_libraries(devel scene_graph)

set_target_properties(
    devel PROPERTIES
    FOLDER "GRAPHITE"
)
