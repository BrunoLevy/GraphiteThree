#include(${GEOGRAM_SOURCE_DIR}/cmake/opengl.cmake)

aux_source_directories(SOURCES "Source Files"                      .)
aux_source_directories(SOURCES "Source Files\\common"              common)
aux_source_directories(SOURCES "Source Files\\types"               types)
aux_source_directories(SOURCES "Source Files\\skin"                skin)
aux_source_directories(SOURCES "Source Files\\grob"                grob)
aux_source_directories(SOURCES "Source Files\\commands"            commands)
aux_source_directories(SOURCES "Source Files\\NL"                  NL)

gomgen(scene_graph)
add_library(scene_graph ${SOURCES})
target_link_libraries(scene_graph gom_gom)

set_target_properties(
    scene_graph PROPERTIES
    FOLDER "GRAPHITE"
)
