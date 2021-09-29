
# The command CMAKE_EXPORT_BUILD_SETTINGS(...) was used by
# Teem to generate this file.  As of CMake 2.8 the
# functionality of this command has been dropped as it was deemed
# harmful (confusing users by changing their compiler).

# CMake 2.6 and below do not support loading their equivalent of this
# file if it was produced by a newer version of CMake.  CMake 2.8 and
# above simply do not load this file.  Therefore we simply error out.
message(FATAL_ERROR
  "This Teem was built by CMake 3.18.4, but this is CMake "
  "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}.  "
  "Please upgrade CMake to a more recent version.")
