#include(${GEOGRAM_SOURCE_DIR}/cmake/opengl.cmake)

aux_source_directories(SOURCES "Source Files"                      .)
aux_source_directories(SOURCES "Source Files\\common"              common)

gomgen(scene_graph_gfx)
add_library(scene_graph_gfx ${SOURCES})
target_link_libraries(scene_graph_gfx scene_graph skin gom_gom geogram_gfx ${OPENGL_LIBRARIES})

set_target_properties(
    scene_graph_gfx PROPERTIES
    FOLDER "GRAPHITE"
)
