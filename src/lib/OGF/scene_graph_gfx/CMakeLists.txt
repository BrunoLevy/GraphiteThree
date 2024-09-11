#include(${GEOGRAM_SOURCE_DIR}/cmake/opengl.cmake)

aux_source_directories(SOURCES "Source Files"                      .)
aux_source_directories(SOURCES "Source Files\\common"              common)
aux_source_directories(SOURCES "Source Files\\shaders"             shaders)
aux_source_directories(SOURCES "Source Files\\tools"               tools)
aux_source_directories(SOURCES "Source Files\\full_screen_effects" full_screen_effects)
aux_source_directories(SOURCES "Source Files\\transforms"          transforms)
aux_source_directories(SOURCES "Source Files\\symbols"             symbols)

gomgen(scene_graph_gfx)
add_library(scene_graph_gfx ${SOURCES})
target_link_libraries(scene_graph_gfx scene_graph renderer)

set_target_properties(
    scene_graph_gfx PROPERTIES
    FOLDER "GRAPHITE"
)
