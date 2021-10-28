# CMake generated Testfile for 
# Source directory: F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet
# Build directory: F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(enmall "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_enmall")
set_tests_properties(enmall PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;26;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;0;")
add_test(kernall "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_kernall")
set_tests_properties(kernall PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;30;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;0;")
add_test(buildinfo "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_buildinfo")
set_tests_properties(buildinfo PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;34;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;0;")
add_test(probeSS_box01 "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_probeSS" "-k" "box" "-supp" "1.0" "-pnum" "1500")
set_tests_properties(probeSS_box01 PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;38;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;0;")
add_test(probeSS_cos01 "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_probeSS" "-k" "cos" "-supp" "1.0" "-pnum" "1500")
set_tests_properties(probeSS_cos01 PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;39;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;0;")
add_test(probeSS_cos02 "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_probeSS" "-k" "cos" "-supp" "2.0" "-pnum" "1200")
set_tests_properties(probeSS_cos02 PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;40;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;0;")
add_test(probeSS_cos04 "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_probeSS" "-k" "cos" "-supp" "4.0" "-pnum" "1000")
set_tests_properties(probeSS_cos04 PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;41;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;0;")
add_test(probeSS_cos10 "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_probeSS" "-k" "cos" "-supp" "9.0" "-pnum" "800")
set_tests_properties(probeSS_cos10 PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;42;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;0;")
add_test(probeSS_ctmr02 "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_probeSS" "-k" "ctmr" "-supp" "2.0" "-pnum" "1300")
set_tests_properties(probeSS_ctmr02 PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;43;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;0;")
add_test(probeSS_ctmr04 "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_probeSS" "-k" "ctmr" "-supp" "4.0" "-pnum" "1300")
set_tests_properties(probeSS_ctmr04 PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;44;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;0;")
add_test(probeSS_ctmr10 "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/bin/test_probeSS" "-k" "ctmr" "-supp" "9.0" "-pnum" "1300")
set_tests_properties(probeSS_ctmr10 PROPERTIES  _BACKTRACE_TRIPLES "F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;45;ADD_TEST;F:/v3d_mingw1/v3d_external/v3d_main/common_lib/src_packages/teem-1.11.0-src/Testing/meet/CMakeLists.txt;0;")
