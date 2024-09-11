
aux_source_directories(SOURCES "Source Files"              .)
aux_source_directories(SOURCES "Source Files\\common"      common)
aux_source_directories(SOURCES "Source Files\\grob"        grob)
aux_source_directories(SOURCES "Source Files\\algo"        algo)
aux_source_directories(SOURCES "Source Files\\commands"    commands)
aux_source_directories(SOURCES "Source Files\\interfaces"  interfaces)
aux_source_directories(SOURCES "Source Files\\shaders"     shaders)
aux_source_directories(SOURCES "Source Files\\tools"       tools)

gomgen(mesh)
add_library(mesh ${SOURCES})
target_link_libraries(mesh scene_graph)

if(GEOGRAM_WITH_VORPALINE)
  target_link_libraries(mesh vorpalib)
endif()

set_target_properties(
    mesh PROPERTIES
    FOLDER "GRAPHITE"
)
