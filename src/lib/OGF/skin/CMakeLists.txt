include(${GEOGRAM_SOURCE_DIR}/cmake/opengl.cmake)

aux_source_directories(SOURCES "Source Files"              .)
aux_source_directories(SOURCES "Source Files\\common"      common)
aux_source_directories(SOURCES "Source Files\\types"       types)
aux_source_directories(SOURCES "Source Files\\gesture"     gesture)
aux_source_directories(SOURCES "Source Files\\transforms"  transforms)
aux_source_directories(SOURCES "Source Files\\symbols"     symbols)
# aux_source_directories(SOURCES "Source Files\\value"       value)

gomgen(skin)
add_library(skin ${SOURCES})
target_link_libraries(skin renderer gom_gom geogram_gfx ${OPENGL_LIBRARIES})

set_target_properties(
    skin PROPERTIES
    FOLDER "GRAPHITE"
)
