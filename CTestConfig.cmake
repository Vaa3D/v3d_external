## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   ENABLE_TESTING()
##   INCLUDE(CTest)
set(CTEST_PROJECT_NAME "V3D")
set(CTEST_NIGHTLY_START_TIME "01:00:00 UTC")
# specify how long to run the continuous in minutes
SET (CTEST_CONTINUOUS_DURATION 650)
SET (CTEST_CONTINUOUS_MINIMUM_INTERVAL 15)

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "my.cdash.org")
set(CTEST_DROP_LOCATION "/submit.php?project=V3D")
set(CTEST_DROP_SITE_CDASH TRUE)
