
aux_source_directories(SOURCES "Source Files"              .)
aux_source_directories(SOURCES "Source Files\\common"      common)
aux_source_directories(SOURCES "Source Files\\grob"        grob)
aux_source_directories(SOURCES "Source Files\\commands"    commands)
aux_source_directories(SOURCES "Source Files\\shaders"     shaders)
aux_source_directories(SOURCES "Source Files\\interfaces"  interfaces)
aux_source_directories(SOURCES "Source Files\\tools"       tools)

gomgen(voxel)
add_library(voxel ${SOURCES})
target_link_libraries(voxel mesh scene_graph_gfx)

set_target_properties(
    voxel PROPERTIES
    FOLDER "GRAPHITE"
)
