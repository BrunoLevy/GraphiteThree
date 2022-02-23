aux_source_directories(SOURCES "Source Files"              .)
aux_source_directories(SOURCES "Source Files\\common"      common)
aux_source_directories(SOURCES "Source Files\\types"       types)
aux_source_directories(SOURCES "Source Files\\widgets"     widgets)

gomgen(skin_imgui)
add_library(skin_imgui ${SOURCES})
target_link_libraries(skin_imgui skin scene_graph renderer gom geogram_gfx glfw)

set_target_properties(
    skin_imgui PROPERTIES
    FOLDER "GRAPHITE"
)

