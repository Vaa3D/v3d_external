# CMake generated Testfile for 
# Source directory: F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing
# Build directory: F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(nrrd.Sanity "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/nrrdSanity")
set_tests_properties(nrrd.Sanity PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/CMakeLists.txt;50;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/CMakeLists.txt;0;")
subdirs("air")
subdirs("biff")
subdirs("nrrd")
subdirs("unrrdu")
subdirs("gage")
subdirs("ten")
subdirs("meet")
