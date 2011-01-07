file(GLOB MTImaging_SRCS "*.cpp")
include_directories( ${V3D_BINARY_DIR}/v3dbase )
add_library( multithreadimageIO ${MTImaging_SRCS} )
add_dependencies(multithreadimageIO v3dbase1)

