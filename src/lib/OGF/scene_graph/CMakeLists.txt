#include(${GEOGRAM_SOURCE_DIR}/cmake/opengl.cmake)

aux_source_directories(SOURCES "Source Files"                      .)
aux_source_directories(SOURCES "Source Files\\common"              common)
aux_source_directories(SOURCES "Source Files\\types"               types)
aux_source_directories(SOURCES "Source Files\\grob"                grob)
aux_source_directories(SOURCES "Source Files\\shaders"             shaders)
aux_source_directories(SOURCES "Source Files\\tools"               tools)
aux_source_directories(SOURCES "Source Files\\commands"            commands)
aux_source_directories(SOURCES "Source Files\\full_screen_effects" full_screen_effects)
aux_source_directories(SOURCES "Source Files\\NL"                  NL)

gomgen(scene_graph)
add_library(scene_graph ${SOURCES})
target_link_libraries(scene_graph skin gom_gom geogram_gfx ${OPENGL_LIBRARIES})

set_target_properties(
    scene_graph PROPERTIES
    FOLDER "GRAPHITE"
)
