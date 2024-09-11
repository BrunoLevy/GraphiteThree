
aux_source_directories(SOURCES "Source Files"              .)
aux_source_directories(SOURCES "Source Files\\common"      common)
aux_source_directories(SOURCES "Source Files\\shaders"     shaders)
aux_source_directories(SOURCES "Source Files\\tools"       tools)
aux_source_directories(SOURCES "Source Files\\commands"    commands)

gomgen(mesh_gfx)
add_library(mesh_gfx ${SOURCES})
target_link_libraries(mesh_gfx mesh scene_graph_gfx)

set_target_properties(
    mesh_gfx PROPERTIES
    FOLDER "GRAPHITE"
)
