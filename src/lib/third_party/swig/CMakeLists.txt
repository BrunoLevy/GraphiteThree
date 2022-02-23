
if(WIN32)
  add_definitions(-D_LIB)
endif(WIN32)

aux_source_directories(SOURCES "Source Files"               .)
aux_source_directories(SOURCES "Source Files\\CParse"       CParse)
aux_source_directories(SOURCES "Source Files\\DOH"          DOH)
aux_source_directories(SOURCES "Source Files\\Modules"      Modules)
aux_source_directories(SOURCES "Source Files\\Preprocessor" Preprocessor)
aux_source_directories(SOURCES "Source Files\\Swig"         Swig)

add_library(swig ${SOURCES})

set_target_properties(
    swig PROPERTIES
    FOLDER "GRAPHITE"
)
