# CMake generated Testfile for 
# Source directory: /Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem-1.11.0-src/Testing
# Build directory: /Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/Testing
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(nrrd.Sanity "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem/bin/nrrdSanity")
set_tests_properties(nrrd.Sanity PROPERTIES  _BACKTRACE_TRIPLES "/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem-1.11.0-src/Testing/CMakeLists.txt;50;ADD_TEST;/Users/jazz/Desktop/v3d_new/v3d_external/v3d_main/common_lib_new/src_packages/teem-1.11.0-src/Testing/CMakeLists.txt;0;")
subdirs("air")
subdirs("biff")
subdirs("nrrd")
subdirs("unrrdu")
subdirs("gage")
subdirs("ten")
subdirs("meet")
