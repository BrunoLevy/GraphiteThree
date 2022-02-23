#
# Build rules for the documentation
#

include(${GEOGRAM_SOURCE_DIR}/cmake/doxygen.cmake)

# Make GEOGRAM_SOURCE_DIR an absolute path, so that
# Doxygen can know which parts of the paths should
# be stripped.
get_filename_component(GEOGRAM_SOURCE_DIR ${GEOGRAM_SOURCE_DIR} REALPATH)

if(DOXYGEN_FOUND)
   add_doc_target(doc)
endif()   


