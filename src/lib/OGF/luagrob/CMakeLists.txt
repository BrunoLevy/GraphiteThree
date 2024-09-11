
aux_source_directories(SOURCES "Source Files"              .)
aux_source_directories(SOURCES "Source Files\\common"      common)
aux_source_directories(SOURCES "Source Files\\grob"        grob)
aux_source_directories(SOURCES "Source Files\\commands"    commands)
aux_source_directories(SOURCES "Source Files\\shaders"     shaders)
aux_source_directories(SOURCES "Source Files\\tools"       tools)

gomgen(luagrob)
add_library(luagrob ${SOURCES})
target_link_libraries(luagrob scene_graph scene_graph_gfx)

set_target_properties(
    luagrob PROPERTIES
    FOLDER "GRAPHITE"
)
