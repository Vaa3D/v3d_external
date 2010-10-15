#
#

if( V3D_USE_OWN_TIFF )
  add_subdirectory(src_packages/tiff-3.8.2.1/libtiff)
endif( V3D_USE_OWN_TIFF )

add_subdirectory(src_packages)
