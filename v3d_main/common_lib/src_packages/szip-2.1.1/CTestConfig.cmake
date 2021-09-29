## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   enable_testing()
##   include(CTest)
set (CTEST_PROJECT_NAME "SZIP")
set (CTEST_NIGHTLY_START_TIME "18:00:00 CST")

set (CTEST_DROP_METHOD "http")
if (CDASH_LOCAL)
  set (CTEST_DROP_SITE "10.10.10.82")
  set (CTEST_DROP_LOCATION "/submit.php?project=Externals")
else ()
  set (CTEST_DROP_SITE "cdash.hdfgroup.org")
  set (CTEST_DROP_LOCATION "/submit.php?project=Externals")
endif ()
set (CTEST_DROP_SITE_CDASH TRUE)

set (UPDATE_TYPE svn)
set (VALGRIND_COMMAND "/usr/bin/valgrind")
set (VALGRIND_COMMAND_OPTIONS "-v --tool=memcheck --leak-check=full --track-fds=yes --num-callers=50 --show-reachable=yes --track-origins=yes --malloc-fill=0xff --free-fill=0xfe")

set (CTEST_TEST_TIMEOUT 3600 CACHE STRING 
    "Maximum time allowed before CTest will kill the test.") 
set (DART_TESTING_TIMEOUT 3600 CACHE STRING 
    "Maximum time allowed before CTest will kill the test." FORCE)

set (CTEST_SUBMIT_RETRY_DELAY 20 CACHE STRING
    "How long to wait between timed-out CTest submissions.")
