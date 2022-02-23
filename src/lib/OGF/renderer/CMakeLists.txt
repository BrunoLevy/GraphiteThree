include(${GEOGRAM_SOURCE_DIR}/cmake/opengl.cmake)

aux_source_directories(SOURCES "Source Files"                      .)
aux_source_directories(SOURCES "Source Files\\common"              common)
aux_source_directories(SOURCES "Source Files\\context"             context)


add_library(renderer ${SOURCES})
target_link_libraries(renderer basic geogram_gfx ${OPENGL_LIBRARIES})

set_target_properties(
    renderer PROPERTIES
    FOLDER "GRAPHITE"
)
