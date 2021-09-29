cmake_minimum_required (VERSION 3.2.2)
PROJECT (SZIP_TEST)

#-----------------------------------------------------------------------------
# Setup include Directories
#-----------------------------------------------------------------------------
INCLUDE_DIRECTORIES (${SZIP_SOURCE_DIR})

#-----------------------------------------------------------------------------
# Define Sources
#-----------------------------------------------------------------------------
set (TEST_LIB_SRCS
    ${SZIP_TEST_SOURCE_DIR}/burst_szip.c
    ${SZIP_TEST_SOURCE_DIR}/mcgill.c
)

set (TEST_LIB_HEADERS
    ${SZIP_TEST_SOURCE_DIR}/mcgill.h
)
add_executable (burst_szip ${TEST_LIB_SRCS} ${TEST_LIB_HEADERS})
TARGET_NAMING (burst_szip STATIC)
TARGET_C_PROPERTIES (burst_szip STATIC " " " ")
target_link_libraries (burst_szip ${SZIP_LIB_TARGET})

#-- Adding test

add_test (NAME burst_szip COMMAND $<TARGET_FILE:burst_szip> -msb 8 8 1024 16 image.8.in)

if (SZIP_ENABLE_ENCODING)
  ADD_EXECUTABLE (gentest gentest.c mcgill.c)
  TARGET_NAMING (gentest STATIC)
  TARGET_C_PROPERTIES (gentest STATIC " " " ")
  target_link_libraries (gentest ${SZIP_LIB_TARGET})

  add_test (NAME gentest COMMAND $<TARGET_FILE:gentest>)
endif ()

#-- Adding test for example
add_executable (example example.c)
TARGET_NAMING (example STATIC)
TARGET_C_PROPERTIES (example STATIC " " " ")
target_link_libraries (example ${SZIP_LIB_TARGET})

add_test (NAME example COMMAND $<TARGET_FILE:example>)
set_tests_properties (example PROPERTIES DEPENDS burst_szip)
