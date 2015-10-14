file(GLOB Imaging_SRCS "*.cpp")
include_directories( ${V3D_BINARY_DIR}/v3dbase )
add_library( imaging ${Imaging_SRCS} )
add_dependencies(imaging ${TIFF_LIBRARY})

