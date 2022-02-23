set(APP_NAME gomgen)

aux_source_directories(SOURCES "" .)
add_executable(${APP_NAME} ${SOURCES})
target_link_libraries(${APP_NAME} gom basic geogram swig)
set_target_properties(
   ${APP_NAME} PROPERTIES
   FOLDER "GRAPHITE/Programs"
)

