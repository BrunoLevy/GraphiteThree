
aux_source_directories(SOURCES "Source Files"              .)
aux_source_directories(SOURCES "Source Files\\common"      common)
aux_source_directories(SOURCES "Source Files\\shaders"     shaders)

gomgen(voxel_gfx)
add_library(voxel_gfx ${SOURCES})
target_link_libraries(voxel_gfx voxel scene_graph_gfx)

set_target_properties(
    voxel_gfx PROPERTIES
    FOLDER "GRAPHITE"
)
