file(GLOB Imaging_SRCS "*.cpp")
include_directories( ${V3D_BINARY_DIR}/v3dbase )
include_directories( ../basic_c_fun )
add_library( imaging ${Imaging_SRCS} )
add_dependencies(imaging ${TIFF_LIBRARY})

