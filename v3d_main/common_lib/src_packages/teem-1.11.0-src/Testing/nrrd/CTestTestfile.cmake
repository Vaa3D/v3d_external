# CMake generated Testfile for 
# Source directory: F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd
# Build directory: F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(trand "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_trand")
set_tests_properties(trand PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;26;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;0;")
add_test(tload "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_tload")
set_tests_properties(tload PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;30;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;0;")
add_test(tskip11p "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_tskip" "-s" "101" "102" "103" "-p" "66" "81" "-o" "tsA.raw" "tsA.nhdr")
set_tests_properties(tskip11p PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;35;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;0;")
add_test(tskip11n "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_tskip" "-s" "101" "102" "103" "-p" "66" "81" "-ns" "-o" "tsB.raw" "tsB.nhdr")
set_tests_properties(tskip11n PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;36;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;0;")
add_test(tskip01p "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_tskip" "-s" "101" "102" "103" "-p" "0" "99" "-o" "tsC.raw" "tsC.nhdr")
set_tests_properties(tskip01p PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;37;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;0;")
add_test(tskip01n "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_tskip" "-s" "101" "102" "103" "-p" "0" "99" "-ns" "-o" "tsD.raw" "tsD.nhdr")
set_tests_properties(tskip01n PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;38;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;0;")
add_test(tskip10p "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_tskip" "-s" "101" "102" "103" "-p" "77" "0" "-o" "tsE.raw" "tsE.nhdr")
set_tests_properties(tskip10p PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;39;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;0;")
add_test(tskip10n "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_tskip" "-s" "101" "102" "103" "-p" "77" "0" "-ns" "-o" "tsF.raw" "tsF.nhdr")
set_tests_properties(tskip10n PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;40;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;0;")
add_test(sanity "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_sanity")
set_tests_properties(sanity PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;44;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;0;")
add_test(macros "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_macros")
set_tests_properties(macros PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;48;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/nrrd/CMakeLists.txt;0;")
